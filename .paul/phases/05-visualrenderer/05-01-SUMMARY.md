---
phase: 05-visualrenderer
plan: 01
subsystem: ui
tags: [visual, lissajous, particles, spectral, juce-component, juce-timer]

requires:
  - phase: 04-additiveengine
    provides: SynthParams via AdditiveEngine.getSmoothedParams()

provides:
  - VisualRenderer: three-layer animated JUCE Component, 30fps, SynthParams-driven
  - Layer 1 Lissajous: parametric curve driven by L1/L2 frequency ratio, phase drifts with l1_timbre
  - Layer 2 Particles: ~50 particles, speed from l1_amplitude, turbulence from l1_timbre, hue from l2_brightness
  - Layer 3 Spectral: harmonic bars (L1) + density overlay (L2) at bottom of window
  - UserParams: 3 blend weight fields added (visual_lissajous/particles/spectral = 0.7 default)
  - PluginEditor: VisualRenderer fills full window; "SolarDrone" label overlay

affects: [06-integration]

tech-stack:
  added: [juce::Component, juce::Timer (30fps), juce::Path, juce::CriticalSection, juce::Random]
  patterns:
    - VisualRenderer polls processor via Timer callback (not push from audio thread)
    - CriticalSection for SynthParams handoff (read on message thread, safe)
    - AdditiveEngine.getSmoothedParams() protected by smoothedLock (CriticalSection)
    - Three layers drawn sequentially: spectral(bg) → particles(mid) → lissajous(top)

key-files:
  created:
    - src/VisualRenderer.h / .cpp
  modified:
    - src/UserParams.h (visual_lissajous, visual_particles, visual_spectral added)
    - src/AdditiveEngine.h / .cpp (getSmoothedParams + smoothedLock)
    - src/PluginProcessor.h / .cpp (getCurrentSynthParams)
    - src/PluginEditor.h / .cpp (VisualRenderer embedded, full window)

key-decisions:
  - "Visual metaphor: 3 layers composited (Lissajous + particles + spectral), user-chosen"
  - "Timer poll pattern (not audio thread push) — safe, simple, sufficient at 30fps"
  - "Lissajous ratio = l2_fundamental_hz / l1_fundamental_hz (default ~1.5 = perfect 5th)"
  - "Phase drift rate proportional to l1_timbre: calm=slow, storm=fast morphing"
  - "Particle hue: blue-violet (Kp=0) → orange (Kp=9) via l2_brightness"
  - "Blend weights in UserParams defaults 0.7 — Phase 6 wires to AudioProcessorParameters"

patterns-established:
  - "VisualRenderer forward-declares SolarDroneAudioProcessor to break circular include"
  - "Particles capped at kMaxParticles=80; spawn/die controlled by l2_harmonic_density"

duration: ~1 session
started: 2026-05-29T00:00:00Z
completed: 2026-05-29T00:00:00Z
---

# Phase 5 Plan 01: VisualRenderer Summary

**Three-layer animated visual (Lissajous + particles + spectral) composited in PluginEditor at 30fps; checkpoint human-verified — visual animates correctly; 15/15 unit tests pass.**

## Performance

| Metric | Value |
|--------|-------|
| Tasks | 2 + human checkpoint completed |
| Files created | 2 (+ 5 modified) |
| Frame rate | 30fps via juce::Timer |
| Unit tests | 15/15 pass (unchanged) |
| Checkpoint | approved — visual animates correctly in standalone |

## Acceptance Criteria Results

| Criterion | Status | Notes |
|-----------|--------|-------|
| AC-1: Visual changes with SynthParams | **Pass** (human-verified) | Checkpoint approved — visual animates |
| AC-2: Frame rate ≥25fps | **Pass** | 30fps Timer; paint() completes within frame |
| AC-3: Audio thread isolation | **Pass** | CriticalSection on both SynthParams read and AdditiveEngine.getSmoothedParams() |
| AC-4: Renders in plugin window | Pending Phase 7 | VST3 in Reaper verified in Phase 7 |

## Visual Layer Mapping

| Layer | Driven by | Effect |
|-------|-----------|--------|
| Lissajous | l1_hz/l2_hz ratio, l1_timbre (phase drift), l1_amplitude (thickness) | Parametric curve morphs from stable figure to chaotic form as Bz drops |
| Particles | l1_amplitude (speed), l1_timbre (turbulence), l2_harmonic_density (count), l2_brightness (hue) | Field shifts from calm blue drift to turbulent orange storm at Kp≥6 |
| Spectral | l1_harmonic_count (L1 bars), l1_timbre (warp), l2_harmonic_density (L2 lines), l2_brightness (color) | Harmonic landscape at bottom, density overlay grows with Kp |

## Next Phase Readiness

**Ready:**
- VisualRenderer blend weights (visual_lissajous/particles/spectral) in UserParams → Phase 6 exposes as parameters
- `processor->getCurrentSynthParams()` pattern established for editor→engine polling
- PluginEditor window set to 600×400; Phase 6 adds UI controls overlaid on visual

**Concerns:**
- AC-4 (plugin window in host) verified in Phase 7 only
- Particle positions and Lissajous phase not reset on plugin close/reopen — state persists as long as editor object lives (acceptable for installation context)

**Blockers:** None.
