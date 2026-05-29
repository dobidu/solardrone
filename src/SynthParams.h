#pragma once

struct SynthParams {
    // Layer 1 — solar wind
    float l1_fundamental_hz   = 55.0f;
    int   l1_harmonic_count   = 2;
    float l1_timbre           = 0.0f;   // 0=open (Bz+), 1=tense (Bz-)
    float l1_amplitude        = 0.2f;

    // Layer 2 — Kp
    float l2_fundamental_hz   = 82.5f;  // perfect 5th above L1 default
    float l2_harmonic_density = 0.0f;   // 0-1, activates at Kp >= 6
    float l2_brightness       = 0.0f;   // 0-1, activates at Kp >= 6
    float l2_amplitude        = 0.0f;   // 0-1, linear from Kp

    // Global
    float stereo_width        = 0.5f;
};
