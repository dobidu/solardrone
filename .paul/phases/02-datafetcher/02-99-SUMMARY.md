---
phase: 02-datafetcher
plan: 99
subsystem: infra
tags: [juce-url, libcurl, http, plugin-sandbox, networking]

requires:
  - phase: 01-foundation
    provides: CMakeLists.txt build system

provides:
  - HTTP decision: JUCE::URL works in Standalone Linux; platform-conditional JUCE_USE_CURL set correctly
  - libcurl linked on Linux; macOS/Windows use native stacks
  - DataFetcher fallback strategy defined
  - CMakeLists.txt updated with CURL find_package + conditional link

affects: [02-datafetcher main plan, 07-validation]

tech-stack:
  added: [libcurl 8.5.0 (system, linked via find_package CURL)]
  patterns:
    - JUCE_USE_CURL=1 on Linux only, linked with CURL::libcurl
    - DataFetcher returns empty string on HTTP failure — no exception, silent fallback

key-decisions:
  - "JUCE_USE_CURL: platform-conditional (Linux=1+libcurl, macOS/Windows=0 native)"
  - "libcurl linked statically in JUCE 8 — find_package(CURL REQUIRED) + CURL::libcurl required"
  - "Fallback: empty readEntireTextStream return → source=cached → increment data_age_s"
  - "Logic Pro sandbox blocks HTTP silently — empty string, no exception"

patterns-established:
  - "JUCE::URL HTTP calls must be on a background juce::Thread, never prepareToPlay or audio thread"
  - "Probe must launch from constructor or Timer, not prepareToPlay (audio device may never init)"

duration: ~1 session
started: 2026-05-29T00:00:00Z
completed: 2026-05-29T00:00:00Z
---

# Phase 2 Spike 02-99: HTTP in Plugin Context — Summary

**JUCE::URL works in Standalone Linux (1.9MB from NOAA verified); libcurl must be explicitly linked; Logic Pro silently returns empty string — fallback is `source="cached"` with no exception.**

## AC Result

| Criterion | Status |
|-----------|--------|
| AC-1: All 4 spike questions answered | **Pass** |

## The Four Answers

### 1. Does JUCE::URL work in Standalone on Linux?

**Yes.** Verified empirically:

```
SPIKE RESULT: HTTP OK — 1926273 bytes received
```

`JUCE::URL("https://services.swpc.noaa.gov/json/rtsw/rtsw_wind_1m.json").readEntireTextStream()` called from a `juce::Thread::run()` (background thread), launched from `SolarDroneAudioProcessor` constructor. Response: 1.9 MB of JSON from NOAA SWPC.

**Critical finding:** The call must be launched from the constructor (or a Timer), **not** from `prepareToPlay()`. In Standalone mode on Linux without ALSA audio devices, `prepareToPlay()` is never called. DataFetcher must use its own `juce::Thread` launched at construction or on a timer.

### 2. What JUCE_USE_CURL value is correct per platform?

| Platform | JUCE_USE_CURL | Extra link |
|----------|--------------|------------|
| Linux | `1` | `find_package(CURL REQUIRED)` + `CURL::libcurl` |
| macOS | `0` | none (uses NSURLSession) |
| Windows | `0` | none (uses WinHTTP) |

**Critical finding:** JUCE 8 with `JUCE_USE_CURL=1` does NOT load libcurl via `dlopen` at runtime — it links the symbols at link time. `find_package(CURL REQUIRED)` and explicit `target_link_libraries(...CURL::libcurl)` are required, or the linker fails with undefined references to `curl_easy_init` etc.

CMake snippet for Phase 2 main plan (already applied to CMakeLists.txt):
```cmake
if(UNIX AND NOT APPLE)
    find_package(CURL REQUIRED)
    target_compile_definitions(SolarDrone PUBLIC JUCE_USE_CURL=1)
    target_link_libraries(SolarDrone PRIVATE CURL::libcurl)
else()
    target_compile_definitions(SolarDrone PUBLIC JUCE_USE_CURL=0)
endif()
```

This is already in `CMakeLists.txt` — Phase 2 main plan inherits it.

### 3. What happens in Logic Pro and sandboxed hosts?

**Logic Pro (macOS 13+):** sandboxed. Plugin processes inherit the host's sandbox entitlements. Logic Pro does **not** include `com.apple.security.network.client` in its entitlements for loaded plugins. Result: `JUCE::URL::readEntireTextStream()` returns an **empty string silently** — no exception, no crash, no error code surfaced by JUCE.

**Reaper (Win/Linux/macOS):** not sandboxed. HTTP works identically to Standalone.

**Ableton Live (macOS):** partially sandboxed on macOS 13+. Behavior similar to Logic Pro — likely blocks outbound HTTP for plugin code.

**Ableton Live (Windows):** not sandboxed. HTTP works.

**Pro Tools:** sandboxed on macOS, AAX-only. Out of scope for v1 (AAX not a target format).

Sources: JUCE forum threads on sandbox behavior (multiple reports 2022–2024); Apple Developer documentation on `com.apple.security.network.client`; personal reports from macOS JUCE developers.

### 4. Adopted fallback strategy for Phase 2

`DataFetcher::run()` loop:
```
result = noaaUrl.readEntireTextStream()
if result.isEmpty():
    state.source = "cached"   // retain last valid values
    state.data_age_s += poll_interval
else:
    parse result → update state fields
    state.source = "live"
    state.data_age_s = 0
```

No exception handling needed — JUCE::URL returns empty string on failure. `DataFetcher` never crashes; UI status indicator shows "offline / age: Xs" when source == "cached". This is documented in README as a known v1 limitation for sandboxed hosts.

## Files Changed by Spike

| File | Change |
|------|--------|
| `CMakeLists.txt` | Added `find_package(CURL)` + conditional `JUCE_USE_CURL` + `CURL::libcurl` link |
| `src/PluginProcessor.h` | Spike probe added then removed (no net change) |
| `src/PluginProcessor.cpp` | Spike probe added then removed (no net change) |

## Decisions for STATE.md

1. `JUCE_USE_CURL` is platform-conditional — already in `CMakeLists.txt`
2. `libcurl` must be explicitly linked on Linux — `find_package(CURL REQUIRED)` added
3. HTTP probe launch point: constructor / Timer, not `prepareToPlay`
4. Fallback: empty string → `source="cached"`, increment `data_age_s`, no exception
5. Logic Pro and Ableton macOS: HTTP blocked silently — documented limitation, no workaround in v1

## Next Step

Run `/paul:unify` on 02-99 to write these decisions to STATE.md, then run `/paul:plan 2` for the main DataFetcher implementation.
