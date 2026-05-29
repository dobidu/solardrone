# SolarDrone — ACCEPTANCE

Criteria in Given/When/Then format per phase. Each AC has a measurable outcome. Spike ACs are open-ended — spike output is a decision recorded in SUMMARY, not a GWT criterion.

---

## Phase 01 — Foundation

**AC 1.1 — Multi-format build**
- Given: CMake configured for VST3, AU, and Standalone targets
- When: `cmake --build` runs on Windows 10+, macOS 11+, and Linux desktop (separate CI jobs)
- Then: all three targets produce valid output on all three OSes with no compiler errors

**AC 1.2 — Standalone opens cleanly**
- Given: Standalone binary built on any supported OS
- When: the binary is launched
- Then: an empty window opens; no crash on open or close; no audio output (expected — stub)

**AC 1.3 — VST3 loads in host**
- Given: VST3 built for the host OS
- When: loaded in Reaper (Windows/Linux) or Ableton Live (macOS)
- Then: plugin loads without crash; host shows it in plugin list; no audio output (expected — stub)

---

## Phase 02 — DataFetcher

**Spike (02-99) — open-ended**
The spike does not have a GWT criterion. SUMMARY must answer:
1. Does `JUCE::URL::readEntireTextStream` work in a VST3 background thread in: Standalone, Reaper (Win), Reaper (Linux), Logic Pro (macOS), Ableton Live (macOS)?
2. For any host where it is blocked: what is the exact failure mode (exception, timeout, empty string)?
3. What is the recommended fallback implementation (last-known cache + status message)?

**AC 2.1 — Successful fetch and parse**
- Given: NOAA SWPC API is reachable
- When: DataFetcher runs for 2 minutes from cold start
- Then: SpaceWeatherState updates at least once with non-null velocity, density, Bz, and Kp; `source = "live"`

**AC 2.2 — Null field handling**
- Given: A fixture JSON file where `speed`, `density`, `bz_gsm`, and `kp` fields are all null in the most recent entry
- When: DataFetcher parses the fixture
- Then: SpaceWeatherState retains previous valid values; `source = "cached"`; `data_age_s` increments on subsequent calls

**AC 2.3 — Default state on cold start / unreachable API**
- Given: DataFetcher is initialized with no prior cache and the API is unreachable (simulate with wrong URL)
- When: DataFetcher is queried immediately after init
- Then: SpaceWeatherState has `source = "default"`, velocity=450, density=5, Bz=0, Kp=0; no crash

**AC 2.4 — Unit test coverage**
- Given: Fixture JSON files for (a) quiet day, (b) storm day, (c) all-null fields
- When: parser unit tests run
- Then: all three fixtures parse correctly; asserted field values match expected; tests pass headlessly in CI

---

## Phase 03 — SynthParamMapper

**AC 3.1 — Quiet-day produces low activation**
- Given: SpaceWeatherState with velocity=400, density=5, Bz=+2, Kp=1 and default UserParams
- When: SynthParamMapper is called
- Then: `l1_timbre < 0.3`; `l2_harmonic_density < 0.3`; `l2_amplitude < 0.4`; `l2_brightness < 0.3`

**AC 3.2 — Storm produces high activation**
- Given: SpaceWeatherState with velocity=700, density=20, Bz=−15, Kp=8 and default UserParams
- When: SynthParamMapper is called
- Then: `l1_timbre > 0.7`; `l2_harmonic_density > 0.7`; `l2_amplitude > 0.7`; `l2_brightness > 0.7`

**AC 3.3 — Kp=9 maximum activation**
- Given: SpaceWeatherState with Kp=9
- When: SynthParamMapper is called
- Then: `l2_harmonic_density = 1.0`; `l2_brightness = 1.0`; `l2_amplitude` at defined ceiling (no clipping above 1.0)

**AC 3.4 — Pure function (deterministic)**
- Given: identical SpaceWeatherState and UserParams inputs
- When: SynthParamMapper is called twice in sequence
- Then: both calls return identical SynthParams (byte-for-byte equality)

**AC 3.5 — Harmony mode affects l2_fundamental_hz**
- Given: two UserParams instances differing only in harmony_mode (just vs equal)
- When: SynthParamMapper is called with each
- Then: l2_fundamental_hz differs between calls by a measurable amount; l1_fundamental_hz is unchanged

**AC 3.6 — Listen test (manual, not automated)**
- Given: storm-day and quiet-day fixture states fed through mapper into a prototype sine oscillator
- When: developer listens to both outputs
- Then: audible contrast is present and subjectively meaningful (documented in Phase 03 SUMMARY)

**AC 3.7 — v2 readiness review**
- Given: SynthParams struct at end of Phase 03
- When: developer does "pretend v2.1 exercise" (mentally adds x_ray_flux field to SpaceWeatherState and checks if SynthParams has a natural home for it)
- Then: no breaking change to existing SynthParams fields required; outcome documented in Phase 03 SUMMARY

---

## Phase 04 — AdditiveEngine + Interpolator

**AC 4.1 — Continuous evolution (no steps)**
- Given: Interpolator with glide_time=120s; SynthParams change instantaneously at t=0
- When: 120 seconds of audio are rendered
- Then: output evolves continuously (no sample-level discontinuity); all params reach within 1% of target by t=120s

**AC 4.2 — CPU budget**
- Given: AdditiveEngine with 32 partials per layer, at 44.1kHz / 512 sample buffer
- When: measured on a mid-spec machine (2020-era 8-core laptop)
- Then: CPU usage from the audio thread ≤ 5% during steady-state playback

**AC 4.3 — Drone on/off**
- Given: Drone on/off parameter
- When: toggled off
- Then: output fades to silence within glide_time; no click or discontinuity

**AC 4.4 — Layer balance**
- Given: layer_balance UserParam set to 0.0 (full L1) vs 1.0 (full L2)
- When: audio is rendered with each setting
- Then: at 0.0, L2 is inaudible; at 1.0, L1 is inaudible; at 0.5, both layers audible at equal RMS

---

## Phase 05 — VisualRenderer

**AC 5.1 — Visual changes meaningfully with data**
- Given: VisualRenderer fed SynthParams from a quiet-day state, then switched to storm-day state
- When: states are fed sequentially (with interpolation)
- Then: visual representation changes in a way that is perceptibly different between the two states; change is documented (screen recording or screenshot pair in SUMMARY)

**AC 5.2 — Frame rate**
- Given: VisualRenderer running in standalone window
- When: measured over 10 seconds
- Then: average frame rate ≥ 25fps; no frame takes longer than 80ms (no visible stuttering)

**AC 5.3 — Audio thread isolation**
- Given: VisualRenderer integrated with AudioProcessor
- When: audio thread's `processBlock()` runs while VisualRenderer `paint()` runs concurrently on message thread
- Then: no shared mutable state accessed without thread-safe mechanism (lock-free value or mutex); no audio dropout during visual updates (verified by listening)

**AC 5.4 — Plugin window rendering**
- Given: plugin (VST3) loaded in Reaper on Windows or Linux
- When: plugin editor opens
- Then: VisualRenderer displays and animates in the plugin window without crash; window can be opened and closed repeatedly

---

## Phase 06 — Integration + Full UI

**AC 6.1 — End-to-end cold start**
- Given: Standalone app on a machine with network access
- When: app is launched (cold start, no cached data)
- Then: within 90 seconds, status indicator shows "live"; drone is audible; visual renderer is animating

**AC 6.2 — Status indicator accuracy**
- Given: network connection cut after initial fetch
- When: DataFetcher fails two consecutive polls
- Then: status indicator changes to "cached"; `data_age_s` is displayed and increments; drone continues playing with last-known state

**AC 6.3 — All parameters automatable**
- Given: VST3 loaded in Reaper with automation recording enabled
- When: each user parameter (volume, glide, dynamics, balance, spread, harmony, interval) is adjusted
- Then: each parameter appears in Reaper's automation lane list with correct name and range; automation playback reproduces the parameter movement

**AC 6.4 — Controls affect output**
- Given: drone playing in steady state
- When: each UI control is moved from min to max (one at a time)
- Then: a corresponding change is audible and/or visible in the output; no control is silently ignored

---

## Phase 07 — Cross-platform validation

**AC 7.1 — VST3 Windows (Reaper)**
- Given: VST3 binary for Windows
- When: loaded in Reaper on Windows 10+
- Then: loads without crash; drone plays; parameters automate; unloads cleanly; no crash on DAW close

**AC 7.2 — VST3 Linux (Reaper)**
- Given: VST3 binary for Linux
- When: loaded in Reaper on Linux desktop
- Then: same as AC 7.1

**AC 7.3 — AU macOS (Logic Pro)**
- Given: AU binary for macOS, Logic Pro with sandbox active
- When: plugin loads in Logic Pro
- Then: plugin loads without crash; if HTTP is blocked, status indicator shows "offline / no data" with a descriptive message; no crash; drone plays with default state

**AC 7.4 — Standalone all OSes**
- Given: Standalone binary for each OS
- When: launched on each OS
- Then: opens cleanly, fetches live data, plays drone with visual; no crash on close

**AC 7.5 — CI build matrix**
- Given: GitHub Actions workflow configured for Windows, macOS, Linux
- When: workflow runs on push to main
- Then: all three builds complete without error; artifacts uploaded

---

## Phase 08 — Docs + Release

**AC 8.1 — New user can install and run**
- Given: a developer who has not seen this project before follows the README on a fresh machine
- When: they complete the "Quick start" section
- Then: they can build and run the standalone app; drone produces sound; no step requires out-of-band knowledge

**AC 8.2 — Demo recording shows contrast**
- Given: a 10-minute audio recording made during a quiet day (Kp ≤ 2) and a 10-minute recording during an active day (Kp ≥ 5)
- When: both recordings are listened to
- Then: audible difference is present and documentable; included in GitHub release notes

**AC 8.3 — Sandbox behavior documented**
- Given: README file
- When: a user searches for "Logic" or "sandbox" or "offline"
- Then: README explains which hosts may block HTTP, what the plugin does in that case, and that this is a known v1 limitation

---

## Note on spike ACs

Spike ACs (02-99) are open-ended by design. A spike's acceptance is a decision recorded in SUMMARY.md, not a GWT criterion. A spike is "done" when the SUMMARY answers the stated question and the main phase plan can be written without ambiguity about the spiked risk.
