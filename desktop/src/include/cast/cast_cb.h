#pragma once

#include "cast_def.h"

/// return status callback function
/// @param[in] port the udp port on a successful connection attempt, CUS_FAILURE on an unsuccessful attempt or other error
/// @param[in] swRevMatch flag if the sw revisions between the api and the clarius app match, CUS_SUCCESS if a match, CUS_FAILURE on a mismatch
typedef void (*CusConnectFn)(int port, int swRevMatch);
/// return status callback function
/// @param[in] retCode the return code (CUS_SUCCESS or CUS_FAILURE)
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
/// @param[in] btn the button that was pressed
/// @param[in] clicks # of clicks performed
typedef void (*CusButtonFn)(CusButton btn, int clicks);
/// progress callback function
/// @param[in] progress the current progress
typedef void (*CusProgressFn)(int progress);
/// raw data callback function
/// @param[in] res the raw data result, typically the size of the data package requested or actually downloaded
typedef void (*CusRawFn)(int res);
/// error callback function
/// @param[in] msg the error message with associated error that occurred
typedef void (*CusErrorFn)(const char* msg);
