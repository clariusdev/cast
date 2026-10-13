#!/usr/bin/env python
"""Records IMU data streamed from a probe into a scenario-tagged CSV file.

Intended for characterizing the stationary detection thresholds with real recordings
(e.g. probe on a table vs held as still as possible vs scanning), but generally useful
for any offline analysis of the inertial stream. Keep the probe imaging for the full
capture so the IMU stays powered.

Example:
    python pyimucapture.py -a 192.168.1.1 -p 5828 -s on_table -d 120
"""

import argparse
import csv
import ctypes
import math
import os.path
import sys
import time

if sys.platform.startswith("linux"):
    libcast_handle = ctypes.CDLL("./libcast.so", ctypes.RTLD_GLOBAL)._handle  # load the libcast.so shared library
    pyclariuscast = ctypes.cdll.LoadLibrary("./pyclariuscast.so")  # load the pyclariuscast.so shared library

import pyclariuscast

FIELDS = ["tm", "gx", "gy", "gz", "ax", "ay", "az", "mx", "my", "mz", "qw", "qx", "qy", "qz"]

# samples keyed by timestamp: the same sample can arrive via both the dedicated imu stream
# and tagged onto image frames, so the dict de-duplicates whichever paths are active
samples = {}
frozen = False


## stores imu samples, de-duplicated by timestamp
# @param imu list of imu data
def record(imu):
    for s in imu:
        samples[s.tm] = [s.tm, s.gx, s.gy, s.gz, s.ax, s.ay, s.az, s.mx, s.my, s.mz, s.qw, s.qx, s.qy, s.qz]


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
    record(imu)


## called when a new raw image is streamed (unused)
def newRawImage(image, lines, samples, bps, axial, lateral, timestamp, jpg, rf, angle):
    return


## called when a new spectrum image is streamed (unused)
def newSpectrumImage(image, lines, samples, bps, period, micronsPerSample, velocityPerSample, pw):
    return


## called when new imu data is streamed
# @param imu inertial data
def newImuData(imu):
    record(imu)


## called when freeze state changes
# @param frozen the freeze state
def freezeFn(state):
    global frozen
    frozen = state
    print("\nimaging frozen - imu may stop streaming" if state else "\nimaging running")


## called when a button is pressed (unused)
def buttonsFn(button, clicks):
    return


## prints simple statistics so a capture can be sanity-checked on the spot
# @param rows the collected imu samples
def printStats(rows):
    if len(rows) < 2:
        return

    durationSec = (rows[-1][0] - rows[0][0]) / 1e9
    rateHz = (len(rows) - 1) / durationSec if durationSec > 0 else 0.0
    accelMags = [math.sqrt(r[4] ** 2 + r[5] ** 2 + r[6] ** 2) for r in rows]
    gyroMags = [math.sqrt(r[1] ** 2 + r[2] ** 2 + r[3] ** 2) for r in rows]

    def meanStd(values):
        mean = sum(values) / len(values)
        var = sum((v - mean) ** 2 for v in values) / (len(values) - 1)
        return mean, math.sqrt(var)

    accelMean, accelStd = meanStd(accelMags)
    gyroMean, gyroStd = meanStd(gyroMags)
    print(f"samples:  {len(rows)} over {durationSec:.1f}s ({rateHz:.1f} Hz)")
    print(f"|accel|:  mean {accelMean:.4f} g, std {accelStd:.5f} g")
    print(f"|gyro|:   mean {gyroMean:.4f} rad/s ({math.degrees(gyroMean):.2f} dps), std {gyroStd:.5f} rad/s")


## main function
def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--address", "-a", dest="ip", help="ip address of probe.", required=True)
    parser.add_argument("--port", "-p", dest="port", type=int, help="port of the probe", required=True)
    parser.add_argument("--scenario", "-s", dest="scenario", help="scenario label used in the output filename")
    parser.add_argument("--duration", "-d", dest="duration", type=float, help="capture duration in seconds")
    parser.add_argument("--output", "-o", dest="output", help="output directory")
    parser.set_defaults(ip=None)
    parser.set_defaults(port=None)
    parser.set_defaults(scenario="capture")
    parser.set_defaults(duration=120.0)
    parser.set_defaults(output=".")
    args = parser.parse_args()

    if not args.ip or not args.port or args.port < 0:
        print("one or more arguments are invalid")
        parser.print_usage()
        return

    # get home path
    path = os.path.expanduser("~/")

    # initialize
    cast = pyclariuscast.Caster(newProcessedImage, newRawImage, newSpectrumImage, newImuData, freezeFn, buttonsFn)
    if not cast.init(path, 640, 480):
        print("initialization failed")
        return

    if not cast.connect(args.ip, args.port, "research"):
        print("connection failed")
        cast.destroy()
        return

    print(f"connected to {args.ip} on port {args.port}")
    print(f"capturing '{args.scenario}' for {args.duration:.0f}s (ctrl+c to stop early and keep the data)...")

    start = time.monotonic()
    try:
        while time.monotonic() - start < args.duration:
            time.sleep(1)
            elapsed = time.monotonic() - start
            print(f"\r{elapsed:5.0f}s  {len(samples)} samples{' (frozen!)' if frozen else ''}", end="")
    except KeyboardInterrupt:
        print("\nstopping early")

    # write the data before tearing down, so a shutdown issue can never cost a capture
    rows = [samples[tm] for tm in sorted(samples)]
    if rows:
        name = os.path.join(args.output, f"imu_{args.scenario}_{time.strftime('%Y%m%d-%H%M%S')}.csv")
        with open(name, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(FIELDS)
            writer.writerows(rows)
        print(f"\nwrote {name}")
        printStats(rows)
    else:
        print("\nno imu samples were collected - was the probe imaging with imu streaming enabled?")

    # destroy() tears down the connection and its streaming threads; matches the other console
    # examples. don't dlclose libcast here - unloading it while those threads unwind crashes.
    cast.destroy()


if __name__ == "__main__":
    main()
