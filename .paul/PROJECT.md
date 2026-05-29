# SolarDrone

## What This Is

Standalone app and JUCE plugin (VST3, AU) that sonifies real-time space weather data from NOAA SWPC as a two-layer additive drone with abstract artistic visual representation. Solar wind parameters (velocity, density, Bz) drive Layer 1; Kp planetary index drives Layer 2. The drone evolves continuously as space weather changes via configurable interpolation. Targets Windows 10+, macOS 11+, Linux desktop.

## Core Value

Makes invisible space weather phenomena audible and aesthetically meaningful — as educational tool and material for artistic installation.

## Current State

| Attribute | Value |
|-----------|-------|
| Type | Application |
| Version | 0.0.0 |
| Status | Initializing |
| Last Updated | 2026-05-29 |

## Requirements

### Core Features

- **DataFetcher**: real-time polling of NOAA SWPC solar wind + Kp data, with null-handling and last-known-state cache
- **SynthParamMapper**: pure function `(SpaceWeatherState, UserParams) → SynthParams` — the canonical product; hybrid linear + non-linear mapping
- **AdditiveEngine**: two-layer oscillator bank (L1=solar wind, L2=Kp) with configurable glide interpolation
- **VisualRenderer**: abstract/artistic JUCE component driven by SynthParams at ~30fps
- **Full UI**: status indicator, drone on/off, volume, glide time, dynamics, panorama, harmony controls — all exposed as DAW-automatable parameters

### Validated (Shipped)
None yet.

### Active (In Progress)
None yet.

### Planned (Next)

- [ ] Phase 1: Foundation — JUCE CMake project building on all 3 OSes

### Out of Scope

- Historical playback (v2.2)
- User-editable mapping curves (v2.3)
- Multi-channel / spatial audio (stereo only)
- MIDI / OSC output (v2 ecosystem)
- Mobile or web build (v2 ecosystem)
- Raw data graphs in UI (visualization is abstract, not a dashboard)
- Companion app for sandboxed plugin hosts

## Target Users

**Primary:** Artists and musicians seeking live data-driven generative sound
- Want aesthetically meaningful output, not scientific accuracy
- May use as standalone installation or within a DAW

**Secondary:**
- Space physics educators wanting an accessible auditory demonstration
- Audiovisual installation designers wanting a self-running ambient layer

## Context

**Technical Context:**
Data source is NOAA SWPC public JSON API (no auth). HTTP requests run on a background thread; may be blocked by sandboxed plugin hosts (Logic Pro, Pro Tools) — documented limitation, plugin uses last-known-state fallback.

## Constraints

### Technical Constraints

- JUCE Community Edition (GPL): plugin binaries must be open-source or require commercial license
- NOAA SWPC API: public, no documented rate limit, fields subject to change without notice
- HTTP blocked in Logic Pro / Pro Tools sandboxed plugin context — no workaround in v1
- macOS AU: code signing required for AU validation in Logic Pro
- VisualRenderer must not block audio thread (message thread only)
- AdditiveEngine target: ≤5% CPU at 32 partials/layer, 44.1kHz/512 samples, mid-spec machine

### Business Constraints

- Solo project, author as primary user and validator
- No external deadline

## Key Decisions

| Decision | Rationale | Date | Status |
|----------|-----------|------|--------|
| SynthParamMapper as canonical product | Keeps AdditiveEngine and VisualRenderer as swappable renderizações; enables v2 extensibility without refactoring | 2026-05-29 | Active |
| Hybrid mapping (linear base + non-linear activations) | Aesthetic quality over data fidelity; quiet conditions = sparse drone, extreme events = rich/tense texture | 2026-05-29 | Active |
| Configurable glide time (30s–5min) | User controls how fast drone evolves; suits both studio and installation contexts | 2026-05-29 | Active |
| No companion app for sandboxed hosts | Scope boundary; documented limitation acceptable for v1 | 2026-05-29 | Active |
| CMake over Projucer | CI-friendly; standard for modern JUCE projects | 2026-05-29 | Active |

## Success Metrics

| Metric | Target | Current | Status |
|--------|--------|---------|--------|
| Audible contrast quiet vs storm | Clear, subjectively meaningful difference in 10-min recording comparison | - | Not started |
| 24h unattended stability | No crash, visual/audio evolving, no memory leak | - | Not started |
| CPU overhead | ≤5% at 32 partials/layer on mid-spec laptop | - | Not started |
| Plugin hosts validated | VST3 (Reaper Win+Linux, Ableton macOS), AU (Logic Pro), Standalone (all 3 OSes) | - | Not started |

## Tech Stack / Tools

| Layer | Technology | Notes |
|-------|------------|-------|
| Core | C++ / JUCE 7+ | Audio DSP, UI, HTTP, JSON, plugin wrappers |
| Build | CMake | No Projucer; CI-friendly |
| Plugin formats | VST3, AU, Standalone | AAX out of scope (requires AVID agreement) |
| Data | NOAA SWPC public JSON API | `rtsw_wind_1m.json`, `planetary_k_index_1m.json` |
| OS | Windows 10+, macOS 11+, Linux desktop | |
| CI | GitHub Actions | Build matrix all 3 OSes |

## Links

| Resource | URL |
|----------|-----|
| Full context | PROJECT.md (project root) |
| Roadmap detail | ROADMAP.md (project root) |
| Acceptance criteria | ACCEPTANCE.md (project root) |

---
*PROJECT.md — Updated when requirements or context change*
*Last updated: 2026-05-29*
