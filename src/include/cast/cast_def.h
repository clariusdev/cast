#pragma once

#include <api_common.h>

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

