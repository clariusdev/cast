#pragma once

/// raw image information supplied with each frame
typedef struct _ClariusRawImageInfo
{
    int lines;              ///< number of ultrasound lines in the image
    int samples;            ///< number of samples per line in the image
    int bitsPerSample;      ///< bits per sample
    double axialSize;       ///< axial microns per sample
    double lateralSize;     ///< lateral microns per line
    long long int tm;       ///< timestamp of image
    int jpeg;               ///< size of the jpeg image, 0 if not a jpeg compressed image

} ClariusRawImageInfo;

/// processedd image information supplied with each frame
typedef struct _ClariusProcessedImageInfo
{
    int width;              ///< width of the image in pixels
    int height;             ///< height of the image in pixels
    int bitsPerPixel;       ///< bits per pixel of the image
    double micronsPerPixel; ///< microns per pixel (always 1:1 aspect ratio axially/laterally)
    long long int tm;       ///< timestamp of imagesed

} ClariusProcessedImageInfo;

/// positional data information structure
typedef struct _ClariusPosInfo
{
    long long int tm;   ///< timestamp in nanoseconds
    double gx;          ///< gyroscope x; angular velocity is given in radians per second (rps)
    double gy;          ///< gyroscope y
    double gz;          ///< gyroscope z
    double ax;          ///< accelerometer x; acceleration is normalized to gravity [~9.81m/s^2] (m/s^2)/(m/s^2)
    double ay;          ///< accelerometer y
    double az;          ///< accelerometer z
    double mx;          ///< magnetometer x; magnetic flux density is normalized to the earth's field [~50 mT] (T/T)
    double my;          ///< magnetometer y
    double mz;          ///< magnetometer z
    double qw;          ///< w component (real) of the orientation quaternion
    double qx;          ///< x component (imaginary) of the orientation quaternion
    double qy;          ///< y component (imaginary) of the orientation quaternion
    double qz;          ///< z component (imaginary) of the orientation quaternion

} ClariusPosInfo;

/// return status callback function
/// @param[in] retCode the return code
typedef void (*ClariusReturnFn)(int retCode);
/// new data callback function
/// @param[in] newImage pointer to the new greyscale image information
/// @param[in] nfo image information associated with the image data
/// @param[in] npos number of positional information data tagged with the image
/// @param[in] pos the positional information data tagged with the image
typedef void (*ClariusNewRawImageFn)(const void* newImage, const ClariusRawImageInfo* nfo, int npos, const ClariusPosInfo* pos);
/// new image callback function
/// @param[in] newImage pointer to the new greyscale image information
/// @param[in] nfo image information associated with the image data
/// @param[in] npos number of positional information data tagged with the image
/// @param[in] pos the positional information data tagged with the image
typedef void (*ClariusNewProcessedImageFn)(const void* newImage, const ClariusProcessedImageInfo* nfo, int npos, const ClariusPosInfo* pos);
/// freeze callback function
/// @param[in] state 1 = frozen, 0 = imaging
typedef void (*ClariusFreezeFn)(int state);
/// button callback function
/// @param[in] btn 0 = up, 1 = down
/// @param[in] clicks # of clicks performed
typedef void (*ClariusButtonFn)(int btn, int clicks);
/// progress callback function
/// @param[in] progress the current progress
typedef void (*ClariusProgressFn)(int progress);
/// error callback function
/// @param[in] msg the error message with associated error that occurred
typedef void (*ClariusErrorFn)(const char* msg);

/// user function commands
#define USER_FN_NONE                0
#define USER_FN_TOGGLE_FREEZE       1
#define USER_FN_CAPTURE_IMAGE       2
#define USER_FN_CAPTURE_CINE        3
#define USER_FN_DEPTH_DEC           4
#define USER_FN_DEPTH_INC           5
#define USER_FN_GAIN_DEC            6
#define USER_FN_GAIN_INC            7
#define USER_FN_TOGGLE_AUTOGAIN     8
#define USER_FN_TOGGLE_ZOOM         9
#define USER_FN_TOGGLE_FLIP         10
#define USER_FN_TOGGLE_CINE_PLAY    11
#define USER_FN_MODE_B              12
#define USER_FN_MODE_M              13
#define USER_FN_MODE_CFI            14
#define USER_FN_MODE_PDI            15
#define USER_FN_MODE_PW             16
#define USER_FN_MODE_ELASTOGRAPHY   17
