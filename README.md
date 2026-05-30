# SolarDrone

A standalone app and JUCE plugin (VST3, AU) that sonifies real-time space weather data from NOAA SWPC as a two-layer additive drone with animated visual. Solar wind parameters (velocity, density, Bz) drive Layer 1; the Kp planetary index drives Layer 2. The drone evolves continuously as space weather changes.

Educational tool and material for audiovisual installation.

## What it does

SolarDrone fetches live data from NOAA's Space Weather Prediction Center every 60 seconds — solar wind speed, density, interplanetary magnetic field (Bz), and the Kp geomagnetic disturbance index. These drive a two-layer additive oscillator bank with configurable interpolation (default: 2-minute glide). A three-layer visual renderer (Lissajous curves, particle field, spectral envelope) animates in real time alongside the audio.

Quiet solar conditions produce a sparse, open drone. Geomagnetic storms (Kp ≥ 6, southward Bz) produce dense, tense harmonic texture.

## Requirements

| Dependency | Version |
|------------|---------|
| C++ compiler | C++17 (GCC 11+, Clang 13+, MSVC 2022+) |
| CMake | 3.22+ |
| Linux | libcurl4-openssl-dev + X11/ALSA/WebKit packages (see below) |
| macOS | Xcode 14+ |
| Windows | Visual Studio 2022 or MSYS2/MinGW |

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**First build**: ~5–10 minutes — JUCE 8 is fetched via CMake FetchContent (~140 MB) and compiled.  
**Subsequent builds**: ~30 seconds (incremental).

### Linux system dependencies

```bash
sudo apt-get install -y \
  libasound2-dev libfreetype6-dev libx11-dev libxcomposite-dev \
  libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \
  libglu1-mesa-dev mesa-common-dev libwebkit2gtk-4.1-dev \
  libcurl4-openssl-dev
```

### Targets built

| Target | Platforms |
|--------|-----------|
| Standalone app | Windows, macOS, Linux |
| VST3 plugin | Windows, macOS, Linux |
| AU plugin | macOS only |
| Unit test binary | All |

## Quick Start — Standalone

```bash
# Linux
./build/SolarDrone_artefacts/Release/Standalone/SolarDrone

# macOS
open build/SolarDrone_artefacts/Release/Standalone/SolarDrone.app

# Windows (PowerShell)
.\build\Release\SolarDrone_artefacts\Standalone\SolarDrone.exe
```

> **Windows SmartScreen warning**: the binary is not Authenticode-signed. Windows will block it on first run. To allow:
> 1. Right-click the `.exe` → **Properties** → check **Unblock** → OK, or
> 2. Run in PowerShell: `Unblock-File -Path .\SolarDrone.exe`, or
> 3. In the SmartScreen dialog: click **More info** → **Run anyway**.

The app opens a 600×480 window: animated visual fills the top 400px, controls at the bottom. The status overlay (top-right) shows `default  Kp 0.0  age 0s` on launch.

After ~30 seconds, a live NOAA fetch completes and the overlay changes to `live  Kp X.X  age 0s` (green). The drone pitch and harmonic content shift to reflect actual solar wind conditions.

## Plugin Install

**VST3** — copy `build/.../SolarDrone.vst3` to your VST3 folder:
- Linux: `~/.vst3/`
- macOS: `~/Library/Audio/Plug-Ins/VST3/`
- Windows: `C:\Program Files\Common Files\VST3\`

**AU (macOS only)** — copy `build/.../SolarDrone.component` to `~/Library/Audio/Plug-Ins/Components/`

See [Known Limitations](#known-limitations) for notes on macOS code signing.

## Controls

All parameters are automatable via DAW (APVTS).

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Volume | 0–1 | 0.7 | Master output level |
| Drone On | on/off | on | Silences output immediately when off |
| Glide Time | 30–300 s | 120 s | Interpolation speed between data updates |
| Dynamics | 0–1 | 1.0 | Scales L2 amplitude excursion |
| Layer Balance | 0–1 | 0.5 | 0 = full L1 (solar wind), 1 = full L2 (Kp) |
| Stereo Spread | 0–1 | 0.5 | Stereo width between layers |
| Harmony Mode | Just / Equal | Just | Interval tuning system |
| L1/L2 Interval | 0–12 semitones | 7 (P5th) | Interval between L1 and L2 fundamentals |
| Visual Lissajous | 0–1 | 0.7 | Blend weight for Lissajous curve layer |
| Visual Particles | 0–1 | 0.7 | Blend weight for particle field layer |
| Visual Spectral | 0–1 | 0.7 | Blend weight for spectral bar layer |

## Architecture

Five components in the signal chain:

1. **DataFetcher** — background thread, polls `rtsw_wind_1m.json` and `planetary_k_index_1m.json` from NOAA SWPC every 60 seconds. Handles null fields (sensor calibration gaps) by retaining last valid values.
2. **SynthParamMapper** — pure function `(SpaceWeatherState, UserParams) → SynthParams`. Hybrid mapping: linear base + non-linear activations at Bz ≤ −10 nT and Kp ≥ 6.
3. **Interpolator** — per-field exponential slew on SynthParams at ~100 Hz control rate. Glide time configurable 30–300 s.
4. **AdditiveEngine** — two independent oscillator banks (L1: solar wind, L2: Kp). Constant-power layer balance crossfade.
5. **VisualRenderer** — JUCE Component, 30 fps, three composited layers driven by smoothed SynthParams.

See `ROADMAP.md` for full phase breakdown and `src/SynthParamMapper.cpp` for mapping formulas.

## Running Tests

```bash
cmake --build build --target SolarDrone_Tests
# Linux / macOS
./build/SolarDrone_Tests_artefacts/Release/SolarDroneTests
# Windows
.\build\Release\SolarDrone_Tests_artefacts\SolarDroneTests.exe
```

15 unit tests: 8 DataFetcher parser tests (quiet day, storm day, null fields), 7 SynthParamMapper tests (mapping values, determinism, harmony modes).

## Known Limitations

**HTTP sandbox in Logic Pro / Pro Tools**: these hosts sandbox plugin processes and block outbound HTTP. SolarDrone detects empty responses and falls back to the last known space weather state (`source = "cached"`, displayed in yellow). The drone continues playing with the last values. This is a documented v1 design decision — no companion app workaround is provided.

**First fetch latency**: NOAA data arrives ~30 seconds after cold start. The drone plays with default values (velocity = 450 km/s, Kp = 0) until the first successful fetch.

**ALSA warnings on Linux/WSL2**: `open /dev/snd/seq failed` messages are non-fatal. JUCE handles missing ALSA devices gracefully; audio output uses the available device.

**Windows SmartScreen**: binaries are not Authenticode-signed. Right-click `.exe` → Properties → **Unblock**, or run `Unblock-File -Path .\SolarDrone.exe` in PowerShell. Alternatively, click **More info → Run anyway** in the SmartScreen dialog.

**AU code signing (macOS)**: unsigned AU builds require Gatekeeper to be disabled or the binary to be signed with an Apple Developer ID. For development use: `sudo spctl --master-disable` or use the VST3 format instead.

**Drone on/off**: toggling off silences immediately (no fade). A glide-out fade is registered for v2.

**Real-time data only**: historical playback is not implemented in v1. Data is always the most recent NOAA reading.

## Data Source

[NOAA Space Weather Prediction Center](https://www.swpc.noaa.gov/) — public JSON API, no authentication required.

- Solar wind: `https://services.swpc.noaa.gov/json/rtsw/rtsw_wind_1m.json`
- Kp index: `https://services.swpc.noaa.gov/json/planetary_k_index_1m.json`

Data updated every ~1 minute. Fields used: `speed` (km/s), `density` (p/cm³), `bz_gsm` (nT), `estimated_kp` (0–9).

## License

SolarDrone source code: **MIT**.  
JUCE framework: **GPL v3** (Community Edition). Distributing binaries requires either open-source release under GPL or a [JUCE commercial license](https://juce.com/get-juce/).  
VST3 SDK: Steinberg VST3 License.
