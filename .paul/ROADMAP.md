# Roadmap: SolarDrone

## Overview

Eight phases from empty JUCE project to v1 release: foundation build, live data fetching (with HTTP spike), synthesis mapping, additive engine, visual renderer, full integration, cross-platform validation, and docs/release. v2 registers additional data sources, historical playback, and user-editable mapping curves.

## Current Milestone

**v2.8.0 — Resonator Engine** ✅ Complete (2026-06-01)

| Release | Status | Date |
|---------|--------|------|
| v1.0.0 Initial Release | ✅ | 2026-05-29 |
| v2.1.0 BeatRepeater + Chopper | ✅ | 2026-05-30 |
| v2.2.0 UI/UX Overhaul | ✅ | 2026-05-30 |
| v2.3.0 Mapping UI | ✅ | 2026-05-31 |
| v2.4.0 X-ray Flux L3 Layer | ✅ | 2026-05-31 |
| v2.5.0 Binaural ILD + EQ | ✅ | 2026-05-31 |
| v2.6.0 OSC + MIDI CC | ✅ | 2026-05-31 |
| v2.7.0 Visual Overhaul 2 | ✅ | 2026-05-31 |
| v2.8.0 Resonator Engine | ✅ | 2026-06-01 |

## Phases

| Phase | Name | Plans | Status | Completed |
|-------|------|-------|--------|-----------|
| 1 | Foundation | 1 | ✅ Complete | 2026-05-29 |
| 2 | DataFetcher | 2 | ✅ Complete | 2026-05-29 |
| 3 | SynthParamMapper | 1 | ✅ Complete | 2026-05-29 |
| 4 | AdditiveEngine + Interpolator | 1 | ✅ Complete | 2026-05-29 |
| 5 | VisualRenderer | 1 | ✅ Complete | 2026-05-29 |
| 6 | Integration + Full UI | 2 | ✅ Complete | 2026-05-29 |
| 7 | Cross-platform Validation | 1 | ✅ Complete | 2026-05-29 |
| 8 | Docs + Release | 1 | ✅ Complete | 2026-05-29 |

## Phase Details

### Phase 1: Foundation

**Goal:** JUCE project builds on Windows, macOS, and Linux with empty AudioProcessor and basic UI shell — repo has PAUL structure, graphify baseline committed.
**Depends on:** Nothing (first phase)
**Research:** Unlikely (standard JUCE CMake setup)

**Scope:**
- JUCE project with CMake, VST3 + AU + Standalone targets
- Stub AudioProcessor (no audio, no parameters)
- Stub AudioProcessorEditor (empty window, no crash)
- Clean build on all 3 OSes
- GitHub Actions CI matrix skeleton

**Plans:**
- [x] 01-01: CMake project, stub AudioProcessor, verify builds on all 3 OSes, CI skeleton, graphify

---

### Phase 2: DataFetcher

**Goal:** Live NOAA SWPC data flowing into `SpaceWeatherState` with null handling and last-known-state cache.
**Depends on:** Phase 1 (builds on all 3 OSes)
**Research:** Likely — HTTP in plugin context may be sandboxed by host
**Research topics:** Does `JUCE::URL` work on a background thread inside a VST3 in: Standalone, Reaper (Win+Linux), Logic Pro (macOS), Ableton Live (macOS)? Failure modes and fallback strategy.

**Spike protocol:** Run `/paul:plan 2 --quick-fix` first → produces `phases/02-datafetcher/02-99-PLAN.md`. Execute spike, write `02-99-SUMMARY.md`. Run `/paul:unify 02-99` → writes decision to STATE.md. Then run main `/paul:plan 2`.

**Scope:**
- `DataFetcher` class: JUCE Thread, polls NOAA at 60s interval
- Parse `rtsw_wind_1m.json` (velocity, density, bz_gsm) and `planetary_k_index_1m.json` (Kp)
- `SpaceWeatherState` struct with atomic update; null-field handling; last-valid cache; `data_age_s`; `source` field
- Default state for cold start / API unreachable (velocity=450, density=5, Bz=0, Kp=0)
- Unit tests with fixture JSONs (quiet day, storm day, all-null fields)

**Plans:**
- [x] 02-99: Spike — HTTP in plugin context (research)
- [x] 02-01: DataFetcher implementation + unit tests

---

### Phase 3: SynthParamMapper

**Goal:** Pure function `(SpaceWeatherState, UserParams) → SynthParams` with hybrid mapping, unit-tested against quiet and storm fixtures.
**Depends on:** Phase 2 (SpaceWeatherState defined)
**Research:** Unlikely (internal design, no external dependencies)

**Scope:**
- `SynthParams` and `UserParams` structs
- Linear base mappings (velocity → pitch, density → partial count, Kp → amplitude)
- Non-linear activations (Bz ≤ −10 nT → timbre shift; Kp ≥ 6 → harmonic density/brightness swell; Kp 9 = max)
- Harmony mode (just, equal, custom ratio) applied to partial frequencies
- v2 readiness review at phase end (can v2.1 data plug in without breaking SynthParams?)
- Unit tests; listen test with prototype oscillator

**Plans:**
- [x] 03-01: SynthParamMapper implementation + unit tests + v2 review

---

### Phase 4: AdditiveEngine + Interpolator

**Goal:** Two-layer additive oscillator bank producing continuous drone audio with configurable glide interpolation.
**Depends on:** Phase 3 (SynthParams struct finalized)
**Research:** Unlikely (JUCE DSP, established patterns)

**Scope:**
- `OscillatorBank`: N sinusoidal partials, harmonic series per harmony mode, amplitude taper
- Layer 1 (solar wind) and Layer 2 (Kp) as separate banks
- Per-layer stereo panning scaled by stereo_spread UserParam
- `Interpolator`: per-field slew at control rate (~100Hz), configurable glide time
- `processBlock()` integration: control-rate param pull, audio-rate render
- CPU profiling: ≤5% at 32 partials/layer (44.1kHz/512); add lookup-table oscillators if needed

**Plans:**
- [x] 04-01: OscillatorBank + Interpolator + processBlock + CPU profiling

---

### Phase 5: VisualRenderer

**Goal:** Abstract/artistic JUCE Component driven by SynthParams at ≥25fps without blocking audio thread.
**Depends on:** Phase 4 (SynthParams smoothing via Interpolator available)
**Research:** Unlikely (visual metaphor is a design choice, not a technical unknown)

**Scope:**
- Choose visual metaphor at phase start (candidates: Lissajous curves, particle field, spectral envelope shape, geometric morphing) — decision logged in SUMMARY before implementation
- JUCE Component with `paint()` override; software renderer baseline; optional OpenGL path
- Map SynthParams → visual parameters (documented per-mapping)
- Lock-free value or mutex for SynthParams handoff from audio thread
- `Timer` at 30fps on message thread
- Test in standalone window and plugin window

**Plans:**
- [x] 05-01: Visual metaphor decision (Lissajous+particles+spectral) + VisualRenderer + checkpoint approved

---

### Phase 6: Integration + Full UI

**Goal:** All components wired end-to-end; full UI with all controls; all parameters exposed as DAW-automatable `AudioProcessorParameter`s.
**Depends on:** Phase 4 (engine) + Phase 5 (renderer) both complete
**Research:** Unlikely

**Scope:**
- Wire: DataFetcher → SynthParamMapper → Interpolator → AdditiveEngine → audio
- Wire: Interpolator output → VisualRenderer via lock-free value
- UI controls: status indicator (source, data_age_s, Kp as text), drone on/off, master volume, glide time, dynamics range, layer balance, stereo spread, harmony mode, L1↔L2 interval
- `AudioProcessorParameter` for each control with correct range + name (verify in Reaper automation lane)
- End-to-end smoke test: cold start → fetch → sound + visual + all controls functional

**Plans:**
- [x] 06-01: DataFetcher→engine pipeline wired, status overlay
- [x] 06-02: APVTS 11 params, controls panel, state save/restore, checkpoint approved

---

### Phase 7: Cross-platform Validation + CI

**Goal:** VST3, AU, and Standalone verified on all three OSes; CI builds passing; macOS code-signed for AU.
**Depends on:** Phase 6 (full build)
**Research:** Unlikely (process, not discovery)

**Scope:**
- VST3 in Reaper (Windows 10+, Linux)
- VST3 in Ableton Live (macOS)
- AU in Logic Pro (macOS) — verify HTTP sandbox behavior + "offline" status message
- Standalone on all 3 OSes
- macOS code signing (developer ID for AU notarization)
- CI matrix: CMake build + headless smoke test on Windows/macOS/Linux
- Document host-specific HTTP behavior in README

**Plans:**
- [ ] 07-01: Cross-platform test matrix + CI matrix + code signing

---

### Phase 8: Docs + Release

**Goal:** Cut v1 — README, demo recording, GitHub release with binaries.
**Depends on:** Phase 7 (all formats validated)
**Research:** Unlikely

**Scope:**
- README: install (build + prebuilt), quick start, known limits, plugin sandbox note
- Demo: 10-min recording quiet day (Kp ≤ 2) vs active day (Kp ≥ 5) — audible contrast documented
- DESIGN.md: architecture summary for contributors
- GitHub release: tagged binaries (VST3 Win+Linux, VST3+AU macOS, Standalone all 3 OSes)

**Plans:**
- [ ] 08-01: README + demo recording + GitHub release

---

## Phases — v2 (all complete)

| Phase | Focus | Status |
|-------|-------|--------|
| v2.1 | BeatRepeater + Chopper, MIDI clock sync | ✅ 2026-05-30 |
| v2.2 | UI/UX Overhaul (SunDisc, MacroOrb, spatial display) | ✅ 2026-05-30 |
| v2.3 | Mapping UI with editable thresholds | ✅ 2026-05-31 |
| v2.4 | X-ray flux → L3 burst layer | ✅ 2026-05-31 |
| v2.5 | Binaural ILD + 3-band EQ + crackling fix | ✅ 2026-05-31 |
| v2.6 | OSC + MIDI CC output | ✅ 2026-05-31 |
| v2.7 | Visual Overhaul 2 (3D Lissajous, bloom, 200+ particles) | ✅ 2026-05-31 |
| v2.8 | Resonator Engine (modal/FDN/strings) + dynamic layout | ✅ 2026-06-01 |

**Future ecosystem (not roadmapped):** historical playback, web companion, mobile port.

## Milestone v2.5 — Visual Overhaul 2: Dynamic & Immersive

**Goal:** Transform the visual renderer from functional to stunning. More particles, 3D projection, color gradients, trail effects, volumetric glow.

**Scope:**
- **Lissajous 3D**: Add Z oscillator (sin(ω₃t+φ)), project to 2D with perspective depth. The figure gains dimensionality and rotates slowly with l1_timbre.
- **Particle system overhaul**: 200+ particles with Z depth, painter's sort, size-by-depth, color gradient by age (birth=accent, death=transparent). Turbulence driven by Bz.
- **Trail effect on Lissajous**: Draw N previous curve positions with decreasing alpha — creates a phosphor/CRT afterglow effect.
- **Volumetric glow on SunDisc**: Radial gradient bloom around disc, intensity = l1_amplitude.
- **Spectral 3D bars**: Draw spectral bars as parallelograms with slight perspective offset — appears as 3D extrusion.
- **Storm flash**: When Kp crosses 7, brief full-screen flash followed by color-shifted visual mode.
- **Gradient strokes**: Lissajous color transitions along curve length (calm blue → storm red over one period).

**Spike required:** evaluate JUCE software renderer performance with 200 particles + 3D projections at 30fps. OpenGL path optional if software renderer < 25fps.

## Milestone v2.1 — Rhythmic Modulation

**v2.1 target:** Beat Repeater + Chopper layered over the drone, with shared BPM sync (MIDI clock from host OR internal BPM parameter, auto-detect).

## Milestone v2.2 — UI/UX Overhaul

**v2.2 target:** Complete redesign of the interface. Goal: innovative, comprehensive, intuitive. Move away from flat horizontal sliders toward a visual-first, spatially meaningful layout. Space weather data front and center. Controls feel like an instrument, not a settings panel.

### Phase v2.2 — UI/UX Overhaul

**Design directions to explore (decide at phase start):**
- **Radial layout**: circular instrument centered on the visual. Drone controls orbit the visual.
- **Space-themed skeuomorphism**: gauges/meters that look like mission control panels. Solar wind as a moving waveform. Kp as a magnetometer needle.
- **Data-first**: real-time numeric displays prominent (velocity, Bz, Kp, BPM). Controls secondary.
- **Touch-friendly**: large targets, minimal clutter — suitable for installation context.

**Scope (minimum for v2.2):**
- New layout architecture (Component hierarchy redesign)
- Visual feedback on every parameter (value labels, color coding by range)
- Status panel: velocity + Bz + Kp as styled gauges, not just text
- Contextual labels explaining when params are active ("Glide: affects transitions", "Dynamics: scales during Kp events")
- Repeater + Chopper visualized as waveform/pattern strip
- Accessibility: keyboard navigation, sufficient contrast

**Spike likely:** visual design decision before implementation.

---

## Phases — v2.1

### Phase v2.1.1 — Beat Repeater

**Goal:** Buffer the drone signal and repeat rhythmically, creating stutter/loop effects synchronized to a BPM parameter.

**Scope:**
- Ring buffer of configurable size (up to 2 bars at given BPM)
- Read head loops through buffer at rhythmic subdivision (1/16 to 2 bars)
- Parameters: BPM (30–300), loop length, feedback (0–1), wet/dry
- Inserts between AdditiveEngine output and volume stage in processBlock()

**Spike likely:** verify real-time buffer read/write without audio dropout at various buffer sizes.

### Phase v2.1.2 — Chopper (Rhythmic Gate)

**Goal:** Apply rhythmic amplitude gating (tremolo/gate) to the drone at musical rates.

**Scope:**
- LFO applied to amplitude post-engine
- Shapes: sine, square, sawtooth
- Parameters: rate (free Hz 0.1–20 OR BPM-sync), depth (0–1), shape
- BPM sync shared with Beat Repeater if both active
- Inserts in same stage as Beat Repeater (chainable)

---

## Milestone v2.8 — Resonator Engine ✅ (complete 2026-06-01)

Three parallel resonator modules driven by live solar data:

- **Modal Bank** (32 IIR bandpass): velocity → decay, Kp → mode count/inharmonicity
- **FDN** (8-tap Hadamard): Dst → T60, dynamic pressure → absorption cutoff
- **Sympathetic Strings** (12 comb): proton flux → damping + noise injection

UI: 3-column layout (1350×760) — Visual | Params | Resonators simultaneously.
HIDE VISUAL toggle collapses to 690×820 with stacked Repeater/Chopper strip.

---
*Roadmap created: 2026-05-29*
*Last updated: 2026-06-01 — v2.8.0 Resonator Engine complete*
