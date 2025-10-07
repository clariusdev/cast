#!/usr/bin/env python

import argparse
import ctypes
import os.path
import sys

if sys.platform.startswith("linux"):
    libcast_handle = ctypes.CDLL("./libcast.so", ctypes.RTLD_GLOBAL)._handle  # load the libcast.so shared library
    pyclariuscast = ctypes.cdll.LoadLibrary("./pyclariuscast.so")  # load the pyclariuscast.so shared library

import pyclariuscast
from PIL import Image

printStream = True


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
    if printStream:
        print(
            f"image: {timestamp}, {width}x{height} @ {bpp} bpp, {micronsPerPixel:.2f} um/px, imu: {len(imu)} pts",
            end="\r",
        )
    if bpp == 4:
        img = Image.frombytes("RGBA", (width, height), image)
    else:
        img = Image.frombytes("L", (width, height), image)
    # img.save("processed_image.png")


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
    # check the rf flag for radiofrequency data vs raw grayscale
    # raw grayscale data is non scan-converted and in polar co-ordinates
    # print(
    #    "raw image: {0}, {1}x{2} @ {3} bps, {4:.2f} um/s, {5:.2f} um/l, rf: {6}".format(
    #        timestamp, lines, samples, bps, axial, lateral, rf
    #    ), end = "\r"
    # )
    # if jpg == 0:
    #    img = Image.frombytes("L", (samples, lines), image, "raw")
    # else:
    #    # note! this probably won't work unless a proper decoder is written
    #    img = Image.frombytes("L", (samples, lines), image, "jpg")
    # img.save("raw_image.jpg")
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
    if frozen:
        print("\nimaging frozen")
    else:
        print("imaging running")


## called when a button is pressed
# @param button the button that was pressed
# @param clicks number of clicks performed
def buttonsFn(button, clicks):
    print(f"button pressed: {button}, clicks: {clicks}")


## main function
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--address", "-a", dest="ip", help="ip address of probe.", required=True)
    parser.add_argument("--port", "-p", dest="port", type=int, help="port of the probe", required=True)
    parser.add_argument("--width", "-w", dest="width", type=int, help="image output width in pixels")
    parser.add_argument("--height", "-ht", dest="height", type=int, help="image output height in pixels")
    parser.set_defaults(ip=None)
    parser.set_defaults(port=None)
    parser.set_defaults(width=640)
    parser.set_defaults(height=480)
    args = parser.parse_args()

    # uncomment to get documentation for pyclariuscast module
    # print(help(pyclariuscast))

    if not args.ip or not args.port or args.port < 0:
        print("one or more arguments are invalid")
        parser.print_usage()
        return

    # get home path
    path = os.path.expanduser("~/")

    # initialize
    cast = pyclariuscast.Caster(newProcessedImage, newRawImage, newSpectrumImage, newImuData, freezeFn, buttonsFn)
    ret = cast.init(path, args.width, args.height)
    if ret:
        print("initialization succeeded")
        ret = cast.connect(args.ip, args.port, "research")
        if ret:
            print(f"connected to {args.ip} on port {args.port}")
        else:
            print("connection failed")
            if sys.platform.startswith("linux"):
                # unload the shared library before destroying the cast object
                ctypes.CDLL("libc.so.6").dlclose(libcast_handle)
            cast.destroy()
            return
    else:
        print("initialization failed")
        return

    # input loop
    key = ""
    while key != "q" and key != "Q":
        key = input("press (q)->quit (a)->action (s)->stream (p)->param change: ")
        if key == "a" or key == "A":
            key = input("(f)->freeze (i)->capture image (c)->capture cine, (d/D)->depth, (g/G)->gain: ")
            if key == "f" or key == "F":
                cast.userFunction(1, 0)
            elif key == "i" or key == "I":
                cast.userFunction(2, 0)
            elif key == "c" or key == "C":
                cast.userFunction(3, 0)
            elif key == "d":
                cast.userFunction(4, 0)
            elif key == "D":
                cast.userFunction(5, 0)
            elif key == "g":
                cast.userFunction(6, 0)
            elif key == "G":
                cast.userFunction(7, 0)
        elif key == "d" or key == "D":
            ret = cast.disconnect()
            if ret:
                print("successful disconnect")
            else:
                print("disconnection failed")
        elif key == "s" or key == "S":
            global printStream
            printStream = not printStream
        elif key == "p" or key == "P":
            inp = input("enter: {parameter name} {value [float/true/false]}").split()
            if len(inp) != 2:
                print("please format as: {parameter name} {value [float/true/false]}")
            elif inp[1] == "true" or inp[1] == "false":
                cast.enableParam(inp[0], 1 if inp[1] == "true" else 0)
            elif "+" in inp[1]:
                cast.setPulse(inp[0], inp[1])
            else:
                cast.setParam(inp[0], float(inp[1]))

    cast.destroy()


if __name__ == "__main__":
    main()
