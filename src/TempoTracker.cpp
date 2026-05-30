#include "TempoTracker.h"
#include <algorithm>
#include <cmath>

void TempoTracker::prepare(double sr) {
    sampleRate = sr;
    beatPeriodSamples = (int)(sr * 60.0 / internalBPM);
    samplesUntilBeat  = beatPeriodSamples;
    currentSample     = 0.0;
    lastTickSample    = -1.0;
    lastMidiClockSample = -1.0;
    validIntervals    = 0;
    intervalIdx       = 0;
}

void TempoTracker::setInternalBPM(float bpm) {
    internalBPM = std::max(30.0f, std::min(300.0f, bpm));
    if (!isUsingMidiClock()) {
        beatPeriodSamples = (int)(sampleRate * 60.0 / internalBPM);
    }
}

void TempoTracker::process(const juce::MidiBuffer& midi, int numSamples) {
    for (const auto meta : midi) {
        const auto msg = meta.getMessage();
        if (msg.getRawDataSize() == 1 && msg.getRawData()[0] == 0xF8) {
            const double tickSample = currentSample + meta.samplePosition;

            if (lastTickSample >= 0.0) {
                const double interval = tickSample - lastTickSample;
                if (interval > 0.0 && interval < sampleRate * 2.0) {
                    tickIntervals[intervalIdx % kAveragingWindow] = interval;
                    ++intervalIdx;
                    validIntervals = std::min(validIntervals + 1, kAveragingWindow);
                }

                if (validIntervals >= 8) {
                    double sum = 0.0;
                    const int count = std::min(validIntervals, kAveragingWindow);
                    for (int i = 0; i < count; ++i) sum += tickIntervals[i];
                    const double avgInterval = sum / count;
                    const double bpm = 60.0 * sampleRate / (avgInterval * kTicksPerBeat);
                    estimatedBPM = (float)std::max(30.0, std::min(300.0, bpm));
                    beatPeriodSamples = (int)(sampleRate * 60.0 / estimatedBPM);
                }
            }
            lastTickSample      = tickSample;
            lastMidiClockSample = tickSample;
        }
    }

    // Beat phase counter
    samplesUntilBeat -= numSamples;
    while (samplesUntilBeat <= 0)
        samplesUntilBeat += beatPeriodSamples;

    currentSample += numSamples;
}

float TempoTracker::getCurrentBPM() const {
    return isUsingMidiClock() ? estimatedBPM : internalBPM;
}

int TempoTracker::getSamplesUntilNextBeat() const {
    return std::max(0, samplesUntilBeat);
}

bool TempoTracker::isUsingMidiClock() const {
    if (lastMidiClockSample < 0.0) return false;
    return (currentSample - lastMidiClockSample) < sampleRate * 0.5;
}
