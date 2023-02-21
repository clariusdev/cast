#pragma once

#include "api_common.h"

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
    FullScreen,         ///< toggle full screen
    VoiceCommand        ///< toggle voice commands

} CusUserFunction;

/// measurement types
typedef enum _CusMeasurementType
{
    CusMeasurementTypeDistance = 1, ///< point-to-point distance
    CusMeasurementTypeTraceDistance, ///< trace distance
    CusMeasurementTypeTraceArea ///< trace area
} CusMeasurementType;
