#pragma once
#include <juce_core/juce_core.h>

struct SpaceWeatherState {
    enum class Source     { live, cached, defaultValue };
    enum class FlareClass { none, B, C, M, X };

    float velocity   = 450.0f;  // km/s
    float density    =   5.0f;  // p/cm³
    float bz_gsm     =   0.0f;  // nT
    float kp         =   0.0f;  // 0–9
    int   data_age_s =   0;
    Source source    = Source::defaultValue;
    juce::String timestamp;

    float      x_ray_flux  = 0.0f;              // W/m² GOES 0.1-0.8nm band
    FlareClass flare_class = FlareClass::none;

    // Resonator drivers
    float temperature        = 80000.0f;  // K, solar wind ion temperature
    float dst_index          =    0.0f;   // nT, Dst ring-current index (negative = storm)
    float proton_flux_10mev  =    0.3f;   // pfu, integral proton flux ≥10 MeV

    static FlareClass classifyFlux(float flux) {
        if (flux >= 1e-4f) return FlareClass::X;
        if (flux >= 1e-5f) return FlareClass::M;
        if (flux >= 1e-6f) return FlareClass::C;
        if (flux >= 1e-7f) return FlareClass::B;
        return FlareClass::none;
    }
};
