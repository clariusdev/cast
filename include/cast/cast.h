#pragma once

#include "cast_export.h"
#include "cast_cb.h"

/// initialization parameters for cusCastInit
typedef struct _CusInitParams
{    struct Args
    {
        int argc;       ///< the argument count for input parameters to pass to the library
        char** argv;    ///< the arguments to pass to the library, possibly required for qt graphics buffer initialization
    }
    args;
    const char* storeDir;                           ///< the directory to store security keys
    CusNewProcessedImageFn newProcessedImageFn;     ///< new processed image callback (scan-converted image)
    CusNewRawImageFn newRawImageFn;                 ///< new raw image callback (pre scan-converted image or rf data)
    CusNewSpectralImageFn newSpectralImageFn;       ///< new processed spectral image callback
    CusNewImuDataFn newImuDataFn;                   ///< new imu data callback
    CusFreezeFn freezeFn;                           ///< freeze state callback
    CusButtonFn buttonFn;                           ///< button press callback
    CusProgressFn progressFn;                       ///< readback progress callback
    CusErrorFn errorFn;                             ///< error message callback
    int width;                                      ///< the width of the output buffer
    int height;                                     ///< the height of the output buffer

} CusInitParams;

#ifdef __cplusplus
extern "C" {
#endif

    /// initializes the casting functionality
    /// @param[in] params the sdk configuration parameters
    /// @return success of the call
    /// @retval 0 the initialization was successful
    /// @retval -1 the initialization was not successful
    /// @note must be called before any other functions will succeed
    CAST_EXPORT int cusCastInit(const CusInitParams* params);

    /// get init params with default values
    /// @return a zero initialized struct
    CAST_EXPORT CusInitParams cusCastDefaultInitParams(void);

    /// cleans up memory allocated by the caster
    /// @retval 0 the destroy attempt was successful
    /// @retval -1 the destroy attempt was not successful
    /// @note should be called prior to exiting the application
    CAST_EXPORT int cusCastDestroy(void);

    /// retrieves the firmware version for a given platform
    /// @param[in] platform the platform to retrieve the firmware version for
    /// @param[out] version holds the firmware version for the given platform
    /// @param[in] sz size of the version string buffer, suggest at least 64 bytes allocated
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    CAST_EXPORT int cusCastFwVersion(CusPlatform platform, char* version, int sz);

    /// tries to connect to a probe that is on the same network as the caller
    /// @param[in] ipAddress the ip address of the probe
    /// @param[in] port the probe's tcp casting port
    /// @param[in] cert the certificate to authenticate the probe being connected to
    /// @param[in] fn callback to obtain success of call. the return value will be the udp port used if successful
    /// @return success of the call
    /// @note pass "research" as the certificate to bypass authentication
    /// @retval 0 the connection attempt was successful
    /// @retval -1 the connection attempt was not successful
    CAST_EXPORT int cusCastConnect(const char* ipAddress, unsigned int port, const char* cert, CusConnectFn fn);

    /// disconnects from an existing connection
    /// @param[in] fn callback to obtain success of call
    /// @return success of the call
    /// @retval 0 the disconnect to the specified connection was successful
    /// @retval -1 the disconnect was unsuccessful
    CAST_EXPORT int cusCastDisconnect(CusReturnFn fn);

    /// retrieves the current connected state of the module
    /// @return the connected state of the module
    /// @retval 0 there is currently no connection
    /// @retval 1 there is currently a connection
    /// @retval -1 the module is not initialized
    CAST_EXPORT int cusCastIsConnected(void);

    /// retrieves the current probe information
    /// @param[out] info the probe information
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    CAST_EXPORT int cusCastProbeInfo(CusProbeInfo* info);

    /// sets the dimensions of the output display for scan conversion
    /// @param[in] w the number of pixels in the horizontal direction
    /// @param[in] h the number of pixels in the vertical direction
    /// @return success of the call
    /// @retval 0 the output size was successfully programmed
    /// @retval -1 the output size could not be set
    /// @note the output will always result in a 1:1 pixel ratio, depending on geometry of scanning array, and parameters
    ///       the frame will have various sizes of black borders around the image
    CAST_EXPORT int cusCastSetOutputSize(int w, int h);

    /// sets a flag to separate overlays into separate images, for example if color/power Doppler or strain
    /// imaging is enabled, two callbacks will be generated, one with the grayscale frame, and the other with the overlay
    /// @param[in] en the enable flag for separating overlays
    /// @return success of the call
    /// @retval 0 the flag was successfully programmed
    /// @retval -1 the flag could not be set
    CAST_EXPORT int cusCastSeparateOverlays(int en);

    /// sets the format for processed images, by default the format will be raw argb
    /// @param[in] format the format of the image
    /// @return success of the call
    /// @retval 0 the format was successfully set
    /// @retval -1 the format could not be set
    CAST_EXPORT int cusCastSetFormat(CusImageFormat format);

    /// makes a request to return the availability of all the raw data currently buffered on the probe
    /// @param[in] fn result callback function that will return all the timestamps of the data blocks that are buffered
    /// @return success of the call
    /// @retval 0 the request was successfully made
    /// @retval -1 the request could not be made
    /// @note the probe must be frozen with raw data buffering enabled prior to calling the function
    CAST_EXPORT int cusCastRawDataAvailability(CusRawAvailabilityFn fn);

    /// makes a request for raw data from the probe
    /// @param[in] start the first frame to request, as determined by timestamp in nanoseconds, set to 0 along with end to requests all data in buffer
    /// @param[in] end the last frame to request, as determined by timestamp in nanoseconds, set to 0 along with start to requests all data in buffer
    /// @param[in] lzo flag to specify a tarball with lzo compressed raw data inside (default) vs no compression of raw data
    /// @param[in] fn result callback function, will return size of buffer required upon success, 0 if no raw data was buffered, or -1 if request could not be made
    /// @return success of the call
    /// @retval 0 the request was successfully made
    /// @retval -1 the request could not be made
    /// @note the probe must be frozen and in a raw data buffering mode in order for the call to succeed
    CAST_EXPORT int cusCastRequestRawData(long long int start, long long int end, int lzo, CusRawRequestFn fn);

    /// retrieves raw data from a previous request
    /// @param[out] data a pointer to a buffer that has been allocated to read the raw data into, this must be pre-allocated with
    ///             the size returned from a previous call to cusCastRequestRawData
    /// @param[in] fn callback to obtain success of call, upon returning 0, the data pointer can be accessed for the size resulting from the raw data request
    /// @return success of the call
    /// @retval 0 the data was successfully read into the buffer
    /// @retval -1 the data could not be read
    /// @note the probe must be frozen and a successful call to cusCastRequestRawData must have taken place in order for the call to succeed
    CAST_EXPORT int cusCastReadRawData(void** data, CusRawFn fn);

    /// performs a user function on a connected probe
    /// @param[in] cmd the user command to run
    /// @param[in] val the value to set if the function supports setting of values
    ///            SetDepth supports setting depth in cm
    ///            SetGain supports setting gain in %
    /// @param[in] fn callback to obtain success of call
    /// @return success of the call
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    CAST_EXPORT int cusCastUserFunction(CusUserFunction cmd, double val, CusReturnFn fn);

    /// sets a parameter to a specific value for researchers to gain access to lower level control of device
    /// @param[in] prm the parameter to change
    /// @param[in] val the value to set the parameter to
    /// @param[in] fn callback to obtain success of call
    /// @return success of the call
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    /// @note see external documentation for supported parameters
    /// @warning changing parameters through this function may result in unstable operation, degradation of image quality, or operation outside of the safety limits that clarius tests to
    CAST_EXPORT int cusCastSetParameter(const char* prm, double val, CusReturnFn fn);

    /// enables or disables a parameter for researchers to gain access to lower level control of device
    /// @param[in] prm the parameter to change
    /// @param[in] en the enable flag, 0 to disable, 1 to enable
    /// @param[in] fn callback to obtain success of call
    /// @return success of the call
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    /// @note see external documentation for supported parameters
    /// @warning changing parameters through this function may result in unstable operation, degradation of image quality, or operation outside of the safety limits that clarius tests to
    CAST_EXPORT int cusCastEnableParameter(const char* prm, int en, CusReturnFn fn);

    /// sets a pulse shape parameter to a specific value for researchers to gain access to lower level control of device
    /// @param[in] prm the parameter to change
    /// @param[in] shape the shape to set the pulse as
    /// @param[in] fn callback to obtain success of call
    /// @return success of the call
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    /// @note see external documentation for supported parameters
    /// @warning changing parameters through this function may result in unstable operation, degradation of image quality, or operation outside of the safety limits that clarius tests to
    CAST_EXPORT int cusCastSetPulse(const char* prm, const char* shape, CusReturnFn fn);

    /// begins a capture with the current image settings
    /// @param[in] timestamp the timestamp of the frame to be captured
    /// @retval the ID of the newly created capture
    /// @retval -1 the call was not successful
    CAST_EXPORT int cusCastStartCapture(long long int timestamp);

    /// adds an image overlay to a capture which was started with cusCastStartCapture
    /// @param[in] id the ID of the capture which is to be added to
    /// @param[in] data pointer to the image data which is to be added (assumed 8-bit grayscale)
    /// @param[in] width width of the image data which is to be added; must match the width from cusCastInit
    /// @param[in] height height of the image data which is to be added; must match the height from cusCastInit
    /// @param[in] red the value of red use in colorizing the overlay, should be between 0.0 and 1.0
    /// @param[in] green the value of green use in colorizing the overlay, should be between 0.0 and 1.0
    /// @param[in] blue the value of blue use in colorizing the overlay, should be between 0.0 and 1.0
    /// @param[in] alpha the value of alpha (opacity) use in colorizing the overlay, should be between 0.0 and 1.0
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    CAST_EXPORT int cusCastAddImageOverlay(int id, const void* data, int width, int height, float red, float green, float blue, float alpha);

    /// adds a label overlay to a capture which was started with cusCastStartCapture
    /// @param[in] id the ID of the capture which is to be added to
    /// @param[in] text string of the label to be added
    /// @param[in] x x-position of the label in pixels (same scale as the width from cusCastInit)
    /// @param[in] y y-position of the label in pixels (same scale as the height from cusCastInit)
    /// @param[in] width width of the label in pixels (same scale as the width from cusCastInit)
    /// @param[in] height height of the label in pixels (same scale as the height from cusCastInit)
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    CAST_EXPORT int cusCastAddLabelOverlay(int id, const char* text, double x, double y, double width, double height);

    /// adds a 2-point distance or trace measurement to a capture which was started with cusCastStartCapture
    /// @details for a distance measurement, 2 points should be given
    /// @param[in] id the ID of the capture which is to be added to
    /// @param[in] type the type of measurement to add
    /// @param[in] label string label for the measurement
    /// @param[in] pts pointer to an array of doubles (x,y positions pixels, same scale as the width/height from cusCastInit)
    /// @param[in] count number of *doubles* in the pts array (not number of points)
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    CAST_EXPORT int cusCastAddMeasurement(int id, CusMeasurementType type, const char* label, const double* pts, int count);

    /// completes a capture which was started with cusCastStartCapture
    /// @param[in] id the ID of the capture which is to be finished
    /// @param[in] fn callback to obtain the success of the call, -1 if the request failed, 0 if it succeeded
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    CAST_EXPORT int cusCastFinishCapture(int id, CusReturnFn fn);

#ifdef __cplusplus
}
#endif
