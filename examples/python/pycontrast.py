#!/usr/bin/env python

import ctypes
import os.path
import re
import sys
from pathlib import Path
from typing import Final

if sys.platform.startswith("linux"):
    libcast_handle = ctypes.CDLL("./libcast.so", ctypes.RTLD_GLOBAL)._handle  # load the libcast.so shared library
    pyclariuscast = ctypes.cdll.LoadLibrary("./pyclariuscast.so")  # load the pyclariuscast.so shared library

import pyclariuscast
from PySide6 import QtCore, QtGui, QtWidgets
from PySide6.QtCore import Slot

CMD_FREEZE: Final = 1

allowedPulse = r"[0+-]+"


def is_number(n):
    try:
        float(n)
    except ValueError:
        return False
    return True


# custom event for handling change in freeze state
class FreezeEvent(QtCore.QEvent):
    def __init__(self, frozen):
        super().__init__(QtCore.QEvent.User)
        self.frozen = frozen


# custom event for handling new images
class ImageEvent(QtCore.QEvent):
    def __init__(self):
        super().__init__(QtCore.QEvent.Type(QtCore.QEvent.User + 1))


# manages custom events posted from callbacks, then relays as signals to the main widget
class Signaller(QtCore.QObject):
    freeze = QtCore.Signal(bool)
    button = QtCore.Signal(int, int)
    image = QtCore.Signal(QtGui.QImage)

    def __init__(self):
        QtCore.QObject.__init__(self)
        self.usimage = QtGui.QImage()

    def event(self, evt):
        if evt.type() == QtCore.QEvent.User:
            self.freeze.emit(evt.frozen)
        elif evt.type() == QtCore.QEvent.Type(QtCore.QEvent.User + 1):
            self.image.emit(self.usimage)
        return True


# global required for the cast api callbacks
signaller = Signaller()


# draws the ultrasound image
class ImageView(QtWidgets.QGraphicsView):
    def __init__(self, cast):
        QtWidgets.QGraphicsView.__init__(self)
        self.cast = cast
        self.setScene(QtWidgets.QGraphicsScene())

    # set the new image and redraw
    def updateImage(self, img):
        self.image = img
        self.scene().invalidate()

    # saves a local image
    def saveImage(self):
        self.image.save(str(Path.home() / "Pictures/clarius_image.png"))

    # resize the scan converter, image, and scene
    def resizeEvent(self, evt):
        w = evt.size().width()
        h = evt.size().height()
        self.cast.setOutputSize(w, h)
        self.image = QtGui.QImage(w, h, QtGui.QImage.Format_ARGB32)
        self.image.fill(QtCore.Qt.black)
        self.setSceneRect(0, 0, w, h)

    # black background
    def drawBackground(self, painter, rect):
        painter.fillRect(rect, QtCore.Qt.black)

    # draws the image
    def drawForeground(self, painter, rect):
        if not self.image.isNull():
            painter.drawImage(rect, self.image)


# main widget with controls and ui
class MainWidget(QtWidgets.QMainWindow):
    def __init__(self, cast, parent=None):
        QtWidgets.QMainWindow.__init__(self, parent)

        self.cast = cast
        self.setWindowTitle("Clarius Cast Demo")

        # create central widget within main window
        central = QtWidgets.QWidget()
        self.setCentralWidget(central)

        ip = QtWidgets.QLineEdit("192.168.1.1")
        ip.setInputMask("000.000.000.000")
        port = QtWidgets.QLineEdit("5828")
        port.setInputMask("00000")

        _txFreq = QtWidgets.QLabel("Tx Freq (MHz)")
        txFreq = QtWidgets.QLineEdit("1.9")
        _rxFreq = QtWidgets.QLabel("Rx Freq (MHz)")
        rxFreq = QtWidgets.QLineEdit("3.8")

        _txApt = QtWidgets.QLabel("Tx Aperture")
        txApt = QtWidgets.QLineEdit("64")

        _txPulse = QtWidgets.QLabel("Tx Pulse")
        txPulse = QtWidgets.QLineEdit("+-+-")
        _txPulseInv = QtWidgets.QLabel("Tx Pulse Invert")
        txPulseInv = QtWidgets.QLineEdit("-+-+")

        _vpp = QtWidgets.QLabel("+ Amplitudue (V)")
        vpp = QtWidgets.QLineEdit("10")
        _vnn = QtWidgets.QLabel("- Amplitudue (V)")
        vnn = QtWidgets.QLineEdit("10")

        _tgcS = QtWidgets.QLabel("TGC Shallow (dB)")
        tgcS = QtWidgets.QLineEdit("0.0")
        _tgcD = QtWidgets.QLabel("TGC Deep (dB)")
        tgcD = QtWidgets.QLineEdit("35.0")

        _nf = QtWidgets.QLabel("Noise Floor (dB)")
        nf = QtWidgets.QLineEdit("20.0")
        _dr = QtWidgets.QLabel("Dynamic Range (dB)")
        dr = QtWidgets.QLineEdit("50.0")

        conn = QtWidgets.QPushButton("Connect")
        self.run = QtWidgets.QPushButton("Run")
        quit = QtWidgets.QPushButton("Quit")
        saveImage = QtWidgets.QPushButton("Save Local")
        apply = QtWidgets.QPushButton("Apply Parameters")

        # try to connect/disconnect to/from the probe
        def tryConnect():
            if not cast.isConnected():
                if cast.connect(ip.text(), int(port.text()), "research"):
                    self.statusBar().showMessage("Connected")
                    conn.setText("Disconnect")
                else:
                    self.statusBar().showMessage(f"Failed to connect to {ip.text()}")
            elif cast.disconnect():
                self.statusBar().showMessage("Disconnected")
                conn.setText("Connect")
            else:
                self.statusBar().showMessage("Failed to disconnect")

        # try to freeze/unfreeze
        def tryFreeze():
            if cast.isConnected():
                cast.userFunction(CMD_FREEZE, 0)

        # try to save a local image
        def trySaveImage():
            self.img.saveImage()

        # try to apply parameters
        def tryApplyParams():
            if cast.isConnected():
                if is_number(txFreq.text()):
                    cast.setParam("ceTxFreq", float(txFreq.text()))
                if is_number(rxFreq.text()):
                    cast.setParam("ceRxFreq", float(rxFreq.text()))
                if re.fullmatch(allowedPulse, txPulse.text()):
                    cast.setPulse("ceTxPulse", txPulse.text())
                if re.fullmatch(allowedPulse, txPulseInv.text()):
                    cast.setPulse("txPulseInv", txPulseInv.text())
                if txApt.text().isnumeric():
                    cast.setParam("txApt", int(txApt.text()))
                if is_number(vpp.text()):
                    cast.setParam("ceVpp", float(vpp.text()))
                if is_number(vnn.text()):
                    cast.setParam("ceVnn", float(vnn.text()))
                if is_number(tgcS.text()):
                    cast.setParam("ceTgcLevel1", float(tgcS.text()))
                if is_number(tgcD.text()):
                    cast.setParam("ceTgcLevel2", float(tgcD.text()))
                if is_number(nf.text()):
                    cast.setParam("ceNoiseFloor", float(nf.text()))
                if is_number(dr.text()):
                    cast.setParam("ceDynRange", float(dr.text()))

        conn.clicked.connect(tryConnect)
        self.run.clicked.connect(tryFreeze)
        quit.clicked.connect(self.shutdown)
        saveImage.clicked.connect(trySaveImage)
        apply.clicked.connect(tryApplyParams)

        # add widgets to layout
        self.img = ImageView(cast)
        layout = QtWidgets.QVBoxLayout()
        layout.addWidget(self.img)

        inplayout = QtWidgets.QHBoxLayout()
        layout.addLayout(inplayout)
        inplayout.addWidget(ip)
        inplayout.addWidget(port)

        connlayout = QtWidgets.QHBoxLayout()
        layout.addLayout(connlayout)
        connlayout.addWidget(conn)
        connlayout.addWidget(self.run)
        connlayout.addWidget(quit)
        central.setLayout(layout)

        txlayout = QtWidgets.QHBoxLayout()
        layout.addLayout(txlayout)
        txlayout.addWidget(_txFreq)
        txlayout.addWidget(txFreq)
        txlayout.addWidget(_rxFreq)
        txlayout.addWidget(rxFreq)

        pulselayout = QtWidgets.QHBoxLayout()
        layout.addLayout(pulselayout)
        pulselayout.addWidget(_txPulse)
        pulselayout.addWidget(txPulse)
        pulselayout.addWidget(_txPulseInv)
        pulselayout.addWidget(txPulseInv)

        voltagelayout = QtWidgets.QHBoxLayout()
        layout.addLayout(voltagelayout)
        voltagelayout.addWidget(_vpp)
        voltagelayout.addWidget(vpp)
        voltagelayout.addWidget(_vnn)
        voltagelayout.addWidget(vnn)

        tgclayout = QtWidgets.QHBoxLayout()
        layout.addLayout(tgclayout)
        tgclayout.addWidget(_tgcS)
        tgclayout.addWidget(tgcS)
        tgclayout.addWidget(_tgcD)
        tgclayout.addWidget(tgcD)

        vislayout = QtWidgets.QHBoxLayout()
        layout.addLayout(vislayout)
        vislayout.addWidget(_nf)
        vislayout.addWidget(nf)
        vislayout.addWidget(_dr)
        vislayout.addWidget(dr)

        txaptlayout = QtWidgets.QHBoxLayout()
        layout.addLayout(txaptlayout)
        txaptlayout.addWidget(_txApt)
        txaptlayout.addWidget(txApt)

        genlayout = QtWidgets.QHBoxLayout()
        layout.addLayout(genlayout)
        genlayout.addWidget(apply)
        genlayout.addWidget(saveImage)

        # connect signals
        signaller.freeze.connect(self.freeze)
        signaller.image.connect(self.image)

        # get home path
        path = os.path.expanduser("~/")
        if cast.init(path, 640, 480):
            self.statusBar().showMessage("Initialized")
        else:
            self.statusBar().showMessage("Failed to initialize")

    # handles freeze messages
    @Slot(bool)
    def freeze(self, frozen):
        if frozen:
            self.run.setText("Run")
            self.statusBar().showMessage("Image Stopped")
        else:
            self.run.setText("Freeze")
            self.statusBar().showMessage("Image Running (check firewall settings if no image seen)")

    # handles new images
    @Slot(QtGui.QImage)
    def image(self, img):
        self.img.updateImage(img)

    # handles shutdown
    @Slot()
    def shutdown(self):
        if sys.platform.startswith("linux"):
            # unload the shared library before destroying the cast object
            ctypes.CDLL("libc.so.6").dlclose(libcast_handle)
        self.cast.destroy()
        QtWidgets.QApplication.quit()


## called when a new processed image is streamed
# @param image the scan-converted image data
# @param width width of the image in pixels
# @param height height of the image in pixels
# @param sz full size of image
# @param micronsPerPixel microns per pixel
# @param timestamp the image timestamp in nanoseconds
# @param angle acquisition angle for volumetric data
# @param imu inertial data tagged with the frame
def newProcessedImage(image, width, height, sz, micronsPerPixel, timestamp, angle, imu):
    bpp = sz / (width * height)
    if bpp == 4:
        img = QtGui.QImage(image, width, height, QtGui.QImage.Format_ARGB32)
    else:
        img = QtGui.QImage(image, width, height, QtGui.QImage.Format_Grayscale8)
    # a deep copy is important here, as the memory from 'image' won't be valid after the event posting
    signaller.usimage = img.copy()
    evt = ImageEvent()
    QtCore.QCoreApplication.postEvent(signaller, evt)


## called when a new raw image is streamed
# @param image the raw pre scan-converted image data, uncompressed 8-bit or jpeg compressed
# @param lines number of lines in the data
# @param samples number of samples in the data
# @param bps bits per sample
# @param axial microns per sample
# @param lateral microns per line
# @param timestamp the image timestamp in nanoseconds
# @param jpg jpeg compression size if the data is in jpeg format
# @param rf flag for if the image received is radiofrequency data
# @param angle acquisition angle for volumetric data
def newRawImage(image, lines, samples, bps, axial, lateral, timestamp, jpg, rf, angle):
    return


## called when a new spectrum image is streamed
# @param image the spectral image
# @param lines number of lines in the spectrum
# @param samples number of samples per line
# @param bps bits per sample
# @param period line repetition period of spectrum
# @param micronsPerSample microns per sample for an m spectrum
# @param velocityPerSample velocity per sample for a pw spectrum
# @param pw flag that is true for a pw spectrum, false for an m spectrum
def newSpectrumImage(image, lines, samples, bps, period, micronsPerSample, velocityPerSample, pw):
    return


## called when a new imu data is streamed
# @param imu inertial data tagged with the frame
def newImuData(imu):
    return


## called when freeze state changes
# @param frozen the freeze state
def freezeFn(frozen):
    evt = FreezeEvent(frozen)
    QtCore.QCoreApplication.postEvent(signaller, evt)


## called when a button is pressed
# @param button the button that was pressed
# @param clicks number of clicks performed
def buttonsFn(button, clicks):
    return


## main function
def main():
    cast = pyclariuscast.Caster(newProcessedImage, newRawImage, newSpectrumImage, newImuData, freezeFn, buttonsFn)
    app = QtWidgets.QApplication(sys.argv)
    widget = MainWidget(cast)
    widget.resize(640, 480)
    widget.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
