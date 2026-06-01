#include "ModalBank.h"
#include <cmath>
#include <algorithm>

void ModalBank::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = (float)spec.sampleRate;
    dryBuf.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    for (int k = 0; k < kMaxModes; ++k) {
        modesL[k].reset();
        modesR[k].reset();
    }
    updateCoefficients();
}

void ModalBank::reset() {
    for (int k = 0; k < kMaxModes; ++k) { modesL[k].reset(); modesR[k].reset(); }
}

void ModalBank::setFundamental(float hz)    { fundamental   = hz;  updateCoefficients(); }
void ModalBank::setModeCount(int count)     { modeCount     = std::max(4, std::min(kMaxModes, count)); }
void ModalBank::setDecaySeconds(float secs) { decaySecs     = secs; updateCoefficients(); }
void ModalBank::setInharmonicity(float v)   { inharmonicity = v;    updateCoefficients(); }
void ModalBank::setWet(float w)             { wet           = w; }

void ModalBank::updateCoefficients() {
    for (int k = 0; k < kMaxModes; ++k) {
        const float harm  = (float)(k + 1);
        // Stretched spectrum
        const float freq  = fundamental * std::pow(harm, 1.0f + inharmonicity * 0.3f);
        const float fcl   = std::min(freq, sampleRate * 0.45f);
        // Q from decay: Q ≈ π × f × τ
        const float q     = std::max(0.5f, std::min(500.f,
                                juce::MathConstants<float>::pi * fcl * decaySecs));
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeBandPass(
                          (double)sampleRate, (double)fcl, (double)q);
        *modesL[k].coefficients = *coeffs;
        *modesR[k].coefficients = *coeffs;
    }
}

void ModalBank::process(juce::AudioBuffer<float>& buffer) {
    if (wet < 0.001f) { buffer.clear(); return; }

    const int n  = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();

    // Save dry
    for (int c = 0; c < ch; ++c)
        dryBuf.copyFrom(c, 0, buffer, c, 0, n);

    buffer.clear();

    for (int k = 0; k < modeCount; ++k) {
        const float modeAmp = 1.0f / (float)(k + 1);  // 1/f taper
        for (int s = 0; s < n; ++s) {
            const float inL = dryBuf.getSample(0, s);
            buffer.addSample(0, s, modesL[k].processSample(inL) * modeAmp);
            if (ch > 1) {
                const float inR = dryBuf.getSample(1, s);
                buffer.addSample(1, s, modesR[k].processSample(inR) * modeAmp);
            }
        }
    }

    // Scale by wet
    buffer.applyGain(wet);
}
