# Project State

## Project Reference

See: .paul/PROJECT.md (updated 2026-05-29 after Phase 1)

**Core value:** Makes space weather audible and aesthetically meaningful — educational tool and artistic installation material
**Current focus:** Phase 8 — Docs + Release (DESIGN.md, git tag v1.0.0, GitHub release)

## Current Position

Milestone: v1.0 Initial Release (v1.0.0)
Milestone: v2.2 — UI/UX Overhaul (in progress)
Phase: v2.2.1 — Layout Foundation (planning)
Phase: v2.1.2 (Chopper + UI) — complete
Plan: unified
Status: MILESTONE COMPLETE — ready for v2.2
Last activity: 2026-05-30 — v2.1.2 checkpoint approved; BeatRepeater+Chopper+UI shipped

Progress:
- Milestone v2.1: [██████████] 100%

## Loop Position

Current loop state:
```
PLAN ──▶ APPLY ──▶ UNIFY
  ✓        ○        ○     [v2.1.1 plan created, awaiting APPLY]
```

## Accumulated Context

### Decisions

| Decision | Phase | Impact |
|----------|-------|--------|
| SynthParamMapper as canonical product (`SpaceWeatherState → SynthParams`) | Init | AdditiveEngine + VisualRenderer are independent renderizações; v2 extensibility preserved |
| Hybrid mapping (linear base + Bz/Kp non-linear activations) | Init | Aesthetic quality over data fidelity; quiet = sparse, storm = dense/tense |
| JUCE 8.0.4 via FetchContent GIT_SHALLOW=TRUE | Phase 1 | All subsequent phases use JUCE 8 API; CI first run ~5min, subsequent runs fast |
| JUCE_USE_CURL platform-conditional (Linux=1+libcurl, macOS/Windows=0) | Phase 2 spike | CMakeLists.txt updated; CURL::libcurl linked on Linux; native stacks on macOS/Win |
| SynthParamMapper no JUCE dependency — pure C++/cmath | Phase 3 | Compiles in any context; thread-safe by design |
| l2_amplitude raw (no layer_balance crossfade in mapper) | Phase 3 | AdditiveEngine applies constant-power crossfade ✓ done |
| Control-rate architecture in processBlock() | Phase 4 | ~100Hz sub-blocking; applyParams() not on audio thread |
| JUCE::URL probe must launch from constructor, not prepareToPlay | Phase 2 spike | Audio device may never init in WSL2/headless — DataFetcher must start its own thread unconditionally |
| Fallback: empty readEntireTextStream → source="cached", increment data_age_s | Phase 2 spike | No exception from JUCE on failure; silent empty string — DataFetcher checks isEmpty() |
| AU not in CI — code signing deferred to Phase 7 | Phase 1 | CI validates VST3+Standalone; AU validated manually in Phase 7 |

### Deferred Issues

| Issue | Origin | Effort | Revisit |
|-------|--------|--------|---------|
| Visual metaphor selection (Lissajous / particles / spectral / geometric) | Init | S | Start of Phase 5 |
| SynthParamMapper v2 readiness review | Init | S | End of Phase 3 |

### Blockers/Concerns

| Blocker | Impact | Resolution Path |
|---------|--------|-----------------|
| Logic Pro / Ableton macOS sandbox blocks HTTP silently | Plugin uses last-known state in those hosts | ✅ Resolved by spike — fallback implemented in DataFetcher; documented in README |

## Session Continuity

Last session: 2026-06-01
Stopped at: v2.9.0 released — SolarTerminal + REP/CHOP expanded + UI audit complete
Next action: Preset System (v2.10) — save/load named APVTS patches
Resume file: .paul/HANDOFF-2026-06-01.md
Resume context:
- v2.9.0 tagged and on GitHub as latest, all docs updated
- Window: 1350×780 full / 690×890 compact
- SolarTerminal: src/SolarTerminal.h/.cpp (new component in ResonatorPanel)
- 6 new DSP params: repeater_reverse/pan/stutter + chopper_attack/release/phase
- ScopedNoDenormals in processBlock + ResonatorEngine::process (critical for Windows)
- Resonators default OFF (prevents IIR instability on first launch)
- No blockers — roadmap complete through v2.9

---
*STATE.md — Updated after every significant action*
