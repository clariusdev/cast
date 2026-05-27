# Cast Parameter & Command Reference

Two ways to control the probe through Cast: high-level **user functions** (the same
actions a button or the App UI triggers) and low-level **research parameters** (direct
acoustic/sequencing control). For function signatures see the
[API reference](api-reference.md); for the call flow see [getting-started.md](getting-started.md).

## User functions

Invoke with `castUserFunction(cmd, val, fn)`. Only `SetDepth` and `SetGain` use `val`;
all other commands ignore it.

### Imaging & display

| Command | Action |
|---------|--------|
| `Freeze` | Toggle freeze. |
| `CaptureImage` | Capture a still image. |
| `CaptureCine` | Capture a cine. |
| `DepthDec` / `DepthInc` | Decrease / increase depth. |
| `SetDepth` | Set depth (uses `val`, cm). |
| `GainDec` / `GainInc` | Decrease / increase gain. |
| `SetGain` | Set gain (uses `val`, %). |
| `AutoGain` | Toggle auto gain. |
| `ContrastDec` / `ContrastInc` | Decrease / increase contrast. |
| `Zoom` | Toggle zoom. |
| `Flip` | Toggle horizontal flip. |
| `CenterGuide` | Toggle center guide. |
| `FullScreen` | Toggle full screen. |
| `VoiceCommand` | Toggle voice controls. |

### Modes

| Command | Action |
|---------|--------|
| `BMode` | B-mode. |
| `TMode` | T-mode. |
| `MMode` | M-mode. |
| `ColorDoppler` | Color Doppler. |
| `PowerDoppler` | Power Doppler. |
| `PwDoppler` | Pulsed-wave Doppler (when available). |
| `NeedleEnhance` | Needle enhance (when available). |
| `NeedleSide` | Toggle needle-enhance side. |
| `Strain` | Strain elastography (when available). |
| `Ceus` | Contrast-enhanced mode (when available). |
| `RfMode` | RF mode (when available). |

### Color Doppler controls

| Command | Action |
|---------|--------|
| `ColorGainDec` / `ColorGainInc` | Decrease / increase color gain. |
| `ColorPrfDec` / `ColorPrfInc` | Decrease / increase color PRF. |

### Cine & layout

| Command | Action |
|---------|--------|
| `PlayCine` | Play cine (when frozen). |
| `PreviousFrame` / `NextFrame` | Step cine frames. |
| `SwitchArray` | Switch array (multi-array probes). |
| `Split1` / `Split2` / `Split4` | Split screen into 1 / 2 / 4 displays. |
| `SplitNext` | Activate the next display. |
| `Annotate` | Open annotations. |
| `ClearScreen` | Clear the screen. |

## Research parameters

> ⚠️ **Safety.** Research parameters bypass the optimized presets and can change the
> acoustic output of the transducer, pushing imaging **outside the safety limits Clarius
> validates**, degrading image quality, or destabilizing the system. They require an
> appropriate license and are intended for research. Parameters marked **\*\*** directly
> affect acoustic output.

Access them through three calls:

```c
castSetParameter("txFreq", 5.0, returnFn);     // numeric value
castEnableParameter("sa", 1, returnFn);        // boolean enable/disable
castSetPulse("txPulseGen", "+-", returnFn);    // transmit pulse shape string
```

The Solum SDK exposes the **same** parameter set via
`solumSetLowLevelParam`/`solumEnableLowLevelParam`/`solumSetLowLevelPulse`. The
authoritative, mode-by-mode list with value semantics lives in the
[research repository](https://github.com/clariusdev/research).

### Numeric parameters

**Generic**

| Parameter | Meaning |
|-----------|---------|
| `maxFrameRate` | Frame-rate limit in Hz (0–100, `0` = no limit). |

**Greyscale**

| Parameter | Meaning |
|-----------|---------|
| `txFreq` ** | Transmit frequency (MHz). |
| `txFreqInv` ** | Transmit frequency for the inversion pulse (MHz). |
| `txFn` ** | Transmit f-number. |
| `txApt` ** | Max transmit aperture in elements (2–64). |
| `txFocus` ** | Focus depth (cm). |
| `vpp` ** | Positive amplitude (V, 10–37). |
| `vnn` ** | Negative amplitude (V, 10–37). |
| `ceVpp` ** | Positive amplitude for CEUS (V, 10–37). |
| `ceVnn` ** | Negative amplitude for CEUS (V, 10–37). |
| `steer` ** | Image steering (degrees). |
| `rxFn` | Receive f-number. |
| `rxFreqShallow` | Start demodulation frequency (MHz). |
| `rxFreqDeep` | End demodulation frequency (MHz). |
| `speedOfSound` | Speed-of-sound correction (m/s). |
| `rfDecimation` | Decimation factor for IQ data. |
| `envDecimation` | Decimation factor for envelope/grayscale data. |
| `rfDecim` | Decimation factor of RF acquisition. |
| `dyn` | Dynamic range (dB). |
| `nf` | Noise floor (dB). |

**Color / Power Doppler**

| Parameter | Meaning |
|-----------|---------|
| `cfiPrf` ** | PRF (kHz; range varies per probe/preset). |
| `cfiTxFreq` ** | Transmit frequency (MHz). |
| `color steer` | Steering angle (degrees). |
| `cfiEnsemble` ** | Ensemble / transmits per line (6–16). |

**Pulsed-Wave Doppler**

| Parameter | Meaning |
|-----------|---------|
| `pwPrf` ** | PRF (kHz; range varies per probe/preset). |
| `pwTxFreq` ** | Transmit frequency (MHz). |
| `pw gate size` | Gate size (mm). |
| `pw steer` | Steering angle (degrees). |
| `pw angle correct` | Correction angle (degrees). |

### Boolean parameters (greyscale)

| Parameter | Meaning |
|-----------|---------|
| `sa` | Synthetic aperture. |
| `pih` | Pulse-inversion harmonics. |
| `trapezoidal` | Extended FOV for linear probes. |

### Pulse-shape (string) parameters

| Parameter | Meaning |
|-----------|---------|
| `txPulseGen` ** | Transmit pulse. |
| `txPulsePen` ** | Transmit pulse in penetration mode. |
| `txPulseInv` ** | Transmit pulse for the inversion pulse. |
| `cfiPulse` ** | Color/power transmit pulse. |
| `pwPulse` ** | PW transmit pulse. |

** Changing this parameter may alter the acoustic output of the transducer, making
imaging operate outside the safety parameters Clarius has programmed into the device.
