#pragma once
#include "listen_export.h"
#include "listen_def.h"

extern "C"
{
    /// initializes the listener functionality
    /// @param[in] argc the argument count for input parameters to pass to the library
    /// @param[in] argv the arguments to pass to the library, possibly required for qt graphics buffer initialization
    /// @param[in] dir the directory to store security keys
    /// @param[in] newProcessedImage new processed image callback (scan-converted image)
    /// @param[in] newRawImage new raw image callback - (pre scan-converted image)
    /// @param[in] freeze freeze state callback
    /// @param[in] btn button press callback
    /// @param[in] progress readback progress callback
    /// @param[in] err error message callback
    /// @param[in] fn callback to obtain success of call, if set to null, then the call will block
    /// @return success of the call
    /// @retval 0 the initialization was successful
    /// @retval -1 the initialization was not successful
    /// @note must be called before any other functions will succeed
    LISTEN_EXPORT int clariusInitListener(int argc,
        char** argv,
        const char* dir,
        ClariusNewProcessedImageFn newProcessedImage,
        ClariusNewRawImageFn newRawImage,
        ClariusFreezeFn freeze,
        ClariusButtonFn btn,
        ClariusProgressFn progress,
        ClariusErrorFn err,
        ClariusReturnFn fn,
        int width,
        int height
    );

    /// cleans up memory allocated by the listener
    /// @retval 0 the destroy attempt was successful
    /// @retval -1 the destroy attempt was not successful
    /// @note should be called prior to exiting the application
    LISTEN_EXPORT int clariusDestroyListener();

    /// tries to connect to a scanner that is on the same network as the caller
    /// @param[in] ipAddress the ip address of the ultrasound scanner
    /// @param[in] port the tcp port that the scanner's listener is setup on
    /// @param[in] fn callback to obtain success of call, if set to null, then the call will block
    /// @return success of the call
    /// @retval 0 the connection attempt was successful
    /// @retval -1 the connection attempt was not successful
    LISTEN_EXPORT int clariusConnect(const char* ipAddress, unsigned int port, ClariusReturnFn fn);

    /// disconnects from an existing connection
    /// @param[in] fn callback to obtain success of call, if set to null, then the call will block
    /// @return success of the call
    /// @retval 0 the disconnect to the specified connection was successful
    /// @retval -1 the disconnect was unsuccessful
    LISTEN_EXPORT int clariusDisconnect(ClariusReturnFn fn);

    /// retrieves the current connected state of the module
    /// @return the connected state of the module
    /// @retval 0 there is currently no connection
    /// @retval 1 there is currently a connection
    /// @retval -1 the module is not initialized
    LISTEN_EXPORT int clariusIsConnected();

    /// sets the dimensions of the output display for scan conversion
    /// @param[in] w the number of pixels in the horizontal direction
    /// @param[in] h the number of pixels in the vertical direction
    /// @return success of the call
    /// @retval 0 the output size was successfully programmed
    /// @retval -1 the output size could not be set
    /// @note if never called, the default resolution is 640x480
    /// @note the output will always result in a 1:1 pixel ratio, depending on geometry of scanning array, and parameters
    ///       the frame will have various sizes of black borders around the image
    LISTEN_EXPORT int clariusSetOutputSize(int w, int h);

    /// sets the callback for when new raw images are acquired and streamed to the listener
    /// @param[in] fn a pointer to the callback function
    /// @return success of the call
    /// @retval 0 the callback function was successfully set
    /// @retval -1 the callback function could not be set
    LISTEN_EXPORT int clariusSetNewRawImageFn(ClariusNewRawImageFn fn);

    /// sets the callback for when new processed images are acquired and streamed to the listener
    /// @param[in] fn a pointer to the callback function
    /// @return success of the call
    /// @retval 0 the callback function was successfully set
    /// @retval -1 the callback function could not be set
    LISTEN_EXPORT int clariusSetNewProcessedImageFn(ClariusNewProcessedImageFn fn);

    /// sets the callback for when there is a change in the freeze state
    /// @param[in] fn a pointer to the callback function
    /// @return success of the call
    /// @retval 0 the callback function was successfully set
    /// @retval -1 the callback function could not be set
    LISTEN_EXPORT int clariusSetFreezeFn(ClariusFreezeFn fn);

    /// sets the callback for when there is a button press
    /// @param[in] fn a pointer to the callback function
    /// @return success of the call
    /// @retval 0 the callback function was successfully set
    /// @retval -1 the callback function could not be set
    LISTEN_EXPORT int clariusSetButtonFn(ClariusButtonFn fn);

    /// sets the callback for when there is a change in the readback progress
    /// @param[in] fn a pointer to the callback function
    /// @return success of the call
    /// @retval 0 the callback function was successfully set
    /// @retval -1 the callback function could not be set
    LISTEN_EXPORT int clariusSetProgressFn(ClariusProgressFn fn);

    /// sets the callback for when an error occurs
    /// @param[in] fn a pointer to the callback function
    /// @return success of the call
    /// @retval 0 the callback function was successfully set
    /// @retval -1 the callback function could not be set
    LISTEN_EXPORT int clariusSetErrorFn(ClariusErrorFn fn);

    /// retrieves the udp port that will be used for streaming images after a successful connection
    /// @return the udp port that will be used for streaming images
    /// @retval -1 no valid udp port exists
    LISTEN_EXPORT int clariusGetUdpPort();

    /// makes a request for raw data from the scanner
    /// @param[in] start the first frame to request, as determined by timestamp in nanoseconds, set to 0 along with end to requets all data in buffer
    /// @param[in] end the last frame to request, as determined by timestamp in nanoseconds, set to 0 along with start to requets all data in buffer
    /// @param[in] fn callback to obtain success of call, if set to null, then the call will block
    /// @return package size of raw data, 0 if no data, or -1 or not successful
    /// @note the scanner must be frozen and in a raw data buffering mode in order for the call to succeed
    LISTEN_EXPORT int clariusRequestRawData(long long int start, long long int end, ClariusReturnFn fn);

    /// retrieves raw data from a previous request
    /// @param[out] data a pointer to a buffer that has been allocated to read the raw data into, this must be pre-allocated with
    ///             the size returned from a previous call to clariusRequestRawData
    /// @param[in] fn callback to obtain success of call, if set to null, then the call will block
    /// @return success of the call
    /// @retval 0 the data was successfully read into the buffer
    /// @retval -1 the data could not be read
    /// @note the scanner must be frozen and a successful call to clariusRequestRawData must have taken place in order for the call to succeed
    LISTEN_EXPORT int clariusReadRawData(void** data, ClariusReturnFn fn);

    /// performs a user function on a connected scanner
    /// @param[in] userFn the function to run
    /// @param[in] fn callback to obtain success of call, if set to null, then the call will block
    /// @return success of the call
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    LISTEN_EXPORT int clariusUserFunction(int userFn, ClariusReturnFn fn);
}
