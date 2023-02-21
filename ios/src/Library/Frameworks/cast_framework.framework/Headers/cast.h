#pragma once

#import <Foundation/Foundation.h>

#include "api_common.h"
#include "cast_def.h"

/// asynchronous result callback function
/// @param[in] result the result (success/fail)
typedef void (^CusResultFn)(BOOL result);
/// asynchronous result callback function
/// @param[in] result the result (success/fail)
/// @param[in] port the UDP port that will be used for streaming (-1 if failed)
/// @param[in] swRevMatch whether the sw revisions between the api and the clarius app match
typedef void (^CusConnectFn)(BOOL result, int port, BOOL swRevMatch);
/// button callback function
/// @param[in] btn the button that was pressed
/// @param[in] clicks # of clicks performed
typedef void (^CusButtonFn)(CusButton btn, int clicks);
/// progress callback function
/// @param[in] progress the current progress in percent
typedef void (^CusProgressFn)(int progress);
/// probe info callback function
/// @param[in] info the probe info
typedef void (^CusProbeInfoFn)(const CusProbeInfo* _Nullable info);
/// firmware callback function
/// @param[in] firmware the firmware version for the given platform, empty if an error occurred
typedef void (^CusFirmwareFn)(NSString * _Nullable firmware);
/// new data callback function
/// @param[in] img pointer to the new grayscale image information
/// @param[in] nfo image information associated with the image data
/// @param[in] npos number of positional information data tagged with the image
/// @param[in] pos the positional information data tagged with the image
typedef void (^CusNewRawImageFn)(NSData * _Nonnull img, const CusRawImageInfo * _Nonnull nfo, int npos, const CusPosInfo * _Nonnull pos);
/// new image callback function
/// @param[in] img pointer to the new grayscale image information
/// @param[in] nfo image information associated with the image data
/// @param[in] npos number of positional information data tagged with the image
/// @param[in] pos the positional information data tagged with the image
typedef void (^CusNewProcessedImageFn)(NSData * _Nonnull img, const CusProcessedImageInfo * _Nonnull nfo, int npos, const CusPosInfo * _Nonnull pos);
/// new spectral image callback function
/// @param[in] img pointer to the new grayscale image information
/// @param[in] nfo image information associated with the image data
typedef void (^CusNewSpectralImageFn)(NSData * _Nonnull img, const CusSpectralImageInfo * _Nonnull nfo);
/// error callback function
/// @param[in] msg the error message with associated error that occurred
typedef void (^CusErrorFn)(NSString * _Nonnull msg);
/// freeze callback function
/// @param[in] frozen true if imaging is currently frozen
typedef void (^CusFreezeFn)(BOOL frozen);
/// raw data size callback function
/// @param[in] res the size of the data package requested or actually downloaded or -1 if an error occurred
typedef void (^CusRawFn)(int res);
/// raw data callback function
/// @param[in] res the raw data result, the size of the data package or -1 if an error occurred
/// @param[in] data the raw data
typedef void (^CusRawDataFn)(int res, NSData * _Nullable data);
/// capture creation callback function
/// @param[in] res the new capture ID, or -1 if an error occurred
typedef void (^CusCaptureFn)(int res);

//! @brief Cast interface
__attribute__((visibility("default"))) @interface CusCast : NSObject

//! @brief initializes the class to enable connection.
//! @param[in] dir the directory to store security keys
/// @param[in] callback the callback function that reports success/failure
- (void)initialize:(NSString * _Nonnull) dir
        callback:(CusResultFn _Nonnull) callback;
//! @brief checks whether the class has been initialized.
//! @return YES if the class has been initialized.
- (BOOL)isInitialized;
/// connects to a probe that is on the same network as the caller
/// @param[in] address the ip address of the probe
/// @param[in] port the probe's tcp port to connect to
/// @param[in] cert the certificate for authenticating the probe
/// @param[in] callback the callback function that reports success/failure
- (void)connect:(NSString * _Nonnull) address
            port:(unsigned int) port
            cert:(NSString * _Nonnull) cert
        callback:(CusConnectFn _Nonnull) callback;
/// disconnects from an existing connection
/// @param[in] callback the callback function that reports success/failure
- (void)disconnect:(CusResultFn _Nonnull) callback;
/// retrieves the current connected state of the module
/// @return the connected state of the module
- (BOOL)isConnected;
/// sets the dimensions of the output display for scan conversion
/// @param[in] width the number of horizontal pixels in the output
/// @param[in] height the number of vertical pixels in the output
/// @note the output will always result in a 1:1 pixel ratio, depending on geometry of scanning array, and parameters
///       the frame will have various sizes of black borders around the image
- (void)setOutputWidth:(int) width
            withHeight:(int) height;
/// sets a flag to separate overlays into separate images, for example if color/power Doppler or strain
/// imaging is enabled, two callbacks will be generated, one with the grayscale frame, and the other with the overlay
/// @param[in] enable the enable flag for separating overlays
- (void)separateOverlays:(BOOL) enable;
/// sets the format for processed images, by default the format will be uncompressed argb
/// @param[in] format the format of the image
- (void)setFormat:(CusImageFormat) format;
/// makes a request for raw data from the probe
/// @param[in] start the first frame to request, as determined by timestamp in nanoseconds, set to 0 along with end to requests all data in buffer
/// @param[in] end the last frame to request, as determined by timestamp in nanoseconds, set to 0 along with start to requests all data in buffer
/// @param[in] res result callback function, will return size of buffer required upon success, 0 if no raw data was buffered, or -1 if request could not be made,
/// @note the probe must be frozen and in a raw data buffering mode in order for the call to succeed
- (void)requestRawData:(CusRawFn _Nonnull) res
                start:(long long int) start
                end:(long long int) end;
/// retrieves raw data from a previous request
/// @param[in] res result callback function, will return size of buffer required upon success, 0 if no raw data was buffered, or -1 if request could not be made,
/// @note the probe must be frozen and a successful call to requestRawData must have taken place in order for the call to succeed
- (void)readRawData:(CusRawDataFn _Nonnull) res;

/// performs a user function on a connected probe
/// @param[in] command the user command to run
/// @param[in] value the value to set if the function supports setting of values
///            SetDepth supports setting depth in cm
///            SetGain supports setting gain in %
/// @param[in] callback the callback function that reports success/failure
- (void) userFunction:(CusUserFunction) command
                value:(double) value
             callback:(CusResultFn _Nonnull) callback;
/// retrieves the probe info
/// @param[in] callback callback to receive the probe info
- (void)getProbeInfo:(CusProbeInfoFn _Nonnull) callback;
/// retrieves the firmware version for a given platform
/// @param[in] platform the platform to retrieve the firmware version for
/// @param[in] callback callback to receive the firmware version
- (void) getFirmwareVersion:(CusPlatform) platform
                   callback:(CusFirmwareFn _Nonnull) callback;

/// begins a capture with the current image settings
/// @param[in] timestamp the timestamp of the frame to be captured
/// @param[in] callback the callback function that receives the new capture ID or -1 if an error occurred
- (void)startCapture:(long long int) timestamp
            callback:(CusCaptureFn _Nonnull) callback;

/// adds an image overlay to a capture which was started with startCapture
/// @param[in] captureID the ID of the capture which is to be added to
/// @param[in] data pointer to the image data which is to be added (assumed 8-bit grayscale)
/// @param[in] width width of the image data which is to be added; must match the width from cusCastInit
/// @param[in] height height of the image data which is to be added; must match the height from cusCastInit
/// @param[in] red the value of red use in colorizing the overlay, should be between 0.0 and 1.0
/// @param[in] green the value of green use in colorizing the overlay, should be between 0.0 and 1.0
/// @param[in] blue the value of blue use in colorizing the overlay, should be between 0.0 and 1.0
/// @param[in] alpha the value of alpha (opacity) use in colorizing the overlay, should be between 0.0 and 1.0
/// @retval whether the call was successful
- (BOOL)addImageOverlay:(int) captureID
                   data:(NSData * _Nonnull) data
                  width:(int) width
                 height:(int) height
                    red:(float) red
                  green:(float) green
                   blue:(float) blue
                  alpha:(float) alpha;

/// adds a label overlay to a capture which was started with startCapture
/// @param[in] captureID the ID of the capture which is to be added to
/// @param[in] text string of the label to be added
/// @param[in] x x-position of the label in pixels (same scale as the width from cusCastInit)
/// @param[in] y y-position of the label in pixels (same scale as the height from cusCastInit)
/// @param[in] width width of the label in pixels (same scale as the width from cusCastInit)
/// @param[in] height height of the label in pixels (same scale as the height from cusCastInit)
/// @retval whether the call was successful
- (BOOL)addLabelOverlay:(int) captureID
                   text:(NSString * _Nonnull) text
                      x:(double) x
                      y:(double) y
                  width:(double) width
                 height:(double) height;

/// adds a 2-point distance or trace measurement to a capture which was started with startCapture
/// @details for a distance measurement, 2 points should be given
/// @param[in] captureID the ID of the capture which is to be added to
/// @param[in] type the type of measurement to be added
/// @param[in] label string label for the measurement
/// @param[in] points NSValue holding an array of CGPoint/NSPoint in pixels (same scale as the width/height from cusCastInit)
/// @retval whether the call was successful
- (BOOL)addMeasurement:(int) captureID
                  type:(CusMeasurementType) type
                 label:(NSString * _Nonnull) label
                points:(NSArray * _Nonnull) points;

/// completes a capture which was started with startCapture
/// @param[in] captureID the ID of the capture which is to be finished
/// @param[in] callback function to obtain the success of the capture creation
/// @retval whether the sending of the capture could be started
- (BOOL)finishCapture:(int) captureID
             callback:(CusResultFn _Nonnull) callback;

- (void)setNewRawImageCallback:(CusNewRawImageFn _Nullable) newRawImageCallback;
- (void)setNewProcessedImageCallback:(CusNewProcessedImageFn _Nullable) newProcessedImageCallback;
- (void)setNewSpectralImageCallback:(CusNewSpectralImageFn _Nullable) newSpectralImageCallback;
- (void)setFreezeCallback:(CusFreezeFn _Nullable) freezeCallback;
- (void)setButtonCallback:(CusButtonFn _Nullable) buttonCallback;
- (void)setProgressCallback:(CusProgressFn _Nullable) progressCallback;
- (void)setErrorCallback:(CusErrorFn _Nullable) errorCallback;

@end
