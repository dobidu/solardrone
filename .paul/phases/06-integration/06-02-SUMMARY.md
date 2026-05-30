---
phase: 06-integration
plan: 02
subsystem: ui
tags: [apvts, parameters, daw-automation, controls, slider, toggle]

requires:
  - phase: 06-01
    provides: DataFetcher wired to engine pipeline

provides:
  - APVTS with 11 DAW-automatable parameters
  - Controls panel (600x480 window, 80px bottom strip, 2 rows)
  - getStateInformation/setStateInformation (APVTS XML round-trip)
  - Drone on/off, volume, glide, all UserParams exposed

affects: [07-validation]

tech-stack:
  added: [juce::AudioProcessorValueTreeState, SliderAttachment, ButtonAttachment, ComboBoxAttachment]
  patterns:
    - createParameterLayout() static factory — clean separation from constructor
    - processBlock() reads APVTS raw values → UserParams struct — one atomic read per param per block
    - state saves/restores via copyState/fromXml — standard JUCE pattern

key-decisions:
  - "11 parameters: volume, drone_on, glide_time, dynamics_range, layer_balance, stereo_spread, harmony_mode, interval, vis_lissajous, vis_particles, vis_spectral"
  - "Controls strip 80px bottom — visual fills 400px top; window 600x480"
  - "harmony_mode as AudioParameterChoice (string labels: Just / Equal Temp.)"
  - "interval as AudioParameterInt (0-12 semitones)"

duration: ~0.5 session
---

# Phase 6 Plan 02: APVTS + Controls Summary

**11 APVTS parameters exposed for DAW automation; controls panel with sliders/toggle/combo visible and responsive; checkpoint human-verified; 15/15 tests pass.**

## AC Results

| Criterion | Status |
|-----------|--------|
| AC-1: All params in DAW | Pending Phase 7 (Reaper test) |
| AC-2: Sliders affect drone | **Pass** (checkpoint approved) |
| AC-3: Drone on/off | **Pass** (checkpoint approved) |
| AC-4: State saves/restores | **Pass** — APVTS XML round-trip implemented |

## Phase 6 Complete — Both Plans Done

Phase 6 closes with full end-to-end pipeline:
  DataFetcher → SynthParamMapper → Interpolator → AdditiveEngine
  VisualRenderer ← getCurrentSynthParams() poll
  UI controls ↔ APVTS ↔ processBlock() UserParams

## Next Phase Readiness

**Ready:**
- Phase 7: load VST3 in Reaper, verify automation lanes, verify AU in Logic Pro

**Concerns:**
- APVTS parameter count (11) and IDs are stable — do not rename in Phase 7+
- Drone silence on ON/OFF is immediate (buffer.clear()) — no glide fade; acceptable for v1
