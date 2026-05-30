#include "BeatRepeater.h"
#include <algorithm>

void BeatRepeater::prepare(double sr, int /*maxBlockSize*/) {
    sampleRate = sr;
    // Max 2 bars at 30 BPM: 8 beats * (sr * 60 / 30) samples/beat
    const int maxSamples = (int)(sr * 60.0 / 30.0 * 8.0) + 1024;
    ringL.assign((size_t)maxSamples, 0.0f);
    ringR.assign((size_t)maxSamples, 0.0f);
    bufferSize = maxSamples;
    writePos   = 0;
}

void BeatRepeater::setEnabled(bool e)              { enabled  = e; }
void BeatRepeater::setLoopLength(LoopLength l)     { loopLen  = l; }
void BeatRepeater::setFeedback(float f)            { feedback = std::max(0.0f, std::min(0.99f, f)); }
void BeatRepeater::setWet(float w)                 { wet      = std::max(0.0f, std::min(1.0f, w)); }
void BeatRepeater::setBeatPeriodSamples(int s)     { beatPeriod = std::max(1, s); }

int BeatRepeater::loopLengthSamples() const {
    switch (loopLen) {
        case LoopLength::Bar16th: return std::max(1, beatPeriod / 4);
        case LoopLength::Bar8th:  return std::max(1, beatPeriod / 2);
        case LoopLength::Bar4th:  return beatPeriod;
        case LoopLength::Bar2nd:  return beatPeriod * 2;
        case LoopLength::Bar1:    return beatPeriod * 4;
        case LoopLength::Bar2:    return beatPeriod * 8;
    }
    return beatPeriod;
}

void BeatRepeater::process(juce::AudioBuffer<float>& buffer) {
    if (bufferSize == 0) return;

    const int n    = buffer.getNumSamples();
    const int nCh  = buffer.getNumChannels();
    const auto* dryL = buffer.getReadPointer(0);
    const auto* dryR = nCh > 1 ? buffer.getReadPointer(1) : dryL;
    auto* outL = buffer.getWritePointer(0);
    auto* outR = nCh > 1 ? buffer.getWritePointer(1) : outL;

    const int loopLen = std::min(loopLengthSamples(), bufferSize - 1);

    for (int i = 0; i < n; ++i) {
        const int readPos = (writePos - loopLen + bufferSize) % bufferSize;
        const float loopedL = ringL[(size_t)readPos];
        const float loopedR = ringR[(size_t)readPos];

        // Always write into ring buffer (so enabling from silence has content)
        ringL[(size_t)writePos] = dryL[i] + loopedL * feedback;
        ringR[(size_t)writePos] = dryR[i] + loopedR * feedback;

        if (enabled) {
            outL[i] = dryL[i] * (1.0f - wet) + loopedL * wet;
            outR[i] = dryR[i] * (1.0f - wet) + loopedR * wet;
        }

        writePos = (writePos + 1) % bufferSize;
    }
}
