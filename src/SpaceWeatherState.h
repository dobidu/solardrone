#pragma once
#include <juce_core/juce_core.h>

struct SpaceWeatherState {
    enum class Source { live, cached, defaultValue };

    float velocity   = 450.0f;  // km/s
    float density    =   5.0f;  // p/cm³
    float bz_gsm     =   0.0f;  // nT
    float kp         =   0.0f;  // 0–9
    int   data_age_s =   0;
    Source source    = Source::defaultValue;
    juce::String timestamp;
};
