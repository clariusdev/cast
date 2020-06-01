#!/usr/bin/env python

import sys
import os.path
from PySide2 import QtCore, QtGui, QtWidgets
from PySide2.QtCore import Qt, Slot, Signal
import pylisten

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
    image = QtCore.Signal(QtGui.QImage)

    def __init__(self):
        QtCore.QObject.__init__(self)
        self.usimage = QtGui.QImage()

    def event(self, evt):
        if evt.type() == QtCore.QEvent.User:
            self.freeze.emit(evt.frozen)
        elif evt.type() == QtCore.QEvent.Type(QtCore.QEvent.User + 1):
            self.button.emit(evt.btn, evt.clicks)
        elif evt.type() == QtCore.QEvent.Type(QtCore.QEvent.User + 2):
            self.image.emit(self.usimage)
        return True

# global required for the listen api callbacks
signaller = Signaller()

# draws the ultrasound image
class ImageView(QtWidgets.QGraphicsView):
    def __init__(self, listen):
        QtWidgets.QGraphicsView.__init__(self)
        self.listen = listen
        self.setScene(QtWidgets.QGraphicsScene())

    # set the new image and redraw
    def updateImage(self, img):
        self.image = img
        self.scene().invalidate()

    # resize the scan converter, image, and scene
    def resizeEvent(self, evt):
        w = evt.size().width()
        h = evt.size().height()
        self.listen.setOutputSize(w, h)
        self.image = QtGui.QImage(w, h, QtGui.QImage.Format_ARGB32)
        self.image.fill(QtCore.Qt.black);
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
    def __init__(self, listen, parent=None):
        QtWidgets.QMainWindow.__init__(self, parent)

        self.listen = listen

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
            if not listen.isConnected():
                if listen.connect(ip.text(), int(port.text())):
                    self.statusBar().showMessage("Connected")
                    conn.setText("Disconnect")
                else:
                    self.statusBar().showMessage("Failed to connect to {0}".format(ip.text()))
            else:
                if listen.disconnect():
                    self.statusBar().showMessage("Disconnected")
                    conn.setText("Connect")
                else:
                    self.statusBar().showMessage("Failed to disconnect")

        conn.clicked.connect(tryConnect)
        quit = QtWidgets.QPushButton("Quit")
        quit.clicked.connect(self.shutdown)

        # add widgets to layout
        self.img = ImageView(listen)
        layout = QtWidgets.QVBoxLayout()
        layout.addWidget(self.img)
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
        if  listen.init(path, 640, 480):
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
    @Slot(QtGui.QImage)
    def image(self, img):
        self.img.updateImage(img)

    # handles shutdown
    @Slot()
    def shutdown(self):
        self.listen.destroy()
        QtWidgets.QApplication.quit()

## called when a new processed image is streamed
# @param image the scan-converted image data
# @param width width of the image in pixels
# @param height height of the image in pixels
# @param bpp bits per pixel
# @param micronsPerPixel microns per pixel
# @param timestamp the image timestamp in nanoseconds
def newProcessedImage(image, width, height, bpp, micronsPerPixel, timestamp, imu):
    img = QtGui.QImage(image, width, height, QtGui.QImage.Format_ARGB32)
    # a deep copy is important here, as the memory from 'image' won't be valid after the event posting
    signaller.usimage = img.copy()
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
def newRawImage(image, lines, samples, bps, axial, lateral, timestamp, jpg):
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
    listen = pylisten.Listener(newProcessedImage, newRawImage, freezeFn, buttonsFn)
    app = QtWidgets.QApplication(sys.argv)
    widget = MainWidget(listen)
    widget.resize(640, 480)
    widget.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
