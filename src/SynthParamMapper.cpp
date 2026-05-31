#include "SynthParamMapper.h"
#include <cmath>

template<typename T>
static T clamp(T val, T lo, T hi) { return val < lo ? lo : val > hi ? hi : val; }

SynthParams SynthParamMapper::map(const SpaceWeatherState& state,
                                   const UserParams& params) {
    SynthParams out;

    // ── Layer 1: solar wind ───────────────────────────────────────────────

    // velocity (300-800 km/s) → l1_fundamental_hz (user-editable range, log scale)
    const float v   = clamp(state.velocity, 300.0f, 800.0f);
    const float vLo = std::max(params.map_vel_lo_hz, 1.0f);
    const float vHi = std::max(params.map_vel_hi_hz, vLo + 1.0f);
    out.l1_fundamental_hz = vLo * std::pow(vHi / vLo, (v - 300.0f) / 500.0f);

    // density (1-50 p/cm³) → l1_harmonic_count (4-24 partials)
    // Minimum 4 partials ensures richer tone even on quiet solar days
    const float d = clamp(state.density, 1.0f, 50.0f);
    out.l1_harmonic_count = 4 + (int)std::round(20.0f * (d - 1.0f) / 49.0f);

    // Bz → l1_timbre: 0=open, 1=tense (threshold user-editable)
    {
        const float bz = state.bz_gsm;
        const float t  = std::min(params.map_bz_thresh, -0.1f);  // e.g. -10
        if (bz >= 0.0f) {
            out.l1_timbre = 0.0f;
        } else if (bz > t) {
            out.l1_timbre = (bz / t) * 0.3f;   // 0 at bz=0, 0.3 at bz=t
        } else {
            const float x = clamp((t - bz) / 7.0f, 0.0f, 1.0f);
            out.l1_timbre = 0.3f + 0.7f * x;
        }
    }

    // velocity → l1_amplitude (linear 0.2-1.0)
    out.l1_amplitude = 0.2f + 0.8f * clamp((v - 300.0f) / 500.0f, 0.0f, 1.0f);

    // ── Layer 2: Kp ───────────────────────────────────────────────────────

    const float kp = clamp(state.kp, 0.0f, 9.0f);

    // Kp → l2_amplitude: floor 0.1 ensures L2 always audible; peaks at 1.0 at Kp=9
    out.l2_amplitude = (0.1f + 0.9f * (kp / 9.0f)) * params.dynamics_range;

    // Kp → l2_harmonic_density: quadratic from user threshold
    out.l2_harmonic_density = (kp / 9.0f) * (kp / 9.0f);

    // Kp → l2_brightness: activates at user-editable threshold
    {
        const float kds   = clamp(params.map_kp_dens_start, 0.0f, 8.0f);
        const float range = std::max(9.0f - kds, 0.1f);
        if (kp < kds) {
            out.l2_brightness = 0.0f;
        } else {
            out.l2_brightness = clamp((kp - kds) / range, 0.0f, 1.0f);
        }
    }

    // ── l2_fundamental_hz from l1 + user interval ─────────────────────────

    out.l2_fundamental_hz = out.l1_fundamental_hz * computeIntervalRatio(params);

    // ── Global ────────────────────────────────────────────────────────────

    out.stereo_width = params.stereo_spread;

    return out;
}

float SynthParamMapper::computeIntervalRatio(const UserParams& params) {
    if (params.harmony_mode == UserParams::HarmonyMode::just) {
        static const float justRatios[13] = {
            1.0f,         // 0  unison
            16.0f/15.0f,  // 1  minor 2nd
            9.0f/8.0f,    // 2  major 2nd
            6.0f/5.0f,    // 3  minor 3rd
            5.0f/4.0f,    // 4  major 3rd
            4.0f/3.0f,    // 5  perfect 4th
            45.0f/32.0f,  // 6  tritone
            3.0f/2.0f,    // 7  perfect 5th (default)
            8.0f/5.0f,    // 8  minor 6th
            5.0f/3.0f,    // 9  major 6th
            16.0f/9.0f,   // 10 minor 7th
            15.0f/8.0f,   // 11 major 7th
            2.0f          // 12 octave
        };
        const int idx = clamp(params.l1_l2_interval_semitones, 0, 12);
        return justRatios[idx];
    }
    return std::pow(2.0f, params.l1_l2_interval_semitones / 12.0f);
}
