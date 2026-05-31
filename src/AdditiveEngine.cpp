#include "AdditiveEngine.h"
#include <cmath>
#include <algorithm>

void AdditiveEngine::prepare(double sampleRate, int samplesPerBlock) {
    l1Bank.prepare(sampleRate, 24);
    l2Bank.prepare(sampleRate, 24);
    l3Bank.prepare(sampleRate, 8);   // burst: 8 bright partials
    interp.prepare(sampleRate, 100);
    // Attack 2s, Decay 20s (per control-rate block at ~100Hz)
    l3AttackRate = 1.0f / (2.0f  * 100.0f);
    l3DecayRate  = 1.0f / (20.0f * 100.0f);
    controlPeriod  = std::max(1, (int)(sampleRate / 100.0));
    controlCounter = 0;
    prepared = true;
    juce::ignoreUnused(samplesPerBlock);
}

void AdditiveEngine::setSynthParams(const SynthParams& target) {
    interp.setTarget(target);
}

void AdditiveEngine::setUserParams(const UserParams& params) {
    userParams = params;
    interp.setGlideTimeSecs(params.glide_time_secs);
}

void AdditiveEngine::processBlock(juce::AudioBuffer<float>& buffer) {
    buffer.clear();
    if (!prepared || buffer.getNumChannels() < 2) return;

    auto* left  = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    int remaining = buffer.getNumSamples();
    int offset    = 0;

    while (remaining > 0) {
        if (controlCounter <= 0) {
            updateControlRate();
            controlCounter = controlPeriod;
        }
        const int chunk = std::min(remaining, controlCounter);
        l1Bank.render(left + offset, right + offset, chunk);
        l2Bank.render(left + offset, right + offset, chunk);
        if (l3Envelope > 0.001f)
            l3Bank.render(left + offset, right + offset, chunk);
        offset    += chunk;
        remaining -= chunk;
        controlCounter -= chunk;
    }
}

SynthParams AdditiveEngine::getSmoothedParams() const {
    juce::ScopedLock sl(smoothedLock);
    return smoothed;
}

void AdditiveEngine::setFlareLevel(float level, float sensitivity) {
    l3Target = std::max(0.0f, std::min(1.0f, level * sensitivity));
}

void AdditiveEngine::updateControlRate() {
    auto s = interp.getSmoothed();
    { juce::ScopedLock sl(smoothedLock); smoothed = s; }
    const auto& sm = s;

    // Constant-power layer balance crossfade
    const float angle  = userParams.layer_balance * juce::MathConstants<float>::halfPi;
    l1Bank.applyParams(sm, 0, std::cos(angle));
    l2Bank.applyParams(sm, 1, std::sin(angle));

    // L3 burst envelope
    if (l3Target > l3Envelope)
        l3Envelope = std::min(l3Target, l3Envelope + l3AttackRate);
    else
        l3Envelope = std::max(l3Target, l3Envelope - l3DecayRate);

    if (l3Envelope > 0.001f) {
        SynthParams l3 = sm;
        l3.l1_fundamental_hz = sm.l1_fundamental_hz * 1.5f;  // perfect 5th
        l3.l1_harmonic_count = 8;
        l3.l1_timbre         = 0.3f + sm.l1_timbre * 0.5f;
        l3.l1_amplitude      = l3Envelope;
        l3Bank.applyParams(l3, 0, 1.0f);
    }
}
