# Cast API Reference

Complete reference for the **v12** Cast C API. Declarations live in
[`include/cast`](../include/cast):

- [`cast.h`](../include/cast/cast.h) — functions and the init struct
- [`cast_def.h`](../include/cast/cast_def.h) — enumerations and data structs
- [`cast_cb.h`](../include/cast/cast_cb.h) — callback signatures

All functions are `extern "C"`, exported with `CAST_EXPORT`, and return `0`
(`CUS_SUCCESS`) / `-1` (`CUS_FAILURE`) unless noted. The Clarius App must be running and
connected to a probe. For the call flow see [getting-started.md](getting-started.md); for
control commands and research parameters see [parameters.md](parameters.md).

## Conventions

| Constant | Value | Meaning |
|----------|-------|---------|
| `CUS_SUCCESS` | `0` | success |
| `CUS_FAILURE` | `-1` | failure |
| `CUS_MAXTGC` | `10` | size of TGC arrays in image info structs |

## Lifecycle & connection

| Function | Description |
|----------|-------------|
| `int castInit(const CusInitParams* params)` | Initialize the module. Must be called first. |
| `CusInitParams castDefaultInitParams(void)` | Zero-initialized init params. |
| `int castDestroy(void)` | Free resources; call before exit. |
| `int castConnect(const char* ipAddress, unsigned int port, const char* cert, CusConnectFn fn)` | Connect to a probe. Pass `"research"` as `cert` to bypass authentication. |
| `int castDisconnect(CusReturnFn fn)` | Disconnect. |
| `int castIsConnected(void)` | `1` connected, `0` not, `-1` not initialized. |
| `int castFwVersion(CusPlatform platform, char* version, int sz)` | Firmware version for a platform (buffer ≥ 64 bytes). |
| `int castProbeInfo(CusProbeInfo* info)` | Probe hardware info. |

### `CusInitParams`

| Field | Type | Notes |
|-------|------|-------|
| `args.argc`, `args.argv` | `int`, `char**` | Forwarded to the library (Qt graphics buffer init). |
| `storeDir` | `const char*` | Writable directory for security keys. |
| `newProcessedImageFn` | `CusNewProcessedImageFn` | Scan-converted frames. |
| `newRawImageFn` | `CusNewRawImageFn` | Pre-scan-converted / RF frames. |
| `newSpectralImageFn` | `CusNewSpectralImageFn` | M / PW spectral blocks. |
| `newImuDataFn` | `CusNewImuDataFn` | IMU samples. |
| `freezeFn` | `CusFreezeFn` | Freeze/run state. |
| `buttonFn` | `CusButtonFn` | Physical button presses. |
| `progressFn` | `CusProgressFn` | Readback (download) progress. |
| `errorFn` | `CusErrorFn` | Error messages. |
| `width`, `height` | `int` | Output buffer dimensions in pixels. |

## Image output

| Function | Description |
|----------|-------------|
| `int castSetOutputSize(int w, int h)` | Set scan-conversion output dimensions (1:1 pixel aspect; black borders fill the rest). |
| `int castSeparateOverlays(int en)` | Deliver grayscale and color/strain overlay as separate callbacks. |
| `int castSetFormat(CusImageFormat format)` | Output format for processed images (default ARGB). |

## Imaging control

| Function | Description |
|----------|-------------|
| `int castUserFunction(CusUserFunction cmd, double val, CusReturnFn fn)` | Run a control command. `SetDepth` uses `val` (cm), `SetGain` uses `val` (%); most others ignore `val`. |

See the full [`CusUserFunction` list](parameters.md#user-functions).

## Research (low-level) control

> ⚠️ These reach below the standard controls and can drive the transducer outside the
> validated safety envelope. See [parameters.md](parameters.md#research-parameters).

| Function | Description |
|----------|-------------|
| `int castSetParameter(const char* prm, double val, CusReturnFn fn)` | Set a low-level numeric parameter. |
| `int castEnableParameter(const char* prm, int en, CusReturnFn fn)` | Enable/disable a low-level boolean parameter. |
| `int castSetPulse(const char* prm, const char* shape, CusReturnFn fn)` | Set a transmit pulse shape string. |

## Raw data

The probe must be **frozen** with raw-data buffering enabled in the App. See
[getting started, step 6](getting-started.md#6-capture-raw-data).

| Function | Description |
|----------|-------------|
| `int castRawDataAvailability(CusRawAvailabilityFn fn)` | Report buffered B and IQ/RF frame timestamps. |
| `int castRequestRawData(long long start, long long end, int lzo, CusRawRequestFn fn)` | Request frames by ns timestamp (`0,0` = all). `lzo=1` → compressed tarball. Callback returns byte size to allocate. |
| `int castReadRawData(void** data, CusRawFn fn)` | Download requested data into a pre-allocated buffer. |

## Capture authoring (v12)

Compose a capture and submit it to the current exam. See
[getting started, step 7](getting-started.md#7-author-captures-v12).

| Function | Description |
|----------|-------------|
| `int castStartCapture(long long timestamp)` | Begin a capture for the frame at `timestamp`; returns a capture `id` (or `-1`). |
| `int castAddImageOverlay(int id, const void* data, int width, int height, float r, float g, float b, float a)` | Add an 8-bit grayscale overlay colorized by RGBA (each 0.0–1.0). `width`/`height` must match `castInit`. |
| `int castAddLabelOverlay(int id, const char* text, double x, double y, double width, double height)` | Add a text label (pixel coordinates). |
| `int castAddMeasurement(int id, CusMeasurementType type, const char* label, const double* pts, int count)` | Add a distance/trace/area measurement. `count` is the number of *doubles* in `pts`, not points. |
| `int castFinishCapture(int id, CusReturnFn fn)` | Finalize and submit the capture. |

## Callbacks

Signatures from [`cast_cb.h`](../include/cast/cast_cb.h):

| Callback | Signature | Fires when |
|----------|-----------|------------|
| `CusConnectFn` | `(int imagePort, int imuPort, int swRevMatch)` | Connection result; ports are UDP stream ports, `swRevMatch` flags API/App version match. |
| `CusReturnFn` | `(int retCode)` | Generic success/failure for an async call. |
| `CusNewRawImageFn` | `(const void* img, const CusRawImageInfo* nfo, int npos, const CusPosInfo* pos)` | Pre-scan / RF frame ready. |
| `CusNewProcessedImageFn` | `(const void* img, const CusProcessedImageInfo* nfo, int npos, const CusPosInfo* pos)` | Scan-converted frame ready. |
| `CusNewSpectralImageFn` | `(const void* img, const CusSpectralImageInfo* nfo)` | M/PW spectral block ready. |
| `CusFreezeFn` | `(int state)` | `1` frozen, `0` imaging. |
| `CusButtonFn` | `(CusButton btn, int clicks)` | Physical button pressed. |
| `CusProgressFn` | `(int progress)` | Download progress (percent). |
| `CusRawAvailabilityFn` | `(int res, int n_b, const long long* b, int n_iqrf, const long long* iqrf)` | Buffered raw-data timestamps. |
| `CusRawRequestFn` | `(int res, const char* extension)` | Raw request sized; `extension` is the package type. |
| `CusRawFn` | `(int res)` | Raw download complete (`res` = bytes). |
| `CusErrorFn` | `(const char* msg)` | An error occurred. |
| `CusNewImuDataFn` | `(const CusPosInfo* pos)` | Streamed IMU sample. |

> Callbacks run on the SDK's threads — keep them short and hand data to your own queues.

## Enumerations

From [`cast_def.h`](../include/cast/cast_def.h).

**`CusPlatform`** — `V1` (first generation), `HD`, `HD3`

**`CusImageFormat`** — `Uncompressed` (32-bit ARGB), `Uncompressed8Bit` (8-bit gray),
`Jpeg`, `Png`

**`CusButton`** — `ButtonUp`, `ButtonDown`, `ButtonHandle`

**`CusMeasurementType`** — `CusMeasurementTypeDistance` (1), `CusMeasurementTypeTraceDistance`,
`CusMeasurementTypeTraceArea`

**`CusUserFunction`** — the control commands for `castUserFunction`; see the
[full table](parameters.md#user-functions).

## Data structures

From [`cast_def.h`](../include/cast/cast_def.h). These mirror the Solum structs.

### `CusProcessedImageInfo` (scan-converted frames)

`width`, `height`, `bitsPerPixel`, `imageSize` (bytes), `micronsPerPixel` (1:1),
`originX`/`originY` (microns), `tm` (ns), `angle`, `fps`, `overlay` (1 = color/strain
overlay), `format` (`CusImageFormat`), `tgc[CUS_MAXTGC]`.

### `CusRawImageInfo` (pre-scan / RF frames)

`lines`, `samples`, `bitsPerSample`, `axialSize`/`lateralSize` (microns), `tm` (ns),
`jpeg` (byte size, `0` if uncompressed), `rf` (1 = RF, 0 = envelope), `angle`, `fps`,
`tgc[CUS_MAXTGC]`.

### `CusSpectralImageInfo` (M / PW)

`lines`, `samples`, `bitsPerSample`, `period` (s), `micronsPerSample` (M),
`velocityPerSample` (m/s, PW), `pw` (1 = PW, 0 = M).

### `CusPosInfo` (IMU / positional)

`tm` (ns), gyro `gx/gy/gz` (rad/s), accel `ax/ay/az` (normalized to g), magnetometer
`mx/my/mz` (normalized to Earth's field), orientation quaternion `qw/qx/qy/qz`.

### `CusProbeInfo`

`version` (1 = gen 1, 2 = HD, 3 = HD3), `elements`, `pitch`, `radius` (mm),
`frequency` (Hz).

### `CusTgcInfo`, `CusPointF`, `CusLineF`

`CusTgcInfo`: `depth` (mm), `gain` (dB). `CusPointF`: `x`, `y`. `CusLineF`: `p1`, `p2`.

## Deprecated aliases

`CusLine` → `CusLineF`, `CusPoint` → `CusPointF`.

## v13 preview (not yet released)

The next API adds a `newImageReadyFn` callback (pull model), `CusRawCompression`
replacing the integer `lzo` flag on `castRequestRawData`, and a `castSetEnum()` function;
the capture-authoring functions are being reworked. Build against v12 until v13 ships.
