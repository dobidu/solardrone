#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <array>

class SympatheticStrings {
public:
    static constexpr int kMaxStrings = 12;
    // Scale degrees: unison, octave, P5, P4, M3, m3, M6, m7, M2, m2, M7, P5+oct
    static constexpr float kRatios[kMaxStrings] = {
        1.0f, 2.0f, 1.5f, 1.333f, 1.25f, 1.2f,
        1.667f, 1.778f, 1.125f, 1.059f, 1.875f, 3.0f
    };

    void prepare(const juce::dsp::ProcessSpec& spec);
    void setFundamental(float hz);
    void setStringCount(int count);     // 4-12
    void setDamping(float d);           // 0=long sustain, 1=very damped
    void setNoiseLevel(float n);        // 0-1, SEP event injection
    void setWet(float w);
    void process(juce::AudioBuffer<float>& buffer);  // wet-only output
    void reset();

private:
    struct CombString {
        std::vector<float> dataL, dataR;
        int writePos = 0;
        float loopState = 0.f;   // 1-pole lowpass state

        void prepare(int maxDelay) {
            dataL.assign((size_t)maxDelay, 0.f);
            dataR.assign((size_t)maxDelay, 0.f);
            writePos = 0; loopState = 0.f;
        }
        void reset() {
            std::fill(dataL.begin(), dataL.end(), 0.f);
            std::fill(dataR.begin(), dataR.end(), 0.f);
            writePos = 0; loopState = 0.f;
        }
    };

    CombString strings[kMaxStrings];
    juce::AudioBuffer<float> dryBuf;
    juce::Random rng;

    float sampleRate  = 44100.f;
    float fundamental = 55.f;
    int   stringCount = 6;
    float damping     = 0.f;
    float noiseLevel  = 0.f;
    float wet         = 0.1f;
};
