#pragma once
#include <juce_dsp/juce_dsp.h>

class ModalBank {
public:
    static constexpr int kMaxModes = 32;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void setFundamental(float hz);
    void setModeCount(int count);        // 4-32
    void setDecaySeconds(float secs);
    void setInharmonicity(float v);      // 0=harmonic, 1=stretched
    void setWet(float w);
    void process(juce::AudioBuffer<float>& buffer);  // wet-only output
    void reset();

private:
    void updateCoefficients();
    using Filter = juce::dsp::IIR::Filter<float>;
    Filter modesL[kMaxModes], modesR[kMaxModes];

    juce::AudioBuffer<float> dryBuf;  // pre-allocated, no audio-thread alloc

    float sampleRate    = 44100.f;
    float fundamental   = 55.f;
    int   modeCount     = 8;
    float decaySecs     = 2.f;
    float inharmonicity = 0.f;
    float wet           = 0.1f;
};
