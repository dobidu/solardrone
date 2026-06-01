#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <array>

class FeedbackDelayNetwork {
public:
    static constexpr int N = 8;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void setT60(float seconds);
    void setSize(float v);           // 0=metallic short, 1=cavernous long
    void setAbsorptionCutoff(float hz);
    void setWet(float w);
    void process(juce::AudioBuffer<float>& buffer);  // wet-only output
    void reset();

private:
    // Base delay lengths (coprime, at 44100 Hz — scaled in prepare)
    static constexpr int kBaseDelays[N] = {743,997,1327,1657,2039,2503,3079,3761};
    // Hadamard 8×8 (signs only)
    static constexpr int kH[N][N] = {
        { 1, 1, 1, 1, 1, 1, 1, 1},
        { 1,-1, 1,-1, 1,-1, 1,-1},
        { 1, 1,-1,-1, 1, 1,-1,-1},
        { 1,-1,-1, 1, 1,-1,-1, 1},
        { 1, 1, 1, 1,-1,-1,-1,-1},
        { 1,-1, 1,-1,-1, 1,-1, 1},
        { 1, 1,-1,-1,-1,-1, 1, 1},
        { 1,-1,-1, 1,-1, 1, 1,-1}
    };

    struct CircularBuffer {
        std::vector<float> data;
        int writePos = 0;
        void prepare(int sz) { data.assign(sz, 0.f); writePos = 0; }
        void write(float v)  { data[(size_t)writePos] = v; writePos = (writePos + 1) % (int)data.size(); }
        float read(int delay) const {
            int rp = (writePos - delay - 1 + (int)data.size()) % (int)data.size();
            return data[(size_t)rp];
        }
        void reset() { std::fill(data.begin(), data.end(), 0.f); writePos = 0; }
    };

    CircularBuffer delaysL[N], delaysR[N];
    // Single-pole lowpass per delay line (absorption)
    float absStateL[N] = {}, absStateR[N] = {};
    float absCoeff = 0.f;  // 0=no absorption, closer to 1=more

    juce::AudioBuffer<float> dryBuf;

    float sampleRate    = 44100.f;
    float t60           = 2.f;
    float size          = 0.5f;
    float absorptionHz  = 8000.f;
    float wet           = 0.08f;
    float feedbackGain  = 0.9f;
    float normFactor    = 1.f / std::sqrt((float)N);

    void updateFeedback();
    void updateAbsorption();
    int  effectiveDelay(int i) const;
};
