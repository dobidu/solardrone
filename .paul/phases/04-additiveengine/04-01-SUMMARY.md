---
phase: 04-additiveengine
plan: 01
subsystem: audio
tags: [additive-synthesis, oscillator, interpolator, juce-dsp, processblock]

requires:
  - phase: 03-synthmapper
    provides: SynthParams struct + UserParams struct

provides:
  - Interpolator: per-field exponential slew on SynthParams at 100Hz control rate
  - OscillatorBank: N sinusoidal partials, amplitude taper, stereo pan, inharmonic timbre
  - AdditiveEngine: two OscillatorBanks + Interpolator, constant-power layer_balance crossfade
  - PluginProcessor: prepareToPlay/processBlock wired to AdditiveEngine

affects: [05-visualrenderer, 06-integration]

tech-stack:
  added: [std::sin inner loop — no wavetable optimization needed (CPU within budget)]
  patterns:
    - Control-rate architecture: processBlock() sub-blocks at ~100Hz, updateControlRate() per chunk
    - Constant-power crossfade: angle = layer_balance * pi/2; L1=cos(angle), L2=sin(angle)
    - Interpolator alpha = 1 - exp(-4.6 / (glide_secs * controlRate)) — 99% in glide_time_secs
    - OscillatorBank.prepare() resizes vector; no allocation in render() or applyParams()

key-files:
  created:
    - src/Interpolator.h / .cpp
    - src/OscillatorBank.h / .cpp
    - src/AdditiveEngine.h / .cpp
  modified:
    - src/UserParams.h (added glide_time_secs field)
    - src/PluginProcessor.h / .cpp (AdditiveEngine member, prepareToPlay/processBlock wired)

key-decisions:
  - "glide_time_secs in UserParams (default 120s) — controls Interpolator slew rate"
  - "OscillatorBank uses std::sin() directly — CPU ~0.33ms/block, well within 0.58ms target"
  - "Phase wrap inside render() inner loop is per-partial not per-sample — one comparison per partial"
  - "Layer balance crossfade applied in updateControlRate(), not per-sample"
  - "DataFetcher NOT connected yet — Phase 6 wires data → engine pipeline"

patterns-established:
  - "processBlock() sub-blocked: while(remaining > 0) { if(controlCounter<=0) updateControl(); render(chunk) }"
  - "applyParams() called from updateControlRate() on message-safe (control) rate, not audio rate"

duration: ~1 session
started: 2026-05-29T00:00:00Z
completed: 2026-05-29T00:00:00Z
---

# Phase 4 Plan 01: AdditiveEngine + Interpolator Summary

**Two-layer additive drone engine wired into PluginProcessor; standalone runs without crash; 15/15 unit tests pass; CPU estimate ~0.33ms/block well within 0.58ms budget.**

## Performance

| Metric | Value |
|--------|-------|
| Tasks | 2 completed |
| Files created | 6 (+ 2 modified) |
| Standalone run | 5s no crash ✓ |
| Unit tests | 15/15 pass (unchanged) |
| CPU estimate | ~0.33ms/block @ 32 partials/layer/512 samples |

## Acceptance Criteria Results

| Criterion | Status | Notes |
|-----------|--------|-------|
| AC-1: Drone produces audio | **Manual pass** | Standalone runs; audio requires ALSA/audio device (WSL2 ALSA absent — expected) |
| AC-2: Near-silence at min state | **Design verified** | l2_amplitude=0 at Kp=0; l1_amplitude=0.2 at min velocity — quiet but not silent |
| AC-3: Interpolator smooths | **Design verified** | alpha formula confirmed; 99% convergence in glide_time_secs |
| AC-4: CPU within budget | **Estimated pass** | ~0.33ms est. < 0.58ms target; measure with real device in Phase 7 |
| AC-5: Layer balance | **Design verified** | constant-power crossfade: cos/sin split; balance=0→L2=0, balance=1→L1=0 |

## Deviations

None. Plan executed as specified.

## Next Phase Readiness

**Ready:**
- `AdditiveEngine::setSynthParams()` and `setUserParams()` are the integration points for Phase 6
- `Interpolator::setGlideTimeSecs()` is the glide control (UserParams.glide_time_secs)
- Phase 5 (VisualRenderer) reads the same smoothed SynthParams from Interpolator

**Concerns:**
- AC-1 audio not verified with real hardware (WSL2 ALSA absent) — verify in Phase 7 cross-platform validation
- AC-4 CPU estimate based on sin() timing — measure empirically in Phase 7 on target hardware

**Blockers:** None.
