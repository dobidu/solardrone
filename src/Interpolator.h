#pragma once
#include "SynthParams.h"

class Interpolator {
public:
    void prepare(double sampleRate, int controlRateHz = 100);
    void setTarget(const SynthParams& target);
    void setGlideTimeSecs(float seconds);
    SynthParams getSmoothed();
    void reset(const SynthParams& value);

private:
    struct SlewState { float current = 0.0f; float target = 0.0f; };

    SlewState l1_hz, l1_timbre, l1_amp;
    SlewState l2_hz, l2_density, l2_bright, l2_amp;
    SlewState stereo_w;
    int l1_harmonic_count_target = 2;

    float alpha         = 0.0f;
    float glideTimeSecs = 120.0f;
    int   controlRateHz = 100;

    void  updateAlpha();
    static float slew(SlewState& s, float a);
};
