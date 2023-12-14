#!/usr/bin/env python

import ctypes
import os.path
import sys

if sys.platform.startswith("linux"):
    libcast_handle = ctypes.CDLL("./libcast.so", ctypes.RTLD_GLOBAL)._handle  # load the libcast.so shared library
    pyclariuscast = ctypes.cdll.LoadLibrary("./pyclariuscast.so")  # load the pyclariuscast.so shared library

import pyclariuscast
from PySide6 import QtCore, QtGui, QtWidgets
from PySide6.Qt3DCore import Qt3DCore
from PySide6.Qt3DExtras import Qt3DExtras
from PySide6.Qt3DRender import Qt3DRender
from PySide6.QtCore import QUrl, Slot
from PySide6.QtGui import QQuaternion, QVector3D


# custom event for handling change in freeze state
class FreezeEvent(QtCore.QEvent):
    def __init__(self, frozen):
        super().__init__(QtCore.QEvent.User)
        self.frozen = frozen


# custom event for handling button presses
class ButtonEvent(QtCore.QEvent):
    def __init__(self, btn, clicks):
        super().__init__(QtCore.QEvent.Type(QtCore.QEvent.User + 1))
        self.btn = btn
        self.clicks = clicks


# custom event for handling new images
class ImageEvent(QtCore.QEvent):
    def __init__(self):
        super().__init__(QtCore.QEvent.Type(QtCore.QEvent.User + 2))


# manages custom events posted from callbacks, then relays as signals to the main widget
class Signaller(QtCore.QObject):
    freeze = QtCore.Signal(bool)
    button = QtCore.Signal(int, int)
    image = QtCore.Signal(float, float, float, float)
    qw = 0
    qx = 0
    qy = 0
    qz = 0

    def __init__(self):
        QtCore.QObject.__init__(self)
        self.usimage = QtGui.QImage()

    def event(self, evt):
        if evt.type() == QtCore.QEvent.User:
            self.freeze.emit(evt.frozen)
        elif evt.type() == QtCore.QEvent.Type(QtCore.QEvent.User + 1):
            self.button.emit(evt.btn, evt.clicks)
        elif evt.type() == QtCore.QEvent.Type(QtCore.QEvent.User + 2):
            self.image.emit(self.qw, self.qx, self.qy, self.qz)
        return True


# global required for the listen api callbacks
signaller = Signaller()


# 3d render class
class ScannerWindow(Qt3DExtras.Qt3DWindow):
    qw = 0.5
    qx = 0.5
    qy = -0.5
    qz = 0.5
    scannerTransform = Qt3DCore.QTransform()

    def __init__(self):
        super(ScannerWindow, self).__init__()

        # camera
        self.camera().lens().setPerspectiveProjection(50, 16 / 9, 0.1, 1000)
        self.camera().setPosition(QVector3D(0, 0, 30))
        self.camera().setViewCenter(QVector3D(0, 0, 0))

        # create scene from obj file
        self.createScene()
        self.setRootEntity(self.rootEntity)

    def updateAngle(self, qw, qx, qy, qz):
        self.qw = qw
        self.qx = qx
        self.qy = qy
        self.qz = qz
        self.addTransform()

    def addTransform(self):
        # correct orientation
        self.scannerTransform.setScale3D(QVector3D(100, 100, 100))
        self.orientation = QQuaternion(self.qw, self.qx, self.qy, self.qz)
        self.axisCorrection = QQuaternion.fromEulerAngles(0, 180, 90)
        self.modelCorrection = QQuaternion.fromEulerAngles(-90, 0, 90)
        self.modelRotation = self.orientation * self.axisCorrection
        self.correctedOrientation = self.modelCorrection * self.modelRotation
        self.scannerTransform.setRotation(self.correctedOrientation)
        self.scannerEntity.addComponent(self.scannerTransform)

    def createScene(self):
        self.rootEntity = Qt3DCore.QEntity()
        self.scannerEntity = Qt3DCore.QEntity(self.rootEntity)
        # QSceneLoader loads materials from scanner.mtl referenced in scanner.obj
        self.scanner = Qt3DRender.QSceneLoader(self.scannerEntity)
        self.scanner.setSource(QUrl.fromLocalFile("scanner.obj"))
        self.scannerEntity.addComponent(self.scanner)
        self.addTransform()


# main widget with controls and ui
class MainWidget(QtWidgets.QMainWindow):
    def __init__(self, cast, parent=None):
        QtWidgets.QMainWindow.__init__(self, parent)

        self.cast = cast

        # create central widget within main window
        central = QtWidgets.QWidget()
        self.setCentralWidget(central)

        ip = QtWidgets.QLineEdit("192.168.1.1")
        ip.setInputMask("000.000.000.000")
        port = QtWidgets.QLineEdit("5828")
        port.setInputMask("00000")

        conn = QtWidgets.QPushButton("Connect")

        # try to connect/disconnect to/from the probe
        def tryConnect():
            if not cast.isConnected():
                if cast.connect(ip.text(), int(port.text()), "research"):
                    self.statusBar().showMessage("Connected")
                    conn.setText("Disconnect")
                else:
                    self.statusBar().showMessage("Failed to connect to {0}".format(ip.text()))
            else:
                if cast.disconnect():
                    self.statusBar().showMessage("Disconnected")
                    conn.setText("Connect")
                else:
                    self.statusBar().showMessage("Failed to disconnect")

        conn.clicked.connect(tryConnect)
        quit = QtWidgets.QPushButton("Quit")
        quit.clicked.connect(self.shutdown)

        # add widgets to layout
        self.scanner = ScannerWindow()
        scannerWidget = QtWidgets.QWidget.createWindowContainer(self.scanner)
        layout = QtWidgets.QVBoxLayout()
        layout.addWidget(scannerWidget)
        layout.addWidget(ip)
        layout.addWidget(port)
        layout.addWidget(conn)
        layout.addWidget(quit)
        central.setLayout(layout)

        # connect signals
        signaller.freeze.connect(self.freeze)
        signaller.button.connect(self.button)
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
            self.statusBar().showMessage("Image Stopped")
        else:
            self.statusBar().showMessage("Image Running")

    # handles button messages
    @Slot(int, int)
    def button(self, btn, clicks):
        self.statusBar().showMessage("Button {0} pressed w/ {1} clicks".format(btn, clicks))

    # handles new images
    @Slot(float, float, float, float)
    def image(self, qw, qx, qy, qz):
        self.scanner.updateAngle(qw, qx, qy, qz)

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
# @param bpp bits per pixel
# @param micronsPerPixel microns per pixel
# @param timestamp the image timestamp in nanoseconds
# @param angle acquisition angle for volumetric data
# @param imu imu data sets
def newProcessedImage(image, width, height, bpp, micronsPerPixel, timestamp, angle, imu):
    if len(imu) > 0:
        signaller.qw = imu[0].qw
        signaller.qx = imu[0].qx
        signaller.qy = imu[0].qy
        signaller.qz = imu[0].qz
        evt = ImageEvent()
        QtCore.QCoreApplication.postEvent(signaller, evt)
    return


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
    return


## called when a button is pressed
# @param button the button that was pressed
# @param clicks number of clicks performed
def buttonsFn(button, clicks):
    evt = ButtonEvent(button, clicks)
    QtCore.QCoreApplication.postEvent(signaller, evt)
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
