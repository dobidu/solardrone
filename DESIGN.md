# SolarDrone — Architecture

Signal chain: **DataFetcher → SynthParamMapper → Interpolator → AdditiveEngine → audio output** with **VisualRenderer** consuming the same smoothed parameters on the message thread.

## Design Principle: SynthParamMapper as Canonical Product

`SynthParamMapper::map(SpaceWeatherState, UserParams) → SynthParams` is the stable contract between data and output. It is a pure function with no JUCE dependency, no side effects, and no state.

Consumers (AdditiveEngine, VisualRenderer) are independent renderings of SynthParams. New data sources plug into `SpaceWeatherState`. New synthesis engines plug in downstream, consuming SynthParams directly. This separation is intentional and must be preserved in v2+ extensions.

---

## Components

### 1. DataFetcher

**What it does.** Background `juce::Thread` that polls two NOAA SWPC JSON endpoints every 60 seconds. Parses `speed`, `density`, `bz_gsm` (solar wind) and `estimated_kp` (planetary disturbance index). Maintains last-valid cache: when NOAA fields are null (sensor calibration gaps), retains previous values and increments `data_age_s`.

**Key files:** `src/DataFetcher.h`, `src/DataFetcher.cpp`, `src/SpaceWeatherState.h`

**Key decisions:**
- Thread launched from constructor (not `prepareToPlay`) — audio device may never initialise (WSL2, headless).
- HTTP failure returns empty string silently — `isEmpty()` check → `source = "cached"`. No exceptions.
- `JUCE_USE_CURL=1` on Linux only (requires `find_package(CURL) + CURL::libcurl` link). macOS/Windows use native HTTP stacks (`JUCE_USE_CURL=0`).
- `parseSolarWindJson()` and `parseKpJson()` are public to enable unit testing without network.

---

### 2. SynthParamMapper

**What it does.** Pure static function. Maps `SpaceWeatherState` + `UserParams` → `SynthParams`. Hybrid mapping: linear base for normal conditions, non-linear activation functions for extreme events.

| Input | Output | Formula |
|-------|--------|---------|
| velocity 300–800 km/s | l1_fundamental_hz 55–220 Hz | `55 × 4^((v-300)/500)` log scale |
| density 1–50 p/cm³ | l1_harmonic_count 2–24 | linear |
| Bz ≥ 0 nT | l1_timbre = 0 (open) | — |
| Bz in [−10, 0] nT | l1_timbre 0→0.3 | linear |
| Bz ≤ −10 nT | l1_timbre 0.3→1.0 | ramp over 7 nT |
| Kp 0–9 | l2_amplitude 0–1 | `kp/9 × dynamics_range` |
| Kp < 6 | l2_harmonic_density = 0 | — |
| Kp ≥ 6 | l2_harmonic_density 0→1 | quadratic `x²`, x=1 at Kp=8 |
| harmony_mode + interval | l2_fundamental_hz | just ratio table or `2^(n/12)` |

**Key files:** `src/SynthParamMapper.h`, `src/SynthParamMapper.cpp`, `src/SynthParams.h`, `src/UserParams.h`

**Key decisions:**
- No JUCE includes — uses only `<cmath>`. Compiles in test binary and plugin equally.
- `l2_amplitude` is raw, pre-mix. AdditiveEngine applies `layer_balance` crossfade downstream.
- v2-safe: adding `x_ray_flux` to `SpaceWeatherState` requires no changes to existing SynthParams fields.

---

### 3. Interpolator

**What it does.** Applies per-field exponential slew to `SynthParams` at control rate (~100 Hz). Ensures the drone evolves continuously between DataFetcher updates rather than stepping.

**Key file:** `src/Interpolator.h`, `src/Interpolator.cpp`

**Key decisions:**
- `alpha = 1 - exp(-4.6 / (glide_secs × controlRate))` — 99% convergence in `glide_time_secs` (ln(100) ≈ 4.6).
- `l1_harmonic_count` snaps immediately (integer — no slew).
- `reset(SynthParams)` for instant state (used on cold start / plugin reload).

---

### 4. AdditiveEngine

**What it does.** Owns two `OscillatorBank`s (L1: solar wind, L2: Kp) and an `Interpolator`. Renders stereo audio in `processBlock()` using a control-rate architecture: the buffer is split into ~10ms chunks; `updateControlRate()` is called at the start of each chunk to apply new SynthParams to the oscillator banks.

**Key files:** `src/AdditiveEngine.h`, `src/AdditiveEngine.cpp`, `src/OscillatorBank.h`, `src/OscillatorBank.cpp`

**Key decisions:**
- Control-rate sub-blocking: `while (remaining > 0) { if (counter <= 0) updateControlRate(); render(chunk); }`. Decouples parameter update rate from audio block size.
- Constant-power layer balance: `angle = layer_balance × π/2; L1 = cos(angle); L2 = sin(angle)`.
- `std::sin()` inner loop — no wavetable. ~0.33 ms/block at 32 partials/layer (44.1 kHz/512 samples), well within 5% CPU budget.
- `getSmoothedParams()` protected by `smoothedLock` (CriticalSection) for thread-safe read by VisualRenderer.

---

### 5. VisualRenderer

**What it does.** A `juce::Component` + `juce::Timer` (30 fps) that composites three animated layers driven by smoothed SynthParams. Polls `PluginProcessor::getCurrentSynthParams()` on the message thread timer tick. Three layers drawn in order: spectral (background) → particles (mid) → Lissajous (top).

| Layer | Driven by | Visual effect |
|-------|-----------|---------------|
| Lissajous | L1/L2 frequency ratio, l1_timbre (phase drift rate), l1_amplitude (line thickness) | Parametric curve morphs from stable figure-8 to chaotic form as Bz drops |
| Particles | l1_amplitude (speed), l1_timbre (turbulence), l2_harmonic_density (count), l2_brightness (hue) | Blue-violet drift → orange storm at Kp ≥ 6 |
| Spectral | l1_harmonic_count (L1 bars), l1_timbre (warp), l2_harmonic_density+brightness (L2 overlay) | Harmonic landscape grows and shifts with space weather |

**Key files:** `src/VisualRenderer.h`, `src/VisualRenderer.cpp`

**Key decisions:**
- Timer poll, not audio thread push — safe, no cross-thread data race, sufficient at 30 fps.
- `CriticalSection paramsLock` for `displayParams` handoff between timer and `paint()`.
- Software renderer only (no OpenGL dependency) — works in all plugin hosts.
- `SpaceWeatherState` cached in VisualRenderer for status overlay (live/cached/default).

---

## Thread Model

```
Background thread (DataFetcher)
    NOAA HTTP → SpaceWeatherState (every 60s)
    ↓ (mutex protected getState())

Audio thread (processBlock, ~87×/s at 44.1kHz/512)
    read SpaceWeatherState → SynthParamMapper → Interpolator → OscillatorBanks
    ↓ (smoothedLock protected getSmoothedParams())

Message thread (juce::Timer, 30fps)
    poll getCurrentSynthParams() → VisualRenderer → repaint()
    poll getLatestSpaceWeatherState() → status overlay
```

---

## Extending SolarDrone

**New data source (e.g., X-ray flux for v2.1)**
1. Add `float x_ray_flux = 0.0f` to `SpaceWeatherState`.
2. Parse new NOAA endpoint in `DataFetcher`.
3. Map in `SynthParamMapper::map()` — e.g., drive `l1_brightness` (new SynthParams field).
4. AdditiveEngine and VisualRenderer consume SynthParams unchanged.

**New synthesis engine (e.g., granular for v2)**
1. Implement `GranularEngine` with the same interface: `prepare()`, `setSynthParams()`, `setUserParams()`, `processBlock()`.
2. Replace `AdditiveEngine engine` in `PluginProcessor`.
3. Zero changes to DataFetcher, SynthParamMapper, or VisualRenderer.

---

## APVTS Parameter IDs

These IDs are stable API. Renaming requires bumping the `apvts` state tag in `PluginProcessor.cpp`.

| ID | Type | Range | Default |
|----|------|-------|---------|
| `volume` | float | 0–1 | 0.7 |
| `drone_on` | bool | — | true |
| `glide_time` | float | 30–300 s | 120 |
| `dynamics_range` | float | 0–1 | 1.0 |
| `layer_balance` | float | 0–1 | 0.5 |
| `stereo_spread` | float | 0–1 | 0.5 |
| `harmony_mode` | choice | Just / Equal Temp. | Just |
| `interval` | int | 0–12 semitones | 7 |
| `vis_lissajous` | float | 0–1 | 0.7 |
| `vis_particles` | float | 0–1 | 0.7 |
| `vis_spectral` | float | 0–1 | 0.7 |
