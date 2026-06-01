#include "Chopper.h"
#include <cmath>
#include <algorithm>

void Chopper::prepare(double sr) {
    sampleRate = sr;
    updatePhaseInc();
}

void Chopper::setEnabled(bool e)  { enabled = e; }
void Chopper::setDepth(float d)   { depth = std::max(0.0f, std::min(1.0f, d)); }
void Chopper::setShape(Shape s)   { shape = s; }

void Chopper::setRate(float hz) {
    rate = std::max(0.01f, hz);
    if (!bpmSync) updatePhaseInc();
}

void Chopper::setBPMSync(bool sync, float bpm, float divBeats) {
    bpmSync = sync;
    if (sync) {
        rate = (bpm / 60.0f) / std::max(0.001f, divBeats);
        updatePhaseInc();
    }
}

void Chopper::updatePhaseInc() {
    targetPhaseInc = rate / sampleRate;
}

float Chopper::lfoValue() const {
    const double p = std::fmod(phase + phaseOffset, 1.0);
    switch (shape) {
        case Shape::sine:
            return 0.5f + 0.5f * (float)std::sin(p * 6.283185307);
        case Shape::square:
            return p < 0.5 ? 1.0f : 0.0f;
        case Shape::saw:
            return 1.0f - (float)p;
    }
    return 1.0f;
}

void Chopper::process(juce::AudioBuffer<float>& buffer) {
    if (!enabled) return;
    phaseInc += 0.05 * (targetPhaseInc - phaseInc);
    const int n  = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();

    // Envelope smoothing time: fraction of half-period in samples
    const float halfPeriod = (float)std::max(1.0, 0.5 / std::max(1e-6, phaseInc));
    const float atkAlpha = (attack  < 0.001f) ? 1.f : std::min(1.f, 1.f / (attack  * halfPeriod + 1.f));
    const float relAlpha = (release < 0.001f) ? 1.f : std::min(1.f, 1.f / (release * halfPeriod + 1.f));

    for (int i = 0; i < n; ++i) {
        const float rawGate = lfoValue();
        const float alpha = rawGate > smoothedGate ? atkAlpha : relAlpha;
        smoothedGate += alpha * (rawGate - smoothedGate);

        const float gain = 1.0f - depth * (1.0f - smoothedGate);
        for (int c = 0; c < ch; ++c)
            buffer.getWritePointer(c)[i] *= gain;
        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
    }
}
