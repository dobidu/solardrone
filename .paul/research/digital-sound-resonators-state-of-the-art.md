# Research: Digital Sound Resonators — State of the Art

**Topic:** Digital sound resonators state-of-the-art (for SolarDrone additive drone enrichment)
**Date:** 2026-05-31
**Agent:** general-purpose (training knowledge)

---

## Resonator Types Evaluated

### 1. Modal Resonator Bank ⭐ PRIMARY RECOMMENDATION
**DSP:** N parallel second-order biquad filters (StateVariableTPTFilter), each {f_k, A_k, τ_k}.
Uses Impulse-Invariant Transform (not bilinear) for frequency accuracy.
Each mode = exponentially decaying sinusoid.

**Timbral character:** resonant spectrum of a physical material — bell partials, wood body resonances, metal plate harmonics. With sustained drone input, accumulates energy at each mode, creating slowly building shimmering spectral profile. Highly controllable by choosing modal frequencies.

**Drone pairing: outstanding.** Modes at drone harmonics build up; others stay quiet. Drone gains a "body" — resonant signature of a chosen material. Can simulate sitar body, bell tower, stone wall.

**JUCE cost: low.** `juce::dsp::StateVariableTPTFilter` in bandpass mode. 32 modes = trivial CPU. Coefficient updates at control rate only.

**Products using this:** Ableton Resonators, IRCAM Modalys, Mutable Instruments Rings (Rings uses 4+16+24+48 modes depending on model)

---

### 2. Feedback Delay Networks (FDN 8-tap) ⭐ SECONDARY RECOMMENDATION
**DSP:** N delay lines with N×N feedback matrix (Hadamard = energy-preserving). Each delay has absorption filter (lowpass/shelving) to control T60 per frequency band. N=8 with coprime delay lengths.

**Timbral character:** spatial body. Short delays (200–500 samples) = metallic shimmer. Long coprime delays (1000–8000 samples) = cavernous, diffuse space. Metallic artifacts with short delays can be intentional (space weather = metallic turbulence).

**Drone pairing: excellent.** Adds the "room" the drone inhabits. Decay time directly = Dst index driver.

**JUCE cost: medium.** 8-tap FDN = 64 multiply-adds per sample (trivial). Reference: jatinchowdhury18/Feedback-Delay-Networks (MIT, JUCE-native).

**Products using this:** Valhalla DSP (entire plugin line), Eventide, Space Designer, most high-quality reverbs.

---

### 3. Sympathetic String Bank ⭐ TERTIARY RECOMMENDATION
**DSP:** 6–12 comb filters with feedback near 0.995, tuned to scale degrees of current drone. Each is a `juce::dsp::DelayLine` with a single-pole lowpass in the feedback loop (= Karplus-Strong body model).

**Timbral character:** harmonic halos. Strings near input harmonics "wake up" and sustain. Creates rich slowly-evolving harmonic resonance — the "halo" around a drone. Like piano strings ringing in sympathy, or sitar chikari strings.

**Drone pairing: ideal.** Directly responds to pitch content. Adds overtone halos that slowly fade and bloom. Can be tuned to musical scale.

**JUCE cost: low.** 12 DelayLines with feedback. ~80 lines of C++. Real-time tunable.

**Products using this:** Mutable Instruments Rings, Native Instruments Collision, Superchord.

---

### 4. Waveguide Networks (Karplus-Strong Extended)
**Timbral character:** instrument-like decay, natural spectral darkening over time.
**Drone pairing:** good but modal bank is more controllable for the same cost.
**JUCE cost:** low. Mostly subsumed by sympathetic strings approach.

---

### 5. FDN Metallic (Short delays)
Can be used as a separate "metallic character" layer when solar wind pressure is high.
Distinct from the spatial FDN — short delays 50–200 samples create pitched resonant peaks.

---

### 6. Spectral/FFT Resonator
**DSP:** FFT → resonance mask → IFFT. Ableton Spectral Resonator approach.
**Drone pairing:** good but introduces 46–93ms latency. Good for ambient but not tight DSP.
**JUCE cost:** medium. `juce::dsp::FFT` available.
**Verdict:** skip for v1 resonators — overhead not worth it vs modal bank.

---

### 7. Convolution Resonator
**DSP:** `juce::dsp::Convolution` with physical object IRs.
**Verdict:** static (can't be driven by solar params), skip for now.

---

## Additional NOAA Solar Parameters

### Already in plasma-1-day.json (zero cost — 4th column unused)
**Solar Wind Temperature**
- Range: ~20,000 K (quiet) → 1,000,000+ K (CME sheath)
- Cadence: 1-minute
- Mapping: log10(T/20000) → EQ tilt / LPF cutoff. Hot = bright, open timbre. Cold = dark, muffled.

### New endpoints

**Dst Index** (kyoto-dst.json)
- Physical: ring current in Earth's magnetosphere. Quiet: 0 to +20 nT. Storm: -50 to -250 nT. Extreme: < -250 nT.
- Cadence: hourly
- Mapping: outstanding macro-drama parameter. Normalize [0, -250 nT] → FDN T60 multiplier [0.5×, 8×]. Deep storm = cavernous long tails. Quiet = dry, short.

**Proton Flux ≥10 MeV** (goes/primary/integral-protons-1-day.json)
- Physical: high-energy protons from solar flares/CME shocks. Quiet: ~0.3 pfu. SEP event threshold: ≥10 pfu. X-class: can reach 10,000+ pfu.
- Cadence: 1-minute
- Mapping: log10(flux) - log10(0.3) → [0,4]. Drive: sympathetic string damping reduction + bandpass noise injection. SEP event = turbulent texture overlay.

**F10.7 Solar Radio Flux** (f107_cm_flux.json)
- Physical: proxy for solar UV/EUV, correlates with solar cycle. Range: 65–300+ sfu.
- Cadence: daily (noon measurement)
- Mapping: slow 27-day rotation + 11-year cycle parameter. Drive: additive partial count or master tuning offset.

**Solar Wind Dynamic Pressure** (derivable: P = 1.67e-6 × density × velocity²)
- No new endpoint — computable from existing data
- Range: 1–5 nPa quiet, 10–50 nPa CIR/CME, >50 nPa extreme
- Mapping: resonator attack sharpness / FDN absorption cutoff. High pressure = percussive excitation.

---

## Solar → Resonator Interaction Matrix

| Solar Parameter | Resonator Target | Mapping |
|----------------|-----------------|---------|
| Wind velocity (existing) | Modal bank decay τ | Fast wind → shorter, brighter modes |
| **Wind temperature (new)** | LPF tilt EQ on input | Hot → open; cold → dark |
| **Dynamic pressure (derived)** | FDN absorption LPF | High pressure → dampened |
| **Dst index (new, hourly)** | FDN T60 | Deep storm → long cavernous tail |
| **Proton flux ≥10MeV (new)** | Sympathetic damping + noise | SEP event → turbulent texture |
| Kp (existing) | Modal mode count | Storm → dense inharmonic modes |

---

## Implementation Priority

1. **Temperature column parsing** — zero cost, already in plasma feed, just unused
2. **Dst fetch** + map to `fdn_t60_multiplier` float (hourly, low bandwidth)
3. **32-mode modal bank** — highest sonic value, lowest cost
4. **Proton flux fetch** + noise injection trigger
5. **8-tap FDN** (Hadamard) driven by Dst/pressure
6. **Sympathetic string bank** (12 comb filters) tuned to drone root

---

## References
- [jatinchowdhury18/Feedback-Delay-Networks](https://github.com/jatinchowdhury18/Feedback-Delay-Networks)
- [Mutable Instruments Rings documentation](https://pichenettes.github.io/mutable-instruments-documentation/modules/rings/)
- [JUCE dsp::StateVariableTPTFilter](https://docs.juce.com/master/classdsp_1_1StateVariableTPTFilter.html)
- [NOAA SWPC Data Access](https://www.swpc.noaa.gov/content/data-access)
- [FDN Optimization — arxiv 2402.11216](https://arxiv.org/html/2402.11216v2)
- [Exploring Modal Synthesis — Nathan Ho](https://nathan.ho.name/posts/exploring-modal-synthesis/)
