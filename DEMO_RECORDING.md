# SolarDrone — Demo Recording Guide

## Goal

Two 10-minute recordings that demonstrate the quiet/storm contrast:
- **Recording A**: quiet solar conditions (Kp ≤ 2)
- **Recording B**: active conditions (Kp ≥ 5, ideally ≥ 7)

The contrast should be immediately perceptible to a non-technical listener.

## Step 1: Check current space weather

**Options:**
- Watch SolarDrone's status overlay (top-right): `live  Kp X.X`
- NOAA real-time Kp: https://www.swpc.noaa.gov/products/planetary-k-index
- NOAA alerts: https://www.swpc.noaa.gov/products/alerts-watches-and-warnings

**Kp reference:**
| Kp | Conditions |
|----|------------|
| 0–2 | Quiet — sparse drone, open Lissajous, slow particles |
| 3–5 | Unsettled — moderate harmonic density |
| 6–7 | Active storm — dense L2, turbulent particles, fast phase drift |
| 8–9 | Severe storm — maximum density, orange particles |

Quiet conditions occur ~80% of days. Active conditions occur during geomagnetic storms (often following solar flares or CMEs). Subscribe to NOAA alerts to catch events.

## Step 2: Recording setup

1. Open SolarDrone Standalone
2. Connect to an audio interface or use system audio
3. Open Audacity (or your DAW of choice)
4. Wait for status overlay to show **"live"** (green, ~30 seconds)
5. Note current Kp value from overlay or NOAA

**Recommended settings:**
- Sample rate: 44100 Hz, stereo
- Default plugin parameters (no adjustments)
- Recording duration: 10 minutes minimum

## Step 3: Capture Recording A (quiet)

Choose a day with Kp ≤ 2. Start recording. Let SolarDrone run at defaults for 10 minutes. Save as `recording_a_quiet_kp[N]_[date].wav`.

## Step 4: Capture Recording B (active)

Watch NOAA alerts for a Kp ≥ 5 event. Repeat with the same settings. Save as `recording_b_active_kp[N]_[date].wav`.

## Step 5: Create comparison clip

Export 30-second segments (e.g., minutes 4–5 of each recording). Place side-by-side in a timeline. Label clearly.

**What to listen for:**

| Feature | Quiet (Kp ≤ 2) | Active (Kp ≥ 6) |
|---------|----------------|-----------------|
| Drone pitch | Moderate (~90–130 Hz) | Higher if faster solar wind |
| Harmonic richness | Sparse (L2 near silent) | Dense, buzzing |
| Timbre | Open, clean | Tense, slightly inharmonic |
| Visual | Stable Lissajous, slow particles | Fast-morphing Lissajous, orange turbulence |

## Notes

- L1 (solar wind) changes are gradual (velocity varies 300–800 km/s on timescales of hours).
- L2 (Kp) transitions are more dramatic — storms can jump from Kp=2 to Kp=8 within minutes of a CME impact.
- With 120s glide time, the drone reaches its new state about 4 minutes after data changes. For demo purposes, glide time can be reduced to 30s.
