#pragma once
#include <vector>
#include "SynthParams.h"

class OscillatorBank {
public:
    void prepare(double sampleRate, int maxPartials = 24);
    void applyParams(const SynthParams& p, int layer, float layerGain);
    void render(float* left, float* right, int numSamples);

private:
    struct Partial {
        double phase    = 0.0;
        double phaseInc = 0.0;
        float  ampLeft  = 0.0f;
        float  ampRight = 0.0f;
    };

    std::vector<Partial> partials;
    double sampleRate   = 44100.0;
    int    activeCount  = 0;
};
