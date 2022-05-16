#pragma once

/// image formats
typedef enum _CusImageFormat
{
    Uncompressed,   ///< processed images are sent in a raw and uncompressed in 32 bit argb
    Jpeg,           ///< processed images are sent as a jpeg
    Png             ///< processed images are sent as a png

} CusImageFormat;

/// user function commands
typedef enum _CusUserFunction
{
    Freeze = 1,         ///< toggle freeze
    CaptureImage = 2,   ///< capture still image
    CaptureCine,        ///< capture cine
    DepthDec,           ///< decrease depth
    DepthInc,           ///< increase depth
    GainDec,            ///< increase gain
    GainInc,            ///< decrease gain
    AutoGain,           ///< toggle auto gain
    Zoom,               ///< toggle zoom
    Flip,               ///< toggle horizontal flip
    PlayCine,           ///< play cine when frozen
    BMode,              ///< enter b mode
    MMode,              ///< enter m mode
    ColorDoppler,       ///< enter color doppler mode
    PowerDoppler,       ///< enter power doppler mode
    PwDoppler,          ///< enter pulsed wave doppler mode (when available)
    NeedleEnhance,      ///< enter needle enhance mode (when available)
    Strain,             ///< enter strain elastography mode (when available)
    RfMode,             ///< enter rf mode (when available)
    NeedleSide,         ///< toggle needle enhance side
    SetDepth,           ///< set depth in cm
    SetGain,            ///< set gain in %
    CenterGuide,        ///< toggle center guide
    FullScreen          ///< toggle full screen

} CusUserFunction;

/// raw image information supplied with each frame
typedef struct _CusRawImageInfo
{
    int lines;              ///< number of ultrasound lines in the image
    int samples;            ///< number of samples per line in the image
    int bitsPerSample;      ///< bits per sample
    double axialSize;       ///< axial microns per sample
    double lateralSize;     ///< lateral microns per line
    long long int tm;       ///< timestamp of image
    int jpeg;               ///< size of the jpeg image, 0 if not a jpeg compressed image
    int rf;                 ///< flag specifying data is rf and not envelope

} CusRawImageInfo;

/// processed image information supplied with each frame
typedef struct _CusProcessedImageInfo
{
    int width;              ///< width of the image in pixels
    int height;             ///< height of the image in pixels
    int bitsPerPixel;       ///< bits per pixel
    int imageSize;          ///< total size of image
    double micronsPerPixel; ///< microns per pixel (always 1:1 aspect ratio axially/laterally)
    double originX;         ///< image origin in microns in the horizontal axis
    double originY;         ///< image origin in microns in the vertical axis
    long long int tm;       ///< timestamp of images
    int overlay;            ///< flag that the image is an overlay without grayscale (ie. color doppler or strain)
    CusImageFormat format;  ///< flag specifying the format of the image (see format definitions above)

} CusProcessedImageInfo;

/// spectral image information supplied with each block
typedef struct _CusSpectralImageInfo
{
    int lines;                  ///< number of lines in the block
    int samples;                ///< number of samples per line
    int bitsPerSample;          ///< bits per sample
    double period;              ///< line acquisition period in seconds
    double micronsPerSample;    ///< microns per pixel/sample in an m spectrum
    double velocityPerSample;   ///< velocity in m/s per pixel/sample in a pw spectrum
    int pw;                     ///< flag specifying the data is pw and not m

} CusSpectralImageInfo;

/// probe information
typedef struct _CusProbeInfo
{
    int version;    ///< version (1 = clarius 1st generation, 2 = clarius HD, 3 = clarius HD3)
    int elements;   ///< # of probe elements
    int pitch;      ///< element pitch
    int radius;     ///< radius in mm

} CusProbeInfo;

/// positional data information structure
typedef struct _CusPosInfo
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

} CusPosInfo;

/// return status callback function
/// @param[in] retCode the return code
typedef void (*CusReturnFn)(int retCode);
/// new data callback function
/// @param[in] img pointer to the new grayscale image information
/// @param[in] nfo image information associated with the image data
/// @param[in] npos number of positional information data tagged with the image
/// @param[in] pos the positional information data tagged with the image
typedef void (*CusNewRawImageFn)(const void* img, const CusRawImageInfo* nfo, int npos, const CusPosInfo* pos);
/// new image callback function
/// @param[in] img pointer to the new grayscale image information
/// @param[in] nfo image information associated with the image data
/// @param[in] npos number of positional information data tagged with the image
/// @param[in] pos the positional information data tagged with the image
typedef void (*CusNewProcessedImageFn)(const void* img, const CusProcessedImageInfo* nfo, int npos, const CusPosInfo* pos);
/// new spectral image callback function
/// @param[in] img pointer to the new grayscale image information
/// @param[in] nfo image information associated with the image data
typedef void (*CusNewSpectralImageFn)(const void* img, const CusSpectralImageInfo* nfo);
/// freeze callback function
/// @param[in] state 1 = frozen, 0 = imaging
typedef void (*CusFreezeFn)(int state);
/// button callback function
/// @param[in] btn 0 = up, 1 = down
/// @param[in] clicks # of clicks performed
typedef void (*CusButtonFn)(int btn, int clicks);
/// progress callback function
/// @param[in] progress the current progress
typedef void (*CusProgressFn)(int progress);
/// error callback function
/// @param[in] msg the error message with associated error that occurred
typedef void (*CusErrorFn)(const char* msg);

