# SolarDrone — PROJECT

## What

Standalone app and JUCE plugin (VST3, AU) that sonifies real-time space weather data from NOAA SWPC as a two-layer additive drone with abstract artistic visual representation. Solar wind parameters (velocity, density, Bz) drive Layer 1; Kp planetary index drives Layer 2. The drone evolves continuously as space weather changes, with configurable interpolation. Targets Windows, macOS, and Linux.

## Why

Space weather monitoring tools speak to specialists — they are technical, visual, numerical. Nothing in the open plugin/app space makes space weather *experienceable* as sustained sound. Sonification research in this domain exists but rarely ships as usable instruments. SolarDrone fills that gap: real NOAA data → aesthetic audio output, live, in any DAW or as a standalone installation piece. The educational and artistic goals reinforce each other — the sound *means* something because it comes from real data.

## Who

- Artists and musicians seeking live data-driven generative sound as compositional material or installation output.
- Educators in space physics who want an accessible, auditory demonstration of geomagnetic phenomena.
- Audiovisual installation designers wanting a self-running, perpetually-changing ambient layer.
- The author (N=1, primary user and first validator).

## Core architectural idea: SynthParamMapper as canonical product

SolarDrone's internal product is the **SynthParamMapper** — a pure function `(SpaceWeatherState, UserParams) → SynthParams`. It encapsulates the mapping philosophy (hybrid linear base + non-linear activations for extreme events). The AdditiveEngine (audio) and VisualRenderer (visual) are independent consumers of `SynthParams` — they do not touch raw space weather data.

This means: v2 can replace the synthesis engine (granular, FM) without touching the mapper. A new data source plugs into `SpaceWeatherState` without touching the engine or renderer. The mapper is the stable contract between data and output.

## Non-goals (v1)

- No historical playback — real-time data only.
- No raw data graphs or numerical displays in the UI — visualization is abstract and artistic, not a data dashboard.
- No multi-channel or spatial audio — stereo output only.
- No user-editable mapping curves — UserParams control dynamics/panorama/harmony within a fixed mapping logic; the mapping itself is not exposed.
- No audio transport between instances.
- No MIDI or OSC output (v2).
- No mobile or web build (v2).
- No companion app for plugin sandboxed hosts — plugin warns and uses last-known state.

## Stack

- **Core**: C++ / JUCE 7+ (audio DSP, UI, HTTP, JSON, plugin wrappers)
- **Build**: CMake (CI-friendly; Projucer not used)
- **Plugin formats**: VST3, AU, Standalone
- **Data**: NOAA SWPC public JSON API (no authentication required)
  - Solar wind: `rtsw_wind_1m.json` (velocity, density, Bz, ~1min updates)
  - Kp: `planetary_k_index_1m.json` (estimated Kp, ~1min updates)
- **OS**: Windows 10+, macOS 11+, Linux desktop
- **License**: JUCE Community Edition (GPL) for open-source distribution; commercial license if monetized

## Architecture

Six components in v1:

1. **DataFetcher** — Background thread. HTTP GET polling NOAA SWPC at ~60s interval using JUCE::URL. Parses solar wind JSON (velocity, density, bz_gsm) and Kp JSON. Produces and atomically updates `SpaceWeatherState`. Handles null fields (sensor calibration gaps, telemetry outages) by retaining last valid values. Tracks `data_age_s`. In plugin hosts with HTTP sandboxing (Logic Pro, Pro Tools), gracefully falls back to cached state and sets `source = "cached"`.

2. **SynthParamMapper** — Pure function, stateless. Takes `(SpaceWeatherState, UserParams)` and returns `SynthParams`. Hybrid mapping: linear base for normal conditions, non-linear activation functions for extreme events (Bz ≤ −10 nT triggers timbre shift; Kp ≥ 6 triggers harmonic density swell and amplitude growth; Kp 9 = maximum activation). `UserParams` scales the output within designed aesthetic ranges (dynamics range, harmony mode, panorama spread) without altering the mapping logic.

3. **Interpolator** — Applies configurable glide (slew) to `SynthParams` transitions at control rate (~100Hz). Glide time user-configurable (30s–5min, default 2min). Ensures the drone evolves continuously between data updates rather than stepping.

4. **AdditiveEngine** — Two independent oscillator banks (Layer 1: solar wind, Layer 2: Kp). Each bank sums N sinusoidal partials at configurable harmonic series ratios. Stereo output with per-layer panning. Target: ≤5% CPU at 32 partials/layer on mid-spec (2020-era 8-core) machine at 44.1kHz/512 samples.

5. **VisualRenderer** — JUCE Component updated at ~30fps. Driven exclusively by smoothed `SynthParams` — not raw space weather data. Visual metaphor to be selected at start of Phase 05 (candidates: Lissajous curves, particle field, spectral envelope shape, geometric morphing). Must work in plugin window (fixed-size, software rendering; OpenGL optional with software fallback).

6. **JUCE AudioProcessor** — Hosts all components. Exposes `AudioProcessorParameter`s for DAW automation. Standalone mode: full DataFetcher, all UI, no restrictions. Plugin mode: DataFetcher on background thread, same UI in plugin window, documented sandbox behavior.

## Data and state model

```
SpaceWeatherState {
  timestamp:    ISO string (from NOAA feed)
  velocity:     float km/s     // nullable → last valid on null
  density:      float p/cm³    // nullable → last valid on null
  bz_gsm:       float nT       // nullable → last valid on null
  kp:           float 0.0–9.0  // nullable → last valid on null
  data_age_s:   int            // seconds since last successful fetch
  source:       "live" | "cached" | "default"
}

SynthParams {
  // Layer 1 — solar wind
  l1_fundamental_hz:    float   // base pitch from velocity
  l1_harmonic_count:    int     // active partials from density
  l1_timbre:            float   // 0=open (Bz+), 1=tense (Bz−)
  l1_amplitude:         float
  // Layer 2 — Kp
  l2_fundamental_hz:    float   // interval from L1 set by harmony UserParam
  l2_harmonic_density:  float   // 0–1, increases with Kp
  l2_brightness:        float   // 0–1, increases with Kp ≥ 6
  l2_amplitude:         float
  // Global
  stereo_width:         float   // 0–1
}
```

## Exposed user parameters (UI + DAW automation)

| Parameter | Range | Default | Notes |
|-----------|-------|---------|-------|
| Master volume | 0–1 | 0.7 | |
| Drone on/off | bool | on | |
| Glide time | 30s–300s | 120s | Interpolator slew |
| Dynamics range | 0–1 | 0.6 | Scales amplitude excursion |
| Layer balance | 0–1 | 0.5 | L1 vs L2 relative level |
| Stereo spread | 0–1 | 0.5 | Per-layer panning width |
| Harmony mode | enum | Just | Just intonation / Equal temp / Custom ratio |
| L1↔L2 interval | semitones | 7 (perfect 5th) | Interval between fundamentals |

## Mapping philosophy (hybrid C)

- **Linear base**: velocity (300–800 km/s) → l1_fundamental_hz (e.g. 55–220 Hz log-scale); density → l1_harmonic_count (2–24 partials); Kp (0–5) → l2_amplitude (0.1–0.5).
- **Non-linear activations**:
  - Bz ≤ −10 nT: sigmoid ramp on l1_timbre (0 → 1); additional detuning on l1 partials.
  - Kp ≥ 6: exponential ramp on l2_harmonic_density and l2_brightness.
  - Kp = 9: clamp to maximum density; amplitude ceiling applied to prevent clipping.
- Aesthetic priority: quiet solar conditions produce a clean, sparse drone; extreme events produce rich, tense, dense texture.

## Constraints

- JUCE Community Edition (GPL): plugin binaries must be open-source or require commercial license.
- NOAA SWPC API: public, no rate limit documented, fields subject to change without notice.
- HTTP in plugin context: may be blocked by Logic Pro / Pro Tools sandboxing. Plugin documents this; no companion app in scope.
- macOS AU: code signing required for AU validation in Logic Pro.
- VisualRenderer: must not block audio thread; updates on UI/message thread only.

## Future integrations (registered, not v1)

- **v2.1** — Additional NOAA data sources: X-ray flux (solar flare class B/C/M/X), proton flux. Plug into `SpaceWeatherState` without changing mapper logic significantly.
- **v2.2** — Historical playback: load NOAA archive data, replay as drone. `DataFetcher` becomes swappable with `ArchivePlayer`.
- **v2.3** — User-editable mapping curves: expose `SynthParamMapper` logic as editable curves in UI.
- **Future ecosystem**: OSC output (SynthParams → OSC for live coding / VJ tools); binaural / Ambisonics rendering; web companion showing visual + data.

## Honest limits (README)

- Data latency: NOAA solar wind data has ~1min propagation + processing lag. This is real-time data, not real-time physics.
- Official Kp updates every 3h; 1-min estimated Kp is noisier.
- First data fetch takes up to 30s on cold start.
- HTTP blocked in some plugin hosts — documented in README, no workaround in v1.
- Aesthetic mappings are authored, not empirically optimal — user expectation should be art tool, not scientific instrument.

## First users / validation

Author's own sessions. Primary validation: record 10-minute drone during a quiet day (Kp ≤ 2) and during a geomagnetically active day (Kp ≥ 5), compare — must be audibly and visually distinct. Secondary: run as unattended installation for 24h, verify no crash, visual/audio evolving meaningfully.

## Glossary

- **Bz**: z-component of the interplanetary magnetic field (IMF). Negative Bz (southward) = geoeffectively coupled to Earth's magnetosphere → triggers geomagnetic storms.
- **Kp index**: Planetary K-index. 0–9 scale measuring global geomagnetic disturbance. Officially updated every 3h; 1-min estimated Kp available from NOAA.
- **DSCOVR**: Deep Space Climate Observatory — NASA satellite at L1 Lagrange point providing upstream solar wind data used by NOAA SWPC.
- **NOAA SWPC**: Space Weather Prediction Center. Provides public real-time and archive space weather data.
- **SpaceWeatherState**: Snapshot of current space environment parameters, fetched and maintained by DataFetcher.
- **SynthParams**: Normalized synthesis parameters produced by SynthParamMapper from SpaceWeatherState.
- **UserParams**: User-controlled parameters that scale or tune the mapping output (dynamics, panorama, harmony).
- **Additive synthesis**: Sound synthesis by summing sinusoidal partials at specific frequency ratios.
- **Drone**: Sustained, slowly-evolving tone — the primary sonic output of SolarDrone.
- **Glide / Slew**: Smooth interpolation between parameter values over time; gives the drone its continuous character rather than stepping between data updates.
- **Layer 1 / Layer 2**: Two independent oscillator banks. L1 driven by solar wind parameters; L2 driven by Kp.
- **Non-linear activation**: A mapping function that stays near-flat for normal conditions and ramps sharply above a threshold (e.g., Kp ≥ 6 triggers exponential harmonic density growth).
