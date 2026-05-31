#pragma once

struct UserParams {
    enum class HarmonyMode { just, equalTemperament };

    float dynamics_range           = 1.0f;   // 0-1: scales l2_amplitude excursion
    float layer_balance            = 0.5f;   // 0=full L1, 1=full L2 (applied by AdditiveEngine)
    float stereo_spread            = 0.5f;   // 0-1
    HarmonyMode harmony_mode       = HarmonyMode::just;
    int   l1_l2_interval_semitones = 7;       // default: perfect 5th
    float glide_time_secs          = 120.0f;  // 30-300s interpolation

    // Mapping thresholds (editable via UI)
    float map_vel_lo_hz    =  55.0f;   // pitch at min velocity (Hz)
    float map_vel_hi_hz    = 220.0f;   // pitch at max velocity (Hz)
    float map_bz_thresh    = -10.0f;   // Bz nT where tense timbre activates
    float map_kp_dens_start =  4.0f;   // Kp where density/brightness activates

    // Visual layer blend weights (0=off, 1=full)
    float visual_lissajous = 0.7f;
    float visual_particles = 0.7f;
    float visual_spectral  = 0.7f;
};
