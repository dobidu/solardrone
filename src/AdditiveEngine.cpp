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

static std::pair<float,float> computeSpatialGains(float azDeg, float /*elDeg*/) {
    const float az = std::max(-90.f, std::min(90.f, azDeg));
    // constant-power panning: az=-90→(1,0), az=0→(0.707,0.707), az=90→(0,1)
    const float t  = (az / 90.0f + 1.0f) * juce::MathConstants<float>::halfPi / 2.0f;
    return {std::cos(t), std::sin(t)};
}

void AdditiveEngine::setFlareLevel(float level, float sensitivity) {
    l3Target = std::max(0.0f, std::min(1.0f, level * sensitivity));
}

void AdditiveEngine::updateControlRate() {
    auto s = interp.getSmoothed();
    { juce::ScopedLock sl(smoothedLock); smoothed = s; }
    const auto& sm = s;

    // Constant-power layer balance crossfade
    const float angle = userParams.layer_balance * juce::MathConstants<float>::halfPi;
    const float bl1   = std::cos(angle);
    const float bl2   = std::sin(angle);

    // Spatial ILD gains
    auto [l1L, l1R] = computeSpatialGains(userParams.l1_azimuth, userParams.l1_elevation);
    auto [l2L, l2R] = computeSpatialGains(userParams.l2_azimuth, userParams.l2_elevation);
    auto [l3L, l3R] = computeSpatialGains(userParams.l3_azimuth, userParams.l3_elevation);

    l1Bank.setSpatialGains(l1L * bl1, l1R * bl1);
    l2Bank.setSpatialGains(l2L * bl2, l2R * bl2);
    if (l3Envelope > 0.001f)
        l3Bank.setSpatialGains(l3L * l3Envelope, l3R * l3Envelope);

    l1Bank.applyParams(sm, 0, 1.0f);
    l2Bank.applyParams(sm, 1, 1.0f);

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
