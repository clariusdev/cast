#pragma once

#define CUS_MAXTGC  10
#define CUS_SUCCESS 0
#define CUS_FAILURE (-1)

/// major clarius platforms
typedef enum _CusPlatform
{
    V1,                         ///< first generation
    HD,                         ///< second generation
    HD3                         ///< third generation

} CusPlatform;

/// image formats
typedef enum _CusImageFormat
{
    Uncompressed,               ///< processed images are sent in a raw and uncompressed in 32 bit argb
    Jpeg,                       ///< processed images are sent as a jpeg
    Png                         ///< processed images are sent as a png

} CusImageFormat;

/// physical buttons
typedef enum _CusButton
{
    ButtonUp,                   ///< up button
    ButtonDown,                 ///< down button
    ButtonHandle                ///< handle button (custom probes only)

} CusButton;

/// tgc information
typedef struct _CusTgcInfo
{
    double depth;               ///< depth in millimeters
    double gain;                ///< gain in decibels

} CusTgcInfo;

/// raw image information supplied with each frame
typedef struct _CusRawImageInfo
{
    int lines;                  ///< number of ultrasound lines in the image
    int samples;                ///< number of samples per line in the image
    int bitsPerSample;          ///< bits per sample
    double axialSize;           ///< axial microns per sample
    double lateralSize;         ///< lateral microns per line
    long long int tm;           ///< timestamp of image
    int jpeg;                   ///< size of the jpeg image, 0 if not a jpeg compressed image
    int rf;                     ///< flag specifying data is rf and not envelope
    double angle;               ///< acquisition angle for volumetric data
    CusTgcInfo tgc[CUS_MAXTGC]; ///< tgc points

} CusRawImageInfo;

/// processed image information supplied with each frame
typedef struct _CusProcessedImageInfo
{
    int width;                  ///< width of the image in pixels
    int height;                 ///< height of the image in pixels
    int bitsPerPixel;           ///< bits per pixel
    int imageSize;              ///< total size of image
    double micronsPerPixel;     ///< microns per pixel (always 1:1 aspect ratio axially/laterally)
    double originX;             ///< image origin in microns in the horizontal axis
    double originY;             ///< image origin in microns in the vertical axis
    long long int tm;           ///< timestamp of images
    double angle;               ///< acquisition angle for volumetric data
    int overlay;                ///< flag that the image is an overlay without grayscale (ie. color doppler or strain)
    CusImageFormat format;      ///< flag specifying the format of the image (see format definitions above)
    CusTgcInfo tgc[CUS_MAXTGC]; ///< tgc points

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
    int version;                ///< version (1 = clarius 1st generation, 2 = clarius HD, 3 = clarius HD3)
    int elements;               ///< # of probe elements
    int pitch;                  ///< element pitch
    int radius;                 ///< radius in mm

} CusProbeInfo;

/// positional data information structure
typedef struct _CusPosInfo
{
    long long int tm;           ///< timestamp in nanoseconds
    double gx;                  ///< gyroscope x; angular velocity is given in radians per second (rps)
    double gy;                  ///< gyroscope y
    double gz;                  ///< gyroscope z
    double ax;                  ///< accelerometer x; acceleration is normalized to gravity [~9.81m/s^2] (m/s^2)/(m/s^2)
    double ay;                  ///< accelerometer y
    double az;                  ///< accelerometer z
    double mx;                  ///< magnetometer x; magnetic flux density is normalized to the earth's field [~50 mT] (T/T)
    double my;                  ///< magnetometer y
    double mz;                  ///< magnetometer z
    double qw;                  ///< w component (real) of the orientation quaternion
    double qx;                  ///< x component (imaginary) of the orientation quaternion
    double qy;                  ///< y component (imaginary) of the orientation quaternion
    double qz;                  ///< z component (imaginary) of the orientation quaternion

} CusPosInfo;

/// point type
typedef struct _CusPoint
{
    double x;                   ///< x coordinate for a point
    double y;                   ///< y coordinate for a point

} CusPoint;

/// line type
typedef struct _CusLine
{
    CusPoint p1;                ///< first point in the line
    CusPoint p2;                ///< second point in the line

} CusLine;
