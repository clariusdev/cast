#pragma once

// SDK: cast
// Version: 12.0.2

#define CUS_MAXTGC  10
#define CUS_SUCCESS 0
#define CUS_FAILURE (-1)

/// The probe buttons
typedef enum _CusButton
{
    ButtonUp,           ///< Up button
    ButtonDown,         ///< Down button
    ButtonHandle,       ///< Handle button (custom probes only)

} CusButton;

/// The compression types available
typedef enum _CusImageFormat
{
    Uncompressed,       ///< Processed images are sent in a raw and uncompressed in 32 bits ARGB
    Uncompressed8Bit,   ///< Processed images are sent in a raw and uncompressed in 8 bit grayscale
    Jpeg,               ///< Processed images are sent as a jpeg (with header)
    Png,                ///< Processed images are sent as a png (with header)

} CusImageFormat;

/// Major Clarius platforms
typedef enum _CusPlatform
{
    V1,                 ///< First generation
    HD,                 ///< Second generation (HD)
    HD3,                ///< Third generation (HD3)

} CusPlatform;

/// The possible user functions from a button, foot pedal, or listener
typedef enum _CusUserFunction
{
    Freeze = 1,         ///< toggle freeze
    CaptureImage,       ///< capture still image
    CaptureCine,        ///< capture cine
    DepthDec,           ///< decrease depth
    DepthInc,           ///< increase depth
    GainDec,            ///< decrease gain
    GainInc,            ///< increase gain
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
    FullScreen,         ///< toggle full screen
    VoiceCommand,       ///< toggle voice controls
    DynamicRngDec,      ///< decrease dynamic range
    DynamicRngInc,      ///< increase dynamic range
    ColorGainDec,       ///< decrease color gain
    ColorGainInc,       ///< increase color gain
    ColorPrfDec,        ///< decrease color prf
    ColorPrfInc,        ///< increase color prf
    SwitchArray,        ///< switch array

} CusUserFunction;

/// Supported measurement types
typedef enum _CusMeasurementType
{
    CusMeasurementTypeDistance = 1, ///< point-to-point distance
    CusMeasurementTypeTraceDistance, ///< trace distance
    CusMeasurementTypeTraceArea, ///< trace area

} CusMeasurementType;

/// TGC information
typedef struct _CusTgcInfo
{
    double depth;       ///< Depth in millimeters
    double gain;        ///< Gain in decibels

} CusTgcInfo;

/// Positional data information structure
typedef struct _CusPosInfo
{
    long long int tm;   ///< Timestamp in nanoseconds
    double gx;          ///< Gyroscope x; angular velocity is given in radians per second (RPS)
    double gy;          ///< Gyroscope y; angular velocity is given in radians per second (RPS)
    double gz;          ///< Gyroscope z; angular velocity is given in radians per second (RPS)
    double ax;          ///< Accelerometer x; acceleration is normalized to gravity [~9.81m/s^2] (m/s^2)/(m/s^2)
    double ay;          ///< Accelerometer y; acceleration is normalized to gravity [~9.81m/s^2] (m/s^2)/(m/s^2)
    double az;          ///< Accelerometer z; acceleration is normalized to gravity [~9.81m/s^2] (m/s^2)/(m/s^2)
    double mx;          ///< Magnetometer x; magnetic flux density is normalized to the earth's field [~50 mT] (T/T)
    double my;          ///< Magnetometer y; magnetic flux density is normalized to the earth's field [~50 mT] (T/T)
    double mz;          ///< Magnetometer z; magnetic flux density is normalized to the earth's field [~50 mT] (T/T)
    double qw;          ///< W component (real) of the orientation quaternion
    double qx;          ///< X component (imaginary) of the orientation quaternion
    double qy;          ///< Y component (imaginary) of the orientation quaternion
    double qz;          ///< Z component (imaginary) of the orientation quaternion

} CusPosInfo;

/// Probe information
typedef struct _CusProbeInfo
{
    int version;        ///< Version (1 = clarius 1st generation, 2 = clarius HD, 3 = clarius HD3)
    int elements;       ///< Number of probe elements
    int pitch;          ///< Element pitch
    int radius;         ///< Radius in millimeters

} CusProbeInfo;

/// Processed image information supplied with each frame
typedef struct _CusProcessedImageInfo
{
    int width;          ///< width of the image in pixels
    int height;         ///< height of the image in pixels
    int bitsPerPixel;   ///< bits per pixel
    int imageSize;      ///< total size of image in bytes
    double micronsPerPixel; ///< microns per pixel (always 1:1 aspect ratio axially/laterally)
    double originX;     ///< image origin in microns in the horizontal axis
    double originY;     ///< image origin in microns in the vertical axis
    long long int tm;   ///< timestamp of images
    double angle;       ///< acquisition angle for volumetric data
    double fps;         ///< frame rate in hz
    int overlay;        ///< flag that the image is an overlay without grayscale (ie. color doppler or strain)
    CusImageFormat format; ///< flag specifying the format of the image (see format definitions above)
    CusTgcInfo tgc [CUS_MAXTGC]; ///< tgc points

} CusProcessedImageInfo;

/// Raw image information supplied with each frame
typedef struct _CusRawImageInfo
{
    int lines;          ///< number of ultrasound lines in the image
    int samples;        ///< number of samples per line in the image
    int bitsPerSample;  ///< bits per sample
    double axialSize;   ///< axial microns per sample
    double lateralSize; ///< lateral microns per line
    long long int tm;   ///< timestamp of image
    int jpeg;           ///< size of the jpeg image, 0 if not a jpeg compressed image
    int rf;             ///< flag specifying data is rf and not envelope
    double angle;       ///< acquisition angle for volumetric data
    double fps;         ///< frame rate in hz
    CusTgcInfo tgc [CUS_MAXTGC]; ///< tgc points

} CusRawImageInfo;

/// Spectral image information supplied with each block
typedef struct _CusSpectralImageInfo
{
    int lines;          ///< Number of lines in the block
    int samples;        ///< Number of samples per line
    int bitsPerSample;  ///< Bits per sample
    double period;      ///< Line acquisition period in seconds
    double micronsPerSample; ///< Microns per pixel/sample in an M spectrum
    double velocityPerSample; ///< Velocity in m/s per pixel/sample in a PW spectrum
    int pw;             ///< Flag specifying the data is PW and not M

} CusSpectralImageInfo;

/// 2D point with double precision
typedef struct _CusPointF
{
    double x;           ///< X coordinate
    double y;           ///< Y coordinate

} CusPointF;

/// 2D line with double precision
typedef struct _CusLineF
{
    CusPointF p1;       ///< First point in the line
    CusPointF p2;       ///< Second point in the line

} CusLineF;

#ifndef CAST_DEPRECATED
#  ifdef _MSC_VER
#    define CAST_DEPRECATED __declspec(deprecated)
#  else
#    define CAST_DEPRECATED __attribute__ ((__deprecated__))
#  endif
#endif

CAST_DEPRECATED typedef struct _CusLineF CusLine;
CAST_DEPRECATED typedef struct _CusPointF CusPoint;
