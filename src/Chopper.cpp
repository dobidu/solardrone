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
    switch (shape) {
        case Shape::sine:
            return 0.5f + 0.5f * (float)std::sin(phase * 6.283185307);
        case Shape::square:
            return phase < 0.5 ? 1.0f : 0.0f;
        case Shape::saw:
            return 1.0f - (float)phase;
    }
    return 1.0f;
}

void Chopper::process(juce::AudioBuffer<float>& buffer) {
    if (!enabled) return;
    // Smooth phaseInc to reduce clicks on rate/div changes
    phaseInc += 0.05 * (targetPhaseInc - phaseInc);
    const int n  = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();
    for (int i = 0; i < n; ++i) {
        const float gain = 1.0f - depth * (1.0f - lfoValue());
        for (int c = 0; c < ch; ++c)
            buffer.getWritePointer(c)[i] *= gain;
        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
    }
}
