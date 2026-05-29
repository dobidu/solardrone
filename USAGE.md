# SolarDrone — USAGE

## The four files and what they're for

| File | Purpose |
|------|---------|
| `PROJECT.md` | Full project context: what, why, who, architecture, stack, glossary. Feed to `/paul:init`. |
| `ROADMAP.md` | Phases, milestones, spike protocol, risks. The development plan skeleton. |
| `ACCEPTANCE.md` | Given/When/Then criteria per phase. Seed for `/paul:plan` calls and QA. |
| `USAGE.md` | This file — toolchain setup and command flow. |

---

## Toolchain setup

Install in this order. Each tool has a gate — don't skip.

### 1. Caveman (optional, tokens)

If you want compressed communication mode:

```bash
# Install caveman skill globally
mkdir -p ~/.claude/skills/
cp -r path/to/caveman ~/.claude/skills/
# Invoke in Claude Code: /caveman
```

### 2. PAUL framework

```bash
npx paul-framework@latest init
# or
npm install -g paul-framework && paul init
```

Then in this project directory:

```
/paul:init
```

PAUL will read `PROJECT.md` and `ROADMAP.md` to scaffold the phase/milestone structure.

### 3. graphify (after Phase 01, when code exists)

```bash
# Install graphify skill globally
mkdir -p ~/.claude/skills/
cp -r path/to/graphify ~/.claude/skills/
```

Run after each major phase:

```
/graphify .
```

Commit `graphify-out/` to the repo. Used for codebase navigation in later phases.

---

## Command flow — first phases

### Initialize project

```
/paul:init
```

Reads `PROJECT.md` + `ROADMAP.md`. Creates `STATE.md` and `phases/` directory structure.

### Start Phase 01

```
/paul:plan 01-foundation
```

Produces `phases/01-foundation/PLAN.md`. Review it; if it looks right:

```
/paul:apply
```

Executes tasks. Mark each task done as you go. When phase is complete:

```
/paul:unify
```

Writes outcomes to `STATE.md`. Closes the phase.

### Phase 02 — spike first

Phase 02 has a spike. Before running the main plan:

```
/paul:plan 02-datafetcher --quick-fix
```

This produces `phases/02-datafetcher/02-99-PLAN.md`. Execute the spike (test HTTP in plugin hosts). Write findings to `phases/02-datafetcher/02-99-SUMMARY.md`. Then:

```
/paul:unify 02-99
```

Now run the main Phase 02 plan:

```
/paul:plan 02-datafetcher
```

PAUL's coherence check will read STATE.md (which now includes the spike SUMMARY) and produce an informed plan.

### Subsequent phases

Repeat the same pattern: `/paul:plan`, `/paul:apply`, `/paul:unify`. Phases without spikes skip the `--quick-fix` step.

### Check progress at any time

```
/paul:progress
/paul:status
```

---

## Spike protocol reminder

A spike is a short technical investigation, not a product deliverable.

- Spike PLAN uses `--quick-fix` flag → short plan, no milestone.
- Spike SUMMARY answers exactly one question (listed in ROADMAP.md under each spike).
- Spike result lives in `phases/0X-name/0X-99-SUMMARY.md`.
- `/paul:unify 0X-99` writes the decision into STATE.md.
- The main phase plan consumes that decision via the coherence check.

**Never start the main plan of a spiked phase without completing the spike.** The spike exists because the main plan cannot be written without it.

---

## Risks summary

Track these in `STATE.md` throughout development:

| Risk | Resolves in | Mitigation |
|------|-------------|------------|
| HTTP blocked in Logic Pro / Pro Tools | Phase 02 spike | Last-known-state fallback + documented limitation |
| NOAA API field names change | Ongoing | Narrow parser, fixture tests, version-pin fixtures |
| Additive engine CPU overhead | Phase 04 profiling | Lookup-table oscillators, partial cap |
| VisualRenderer blocks audio thread | Phase 05 | Lock-free shared value pattern |
| macOS code signing for AU | Phase 07 | Document unsigned dev path; sign for release |
| Null NOAA data gaps | DataFetcher | Last-valid cache, data_age_s display |
| SynthParamMapper v2 contract | Phase 03 review | "Pretend v2.1" exercise in Phase 03 SUMMARY |

---

## What's deliberately out of scope (v1)

- Historical playback (v2.2).
- User-editable mapping curves (v2.3).
- Multi-channel / spatial audio.
- MIDI or OSC output (v2 ecosystem).
- Mobile or web build.
- Visual graphs of raw space weather data — the VisualRenderer is artistic, not a data dashboard.
- Companion app for sandboxed plugin hosts — plugin warns and continues with last-known state.

These are not oversights. They are registered ideas (see `ROADMAP.md` v2 and future ecosystem sections). v1 ships without them.
