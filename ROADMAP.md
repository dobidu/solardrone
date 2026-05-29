# SolarDrone — ROADMAP

## v1 target

Standalone app + VST3 + AU that sonifies live NOAA SWPC solar wind and Kp data as a two-layer additive drone, with abstract/artistic visual representation, on Windows / macOS / Linux. All user parameters exposed for DAW automation. Runs as unattended installation or studio tool.

## v2 target (registered, post-v1)

- v2.1 — Additional NOAA data sources: X-ray flux (solar flare class), proton flux. Plug into `SpaceWeatherState`.
- v2.2 — Historical playback: load NOAA archive data, replay as drone.
- v2.3 — User-editable mapping curves: expose SynthParamMapper logic as UI-editable curves.

## Future ecosystem (not roadmapped)

OSC output (SynthParams → OSC for live coding / VJ tools); binaural / Ambisonics rendering; web companion (visual + data dashboard); mobile port.

## Spike protocol

Phases with material technical risk start with a `/paul:plan <phase> --quick-fix` **before** the main plan. The quick-fix produces a short PLAN.md + SUMMARY.md with the technical decision, stored as `phases/0X-name/0X-99-PLAN.md` and `0X-99-SUMMARY.md` (suffix 99 reserved for the phase spike). The spike's `/paul:unify` writes the decision into STATE.md, which the main plan's coherence check consumes.

Spikes are not product milestones. They are technical obligations inside the phase that needs them. The SUMMARY is the input to the main PLAN of the phase.

Spikes required in v1:

- **Phase 02** — HTTP requests in plugin context. Test `JUCE::URL` on background thread inside VST3 in: Standalone, Reaper (Windows + Linux), Logic Pro (macOS), one other host. Determine: does it work? What is the fallback strategy when blocked?

Phase v2.2 (archive playback) will likely require a spike on NOAA archive API structure and field availability.

## Milestones

- **M1 — Builds clean on all 3 OSes.** CMake produces VST3 + AU + Standalone on Windows, macOS, Linux. No audio yet.
- **M2 — Live data flowing.** DataFetcher running; SpaceWeatherState updating from NOAA in debug output within 2min of cold start.
- **M3 — Drone sounds.** AdditiveEngine producing audio driven by live SynthParams. Standalone working end-to-end.
- **M4 — Full UI.** VisualRenderer active, all user parameters exposed in UI and as DAW automation parameters.
- **M5 — All formats validated, docs, v1 cut.** VST3 + AU + Standalone verified on all OSes. README complete. GitHub release.

## Phases — v1

### Phase 01 — Foundation

Goal: JUCE project builds on all three OSes with empty AudioProcessor and basic UI shell. Repo has PAUL structure. Graphify baseline committed.

Tasks (indicative; `/paul:plan` produces final list):

- Create JUCE project with CMake (no Projucer).
- Configure VST3 + AU + Standalone build targets.
- Stub AudioProcessor: no audio, no parameters yet.
- Stub AudioProcessorEditor: empty window opens without crash.
- Verify clean CMake build on Windows 10+, macOS 11+, Linux desktop.
- CI matrix skeleton (GitHub Actions): build on all 3 OSes.
- Run `/graphify .`, commit `graphify-out/`.

Spike: not required.

Acceptance: see ACCEPTANCE.md Phase 01.

---

### Phase 02 — DataFetcher

Goal: Live NOAA SWPC data flowing into a `SpaceWeatherState` struct, with null handling and last-known-state cache.

**Spike (02-99): HTTP in plugin context.**
Question: Does `JUCE::URL::readEntireTextStream` (or `DownloadTask`) work reliably in a background thread inside a VST3 in each of: Standalone, Reaper (Win + Linux), Logic Pro (macOS), Ableton Live (macOS)? If blocked, what is the best fallback (cached state + status message)?
SUMMARY must decide: HTTP approach to use; hosts where it works; hosts where it is blocked; fallback implementation.

Main plan tasks:

- `DataFetcher` class: JUCE `Thread` subclass, polls at configurable interval (default 60s).
- HTTP GET `rtsw_wind_1m.json`: parse most recent entry's `speed`, `density`, `bz_gsm`.
- HTTP GET `planetary_k_index_1m.json`: parse most recent entry's estimated Kp.
- Null field handling: retain last valid value; increment `data_age_s`.
- `SpaceWeatherState` struct with atomic update (lock-free or mutex); `source` field ("live" / "cached" / "default").
- Default state for cold start / API unreachable: velocity=450, density=5, Bz=0, Kp=0, source="default".
- Unit tests: parse fixture JSON files (quiet day, storm day, nulls-in-fields), assert fields extracted correctly and defaults applied.

Acceptance: see ACCEPTANCE.md Phase 02.

---

### Phase 03 — SynthParamMapper

Goal: Pure function `(SpaceWeatherState, UserParams) → SynthParams`. Hybrid mapping (linear base + non-linear activations). Unit-tested with real NOAA snapshot data.

Tasks:

- `SynthParams` struct (see PROJECT.md data model).
- `UserParams` struct: dynamics_range, layer_balance, stereo_spread, harmony_mode enum, l1_l2_interval_semitones.
- Linear base mappings:
  - velocity (300–800 km/s) → l1_fundamental_hz (log-scale, e.g. 55–220 Hz).
  - density (1–50 p/cm³) → l1_harmonic_count (2–24 partials, clamped).
  - Kp (0–5) → l2_amplitude (0.1–0.5 scaled by dynamics_range UserParam).
  - l1_timbre = 0 when Bz ≥ 0.
- Non-linear activations:
  - Bz ≤ −10 nT: sigmoid ramp → l1_timbre 0→1; adds partial detuning factor.
  - Kp ≥ 6: exponential ramp → l2_harmonic_density and l2_brightness swell.
  - Kp = 9: all params clamped to maximum activation values.
- l2_fundamental_hz derived from l1_fundamental_hz + l1_l2_interval_semitones UserParam.
- Harmony mode: just intonation, equal temperament, or fixed ratio applied to partial frequencies.
- Pure function: no side effects, no state, deterministic.
- Unit tests: quiet-day fixture → low activation asserts; storm-day fixture → high activation asserts; identical inputs → identical outputs.
- Listen test: feed fixture states through mapper + prototype sine oscillator; verify audible contrast between quiet and storm.

Acceptance: see ACCEPTANCE.md Phase 03.

---

### Phase 04 — AdditiveEngine + Interpolator

Goal: Two-layer additive oscillator bank producing continuous drone audio. Interpolator ensuring smooth parameter evolution.

Tasks:

- `Oscillator` class: wavetable or BLIT sinusoid; frequency and amplitude per partial.
- `OscillatorBank` class: N partials, `setSynthParams(SynthParams)` applies mapping to individual partials.
- Layer 1 bank: partials tuned to l1_fundamental_hz × harmonic series ratios per harmony_mode; count = l1_harmonic_count; amplitude tapers with partial index; l1_timbre modulates spectral envelope slope.
- Layer 2 bank: partials at l2_fundamental_hz; density controlled by l2_harmonic_density; brightness by l2_brightness.
- Stereo panning: per-layer, layers panned opposite sides scaled by stereo_spread UserParam.
- `Interpolator` class: per-field slew filter at control rate (~100Hz); rate derived from GlideTime UserParam.
- `processBlock()`: pull smoothed SynthParams from Interpolator at control rate; render partials at audio rate; sum and output stereo buffer.
- CPU profiling: measure % CPU with 32 partials/layer at 44.1kHz / 512 samples on reference machine. Target: <5%.
- If CPU target missed: reduce default partial count or add lookup-table oscillators.

Acceptance: see ACCEPTANCE.md Phase 04.

---

### Phase 05 — VisualRenderer

Goal: Abstract/artistic JUCE Component driven by smoothed `SynthParams`. Renders at ≥25fps without blocking audio thread.

Tasks:

- Choose visual metaphor at start of phase (candidates: Lissajous curves driven by L1/L2 amplitudes; particle field density driven by l1_harmonic_count and l2_harmonic_density; spectral envelope shape as morphing curve; concentric geometric forms driven by Kp activation). Decision logged in SUMMARY before implementation starts.
- JUCE `Component` subclass with `paint()` override (software renderer baseline).
- Map SynthParams fields to visual parameters (documented in code comments, one line per mapping).
- Update via JUCE `Timer` at 30fps; reads latest smoothed SynthParams via shared pointer or lock-free ring buffer from audio thread.
- Optional: OpenGL accelerated path with software fallback for plugin hosts that don't support OpenGL windows.
- Test in standalone window and in plugin window (fixed-size, no resize assumed).

Acceptance: see ACCEPTANCE.md Phase 05.

---

### Phase 06 — Integration + Full UI

Goal: All components wired. Full UI with all controls. All AudioProcessorParameters exposed for DAW automation.

Tasks:

- Wire: DataFetcher → SynthParamMapper → Interpolator → AdditiveEngine → audio output.
- Wire: Interpolator output → VisualRenderer (read via lock-free shared value, not audio thread direct call).
- UI controls: status indicator (source, data_age_s, last Kp value as text), drone on/off toggle, master volume, glide time, dynamics range, layer balance, stereo spread, harmony mode selector, L1↔L2 interval.
- Expose all parameters as `AudioProcessorParameter`s with correct range, default, and name. Verify automation in Reaper.
- Status indicator updates: green = "live" + timestamp; yellow = "cached" + age; red = "default / no data".
- End-to-end smoke test: cold start → data fetch → SynthParams computed → drone audible → visual active → all controls affect output.

Acceptance: see ACCEPTANCE.md Phase 06.

---

### Phase 07 — Cross-platform validation + CI

Goal: All three plugin formats verified on all three OSes. CI builds pass. macOS code-signed for AU.

Tasks:

- VST3 in Reaper on Windows and Linux — load, play, automate parameters, unload cleanly.
- VST3 in Ableton Live on Windows and macOS.
- AU in Logic Pro on macOS — load, verify HTTP behavior (sandbox likely blocks), confirm "cached / default" state message visible.
- Standalone on all three OSes.
- macOS code signing: at minimum developer ID signing for AU notarization.
- CI matrix (GitHub Actions): CMake build + headless startup smoke test on Windows, macOS, Linux.
- Document host-specific behavior in README (HTTP sandbox hosts list).

Acceptance: see ACCEPTANCE.md Phase 07.

---

### Phase 08 — Docs + Release

Goal: Cut v1.

Tasks:

- README: install (build from source + prebuilt release), quick start, known limits, plugin sandbox note.
- Demo recording: 10-minute session quiet day (Kp ≤ 2) vs. active day (Kp ≥ 5) — audible and visual difference documented.
- DESIGN.md: architecture summary for contributors.
- GitHub release: tagged binaries (VST3 Windows + Linux, VST3 + AU macOS, Standalone all 3 OSes). License file included.

Acceptance: see ACCEPTANCE.md Phase 08.

---

## Phases — v2 (registered, not planned)

### Phase v2.1 — Additional data sources

Goal: Plug X-ray flux (solar flare class B/C/M/X) and proton flux into SpaceWeatherState. Map new fields to new SynthParams dimensions without breaking existing mapping.

Spike likely: NOAA X-ray flux API structure and update cadence.

### Phase v2.2 — Historical playback

Goal: Load NOAA archive JSON data, replay as drone in real or accelerated time.

Spike likely: NOAA archive API endpoints, date range availability, field format consistency.

### Phase v2.3 — User-editable mapping curves

Goal: Expose SynthParamMapper logic as curve editors in UI. Mapping becomes data, not code.

---

## Risks to track in STATE.md

- **HTTP sandbox in plugin hosts** (Logic Pro, Pro Tools): Phase 02 spike resolves. Mitigation: last-known-state fallback + documented limitation.
- **NOAA API field name changes**: JSON field names are undocumented contracts. Mitigation: narrow parser, integration test that hits real endpoint, version-pin fixture files.
- **Additive CPU overhead**: 32+ oscillators per block. Mitigation: Phase 04 profiling, lookup-table oscillators if needed, per-bank partial cap.
- **Aesthetic validation subjectivity**: "sounds good" is not a unit test. Mitigation: Phase 03 listen test + Phase 08 demo recording (quiet vs storm comparison).
- **macOS code signing**: required for AU validation; needs Apple developer account. Mitigation: document unsigned dev path; signing in Phase 07.
- **Null NOAA data in consecutive fetches**: telemetry gaps can last minutes. Mitigation: last-valid cache in DataFetcher; status indicator shows data age to user.
- **VisualRenderer blocking audio thread**: any paint() call accidentally crossing thread boundary would cause audio dropout. Mitigation: lock-free shared value pattern, enforced in Phase 05.
- **SynthParamMapper as v2 contract**: if mapper output shape is wrong in v1, v2.1 will need a breaking refactor. Mitigation: review SynthParams struct against v2.1 sketch at end of Phase 03.
