#include "SympatheticStrings.h"
#include <cmath>
#include <algorithm>

constexpr float SympatheticStrings::kRatios[SympatheticStrings::kMaxStrings];

void SympatheticStrings::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = (float)spec.sampleRate;
    // Max delay: fundamental can go as low as 20Hz → sr/20 = 2205 samples
    const int maxDelay = (int)(sampleRate / 20.f) + 16;
    for (int k = 0; k < kMaxStrings; ++k)
        strings[k].prepare(maxDelay);
    dryBuf.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
}

void SympatheticStrings::reset() {
    for (int k = 0; k < kMaxStrings; ++k) strings[k].reset();
}

void SympatheticStrings::setFundamental(float hz) { fundamental = std::max(20.f, hz); }
void SympatheticStrings::setStringCount(int n)    { stringCount = std::max(4, std::min(kMaxStrings, n)); }
void SympatheticStrings::setDamping(float d)      { damping    = std::max(0.f, std::min(1.f, d)); }
void SympatheticStrings::setNoiseLevel(float n)   { noiseLevel = std::max(0.f, std::min(1.f, n)); }
void SympatheticStrings::setWet(float w)          { wet        = w; }

void SympatheticStrings::process(juce::AudioBuffer<float>& buffer) {
    if (wet < 0.001f) { buffer.clear(); return; }

    const int n  = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();

    for (int c = 0; c < ch; ++c)
        dryBuf.copyFrom(c, 0, buffer, c, 0, n);

    buffer.clear();

    for (int s = 0; s < n; ++s) {
        const float inL = dryBuf.getSample(0, s);
        const float inR = (ch > 1) ? dryBuf.getSample(1, s) : inL;
        const float noise = noiseLevel > 0.01f ? (rng.nextFloat() - 0.5f) * noiseLevel * 0.04f : 0.f;

        float outL = 0, outR = 0;

        for (int k = 0; k < stringCount; ++k) {
            const float freq = fundamental * kRatios[k];
            const int   dl   = std::max(1, (int)(sampleRate / std::max(1.f, freq)));
            const float fb   = 0.995f - damping * 0.15f;
            // Loop filter: single-pole lowpass cutoff = sr/4 * (1-damping*0.7)
            const float loopCutoff = std::max(0.01f, 1.f - damping * 0.7f) * 0.25f;

            // Read from delay
            int rposL = (strings[k].writePos - dl + (int)strings[k].dataL.size()) % (int)strings[k].dataL.size();
            int rposR = (strings[k].writePos - dl + (int)strings[k].dataR.size()) % (int)strings[k].dataR.size();
            float sL = strings[k].dataL[(size_t)rposL];
            float sR = strings[k].dataR[(size_t)rposR];

            // Loop lowpass
            strings[k].loopState += loopCutoff * (sL - strings[k].loopState);
            sL = strings[k].loopState;
            sR = sR * loopCutoff + sL * (1.f - loopCutoff);  // approximate for R

            // Write: input + feedback
            strings[k].dataL[(size_t)strings[k].writePos] = inL + noise + sL * fb;
            strings[k].dataR[(size_t)strings[k].writePos] = inR + noise + sR * fb;
            strings[k].writePos = (strings[k].writePos + 1) % (int)strings[k].dataL.size();

            outL += sL;
            outR += sR;
        }

        outL /= (float)stringCount;
        outR /= (float)stringCount;
        buffer.setSample(0, s, outL * wet);
        if (ch > 1) buffer.setSample(1, s, outR * wet);
    }
}
