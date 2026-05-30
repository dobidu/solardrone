#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

class TempoTracker {
public:
    void  prepare(double sampleRate);
    void  process(const juce::MidiBuffer& midi, int numSamples);
    void  setInternalBPM(float bpm);

    float getCurrentBPM()          const;
    int   getSamplesUntilNextBeat() const;
    bool  isUsingMidiClock()        const;

private:
    static constexpr int kTicksPerBeat    = 24;
    static constexpr int kAveragingWindow = 24;

    double sampleRate     = 44100.0;
    float  internalBPM    = 120.0f;

    // MIDI clock
    double currentSample      = 0.0;
    double lastTickSample     = -1.0;
    double lastMidiClockSample = -1.0;  // -1 = never received
    double tickIntervals[24]  = {};
    int    intervalIdx        = 0;
    int    validIntervals     = 0;

    float  estimatedBPM       = 120.0f;
    int    beatPeriodSamples  = 22050;
    int    samplesUntilBeat   = 22050;
};
