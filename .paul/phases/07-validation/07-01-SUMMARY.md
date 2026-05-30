---
phase: 07-validation
plan: 01
subsystem: infra
tags: [ci, readme, documentation, validation]

provides:
  - CI matrix updated with headless test run on Linux
  - README.md: install, build, quick start, controls table, architecture, known limits, license
  - Sandbox limitation documented (Logic Pro/Pro Tools HTTP block)

key-decisions:
  - "Test run in CI Linux-only (macOS needs audio mock for juce::Thread; Windows path differs)"
  - "Manual host validation (Reaper VST3, Logic Pro AU) documented as pending hardware"
  - "AU code signing documented but not automated (requires Apple Developer account + CI secrets)"

duration: ~0.5 session
---

# Phase 7 Plan 01: Validation + README Summary

**CI matrix updated with headless test run; README complete (149 lines); all 3 ACs pass.**

## AC Results

| Criterion | Status |
|-----------|--------|
| AC-1: CI runs tests headlessly | **Pass** — YAML valid; Linux step runs SolarDroneTests |
| AC-2: README covers install + quick start | **Pass** — 149 lines, full build + install + controls table |
| AC-3: Sandbox documented | **Pass** — "Logic Pro / Pro Tools" section in Known Limitations |

## Pending Manual Validation

These items require hardware not available on Linux/WSL2:

| Item | Requires | Status |
|------|----------|--------|
| VST3 in Reaper Windows | Windows + Reaper | Pending |
| VST3 in Reaper Linux | Reaper install | Pending (Reaper not installed) |
| VST3 in Ableton Live macOS | macOS + Ableton | Pending |
| AU in Logic Pro macOS | macOS + Logic + Apple dev cert | Pending |
| macOS code signing | Apple Developer account | Documented only |
| CI test run on macOS | (done — build-only) | Build ✓ |
| CI test run on Windows | (done — build-only) | Build ✓ |

CI validates builds on all 3 OSes. Test binary runs headlessly on Linux in CI.

## Next

Phase 8: Docs (DESIGN.md) + demo recording + GitHub release.
