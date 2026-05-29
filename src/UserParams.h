#pragma once

struct UserParams {
    enum class HarmonyMode { just, equalTemperament };

    float dynamics_range           = 1.0f;   // 0-1: scales l2_amplitude excursion
    float layer_balance            = 0.5f;   // 0=full L1, 1=full L2 (applied by AdditiveEngine)
    float stereo_spread            = 0.5f;   // 0-1
    HarmonyMode harmony_mode       = HarmonyMode::just;
    int   l1_l2_interval_semitones = 7;       // default: perfect 5th
    float glide_time_secs          = 120.0f;  // 30-300s interpolation
};
