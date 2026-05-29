---
phase: 03-synthmapper
plan: 01
subsystem: core
tags: [synthmapper, mapping, additive-synthesis, pure-function, unit-tests]

requires:
  - phase: 02-datafetcher
    provides: SpaceWeatherState struct

provides:
  - SynthParams struct (L1 solar wind params, L2 Kp params, global)
  - UserParams struct (dynamics_range, layer_balance, stereo_spread, harmony_mode, interval)
  - SynthParamMapper::map() — pure static function, no JUCE dependency, no side effects
  - 7 unit tests covering quiet/storm/Kp=9/determinism/harmony/velocity/density

affects: [04-additiveengine, 05-visualrenderer, 06-integration]

tech-stack:
  added: []
  patterns:
    - SynthParamMapper uses only <cmath> — no JUCE dependency, zero coupling to audio system
    - template clamp<T> avoids juce::jlimit dependency while staying portable
    - Non-linear activations: Bz < -10nT = timbre ramp (0.3→1.0 over 7nT); Kp >= 6 = quadratic density, linear brightness
    - l2_amplitude is "raw" pre-mix (division by 9 from Kp); AdditiveEngine applies layer_balance crossfade in Phase 4

key-files:
  created:
    - src/SynthParams.h
    - src/UserParams.h
    - src/SynthParamMapper.h
    - src/SynthParamMapper.cpp
    - tests/SynthParamMapperTests.cpp

key-decisions:
  - "SynthParamMapper.cpp uses only <cmath> — no JUCE dependency. Compiles in any context."
  - "l2_amplitude = (kp/9) * dynamics_range — linear, scaled; layer_balance crossfade deferred to Phase 4"
  - "Bz activation threshold: -10nT to -17nT ramp (reaches 1.0 at -17nT)"
  - "Kp density activation: quadratic (x^2) from Kp=6-8; 1.0 at Kp=8+ (clamped)"
  - "Just intonation lookup table: 13 intervals (unison to octave), perfect 5th default"
  - "UserParams.dynamics_range default: 1.0 (full range), not 0.6 — ensures AC thresholds reachable"

patterns-established:
  - "Consumers of SynthParams (AdditiveEngine, VisualRenderer) receive raw per-layer amplitudes; mixing is NOT mapper's responsibility"
  - "SynthParamMapper has no state — instantiate, call map(), done. Thread-safe by design."

duration: ~30min
started: 2026-05-29T00:00:00Z
completed: 2026-05-29T00:00:00Z
---

# Phase 3 Plan 01: SynthParamMapper Summary

**Pure function `SynthParamMapper::map(SpaceWeatherState, UserParams) → SynthParams` with hybrid linear + non-linear mapping; 15/15 unit tests pass (8 DataFetcher + 7 SynthParamMapper).**

## Performance

| Metric | Value |
|--------|-------|
| Tasks | 2 completed |
| Files created | 5 |
| Unit tests total | 15/15 pass, exit 0 |
| SynthParamMapper tests | 7 pass |
| Build time | ~1min (incremental) |

## Acceptance Criteria Results

| Criterion | Status | Notes |
|-----------|--------|-------|
| AC-1: Quiet day low activation | **Pass** | timbre=0.0, density=0.0, amp=0.111, brightness=0.0 |
| AC-2: Storm high activation | **Pass** | timbre=0.8, density=1.0, amp=0.889, brightness=1.0 |
| AC-3: Kp=9 max activation | **Pass** | density=brightness=amplitude=1.0 ✓ |
| AC-4: Deterministic | **Pass** | Identical inputs → identical float outputs |
| AC-5: Harmony mode differs | **Pass** | Just: 3/2 ratio; equal: 2^(7/12) ratio |
| AC-3.6 (listen test) | Manual — pending AdditiveEngine (Phase 4) |
| AC-3.7 (v2 review) | See below |

## Mapping Summary

| Data | Range | Output | Formula |
|------|-------|--------|---------|
| velocity | 300-800 km/s | l1_fundamental_hz 55-220 Hz | `55 * 4^((v-300)/500)` log scale |
| density | 1-50 p/cm³ | l1_harmonic_count 2-24 | linear |
| Bz | 0 to -17+ nT | l1_timbre 0→1 | linear 0→0.3 at -10nT, ramp 0.3→1.0 over 7nT |
| Kp | 0-9 | l2_amplitude 0-1 | linear * dynamics_range |
| Kp | 6-8+ | l2_harmonic_density 0-1 | quadratic (x²) from Kp=6 |
| Kp | 6-8+ | l2_brightness 0-1 | linear from Kp=6 |

## v2 Readiness Review

**Question:** Can v2.1 data (X-ray flux: B/C/M/X class, ~0.001–1.0 logarithmic scale) plug into SynthParams without breaking existing fields?

**Answer: Yes — no breaking change required.**

- `SpaceWeatherState` can gain a new `float x_ray_flux = 0.0f` field without touching existing fields.
- `SynthParamMapper::map()` can use that field to drive an additional SynthParams dimension. The obvious mapping is: X-class flare (flux > 0.1) → `l1_timbre` swell or a new `l1_brightness` field.
- Current SynthParams fields (l1_*, l2_*, stereo_width) remain stable — v2.1 adds fields, not changes.
- The only v2 risk: if X-ray flux should REPLACE Bz as the timbre driver rather than ADD to it. Recommendation: add as a separate field and blend — keeps the mapper additive.

**Verdict:** v1 SynthParams shape is v2-safe as-is. Register X-ray flux as an additive field in v2.1.

## Next Phase Readiness

**Ready:**
- `SynthParams` is the input type for `AdditiveEngine` (Phase 4) and `VisualRenderer` (Phase 5)
- `UserParams` carries layer_balance — Phase 4 will implement the constant-power crossfade
- Mapper is stateless and thread-safe — can be called from any thread in Phase 6 wiring

**Concerns:** None. Mapper is minimal and correct.

**Blockers:** None.
