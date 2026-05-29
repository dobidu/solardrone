# Project State

## Project Reference

See: .paul/PROJECT.md (updated 2026-05-29 after Phase 1)

**Core value:** Makes space weather audible and aesthetically meaningful — educational tool and artistic installation material
**Current focus:** Phase 3 — SynthParamMapper (pure function SpaceWeatherState → SynthParams)

## Current Position

Milestone: v1.0 Initial Release (v1.0.0)
Phase: 3 of 8 (SynthParamMapper) — Planning
Plan: 03-01 created, awaiting approval
Status: PLAN created, ready for APPLY
Last activity: 2026-05-29 — Created .paul/phases/03-synthmapper/03-01-PLAN.md

Progress:
- Milestone: [██░░░░░░░░] 25%
- Phase 3: [░░░░░░░░░░] 0%

## Loop Position

Current loop state:
```
PLAN ──▶ APPLY ──▶ UNIFY
  ✓        ○        ○     [Plan 03-01 created, awaiting approval]
```

## Accumulated Context

### Decisions

| Decision | Phase | Impact |
|----------|-------|--------|
| SynthParamMapper as canonical product (`SpaceWeatherState → SynthParams`) | Init | AdditiveEngine + VisualRenderer are independent renderizações; v2 extensibility preserved |
| Hybrid mapping (linear base + Bz/Kp non-linear activations) | Init | Aesthetic quality over data fidelity; quiet = sparse, storm = dense/tense |
| JUCE 8.0.4 via FetchContent GIT_SHALLOW=TRUE | Phase 1 | All subsequent phases use JUCE 8 API; CI first run ~5min, subsequent runs fast |
| JUCE_USE_CURL platform-conditional (Linux=1+libcurl, macOS/Windows=0) | Phase 2 spike | CMakeLists.txt updated; CURL::libcurl linked on Linux; native stacks on macOS/Win |
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

Last session: 2026-05-29
Stopped at: Phase 1 complete, loop closed, transitioned to Phase 2
Next action: `/paul:plan 3` — SynthParamMapper (pure function, hybrid mapping, unit tests)
Resume context: SpaceWeatherState struct in src/SpaceWeatherState.h. SynthParams struct to define. Linear base + non-linear activations (Bz ≤-10nT, Kp ≥6). v2 readiness review at end.

---
*STATE.md — Updated after every significant action*
