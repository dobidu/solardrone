#include "AdditiveEngine.h"
#include <cmath>
#include <algorithm>

void AdditiveEngine::prepare(double sampleRate, int samplesPerBlock) {
    l1Bank.prepare(sampleRate, 24);
    l2Bank.prepare(sampleRate, 24);
    interp.prepare(sampleRate, 100);
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
        offset    += chunk;
        remaining -= chunk;
        controlCounter -= chunk;
    }
}

void AdditiveEngine::updateControlRate() {
    smoothed = interp.getSmoothed();

    // Constant-power layer balance crossfade
    const float angle = userParams.layer_balance
                        * juce::MathConstants<float>::halfPi;
    const float l1Gain = std::cos(angle);
    const float l2Gain = std::sin(angle);

    l1Bank.applyParams(smoothed, 0, l1Gain);
    l2Bank.applyParams(smoothed, 1, l2Gain);
}
