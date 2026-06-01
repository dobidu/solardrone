#include "FeedbackDelayNetwork.h"
#include <cmath>
#include <algorithm>
#include <numeric>

constexpr int FeedbackDelayNetwork::kBaseDelays[FeedbackDelayNetwork::N];
constexpr int FeedbackDelayNetwork::kH[FeedbackDelayNetwork::N][FeedbackDelayNetwork::N];

int FeedbackDelayNetwork::effectiveDelay(int i) const {
    const float scale = 0.3f + size * 1.4f;
    const float ratio = sampleRate / 44100.f;
    return std::max(1, (int)(kBaseDelays[i] * scale * ratio));
}

void FeedbackDelayNetwork::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = (float)spec.sampleRate;
    // Max delay: kBaseDelays[N-1] * 1.7 * sampleRate/44100
    const int maxDelay = (int)(kBaseDelays[N-1] * 1.8f * sampleRate / 44100.f) + 16;
    for (int i = 0; i < N; ++i) {
        delaysL[i].prepare(maxDelay);
        delaysR[i].prepare(maxDelay);
    }
    dryBuf.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    std::fill(absStateL, absStateL+N, 0.f);
    std::fill(absStateR, absStateR+N, 0.f);
    normFactor = 1.f / std::sqrt((float)N);
    updateFeedback();
    updateAbsorption();
}

void FeedbackDelayNetwork::reset() {
    for (int i = 0; i < N; ++i) { delaysL[i].reset(); delaysR[i].reset(); }
    std::fill(absStateL, absStateL+N, 0.f);
    std::fill(absStateR, absStateR+N, 0.f);
}

void FeedbackDelayNetwork::setT60(float s)          { t60 = std::max(0.05f, s); updateFeedback(); }
void FeedbackDelayNetwork::setSize(float v)          { size = std::max(0.f, std::min(1.f, v)); updateFeedback(); }
void FeedbackDelayNetwork::setAbsorptionCutoff(float hz){ absorptionHz = hz; updateAbsorption(); }
void FeedbackDelayNetwork::setWet(float w)           { wet = w; }

void FeedbackDelayNetwork::updateFeedback() {
    float sumDelay = 0;
    for (int i = 0; i < N; ++i)
        sumDelay += (float)effectiveDelay(i) / sampleRate;
    const float avgDelay = sumDelay / N;
    feedbackGain = std::min(0.998f, std::exp(-3.0f * avgDelay / std::max(0.05f, t60)));
}

void FeedbackDelayNetwork::updateAbsorption() {
    // Single-pole lowpass: y = a*x + (1-a)*y_prev; a = 1 - exp(-2π*fc/sr)
    absCoeff = 1.f - std::exp(-juce::MathConstants<float>::twoPi * absorptionHz / sampleRate);
    absCoeff = std::max(0.01f, std::min(1.f, absCoeff));
}

void FeedbackDelayNetwork::process(juce::AudioBuffer<float>& buffer) {
    if (wet < 0.001f) { buffer.clear(); return; }

    const int n  = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();

    // Save dry
    for (int c = 0; c < ch; ++c)
        dryBuf.copyFrom(c, 0, buffer, c, 0, n);

    buffer.clear();

    for (int s = 0; s < n; ++s) {
        const float inL = dryBuf.getSample(0, s);
        const float inR = (ch > 1) ? dryBuf.getSample(1, s) : inL;
        const float in  = (inL + inR) * 0.5f;

        // Read delayed + apply absorption lowpass
        float xL[N], xR[N];
        for (int i = 0; i < N; ++i) {
            const int dl = effectiveDelay(i);
            float rL = delaysL[i].read(dl);
            float rR = delaysR[i].read(dl);
            // Single-pole lowpass (absorption)
            absStateL[i] += absCoeff * (rL - absStateL[i]);
            absStateR[i] += absCoeff * (rR - absStateR[i]);
            xL[i] = absStateL[i];
            xR[i] = absStateR[i];
        }

        // Hadamard multiply
        float yL[N] = {}, yR[N] = {};
        for (int row = 0; row < N; ++row) {
            for (int col = 0; col < N; ++col) {
                yL[row] += kH[row][col] * xL[col];
                yR[row] += kH[row][col] * xR[col];
            }
            yL[row] *= normFactor;
            yR[row] *= normFactor;
        }

        // Write new values (input + feedback)
        for (int i = 0; i < N; ++i) {
            delaysL[i].write(in + yL[i] * feedbackGain);
            delaysR[i].write(in + yR[i] * feedbackGain);
        }

        // Stereo output: even→L, odd→R
        float outL = 0, outR = 0;
        for (int i = 0; i < N; i += 2) { outL += xL[i]; outR += xR[i+1]; }
        outL *= 0.25f; outR *= 0.25f;

        buffer.setSample(0, s, outL * wet);
        if (ch > 1) buffer.setSample(1, s, outR * wet);
    }
}
