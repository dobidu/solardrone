#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

class BeatRepeater {
public:
    enum class LoopLength { Bar16th, Bar8th, Bar4th, Bar2nd, Bar1, Bar2 };

    void prepare(double sampleRate, int maxBlockSize);
    void setEnabled(bool e);
    void setLoopLength(LoopLength l);
    void setFeedback(float f);
    void setWet(float w);
    void setBeatPeriodSamples(int samples);
    void setDensity(float d);

    void process(juce::AudioBuffer<float>& buffer);

private:
    int loopLengthSamples() const;

    double     sampleRate  = 44100.0;
    bool       enabled     = false;
    float      feedback    = 0.5f;
    float      wet         = 0.7f;
    int        beatPeriod  = 22050;
    LoopLength loopLen        = LoopLength::Bar4th;
    LoopLength pendingLoopLen = LoopLength::Bar4th;  // deferred to boundary

    std::vector<float> ringL, ringR;
    int   writePos           = 0;
    int   bufferSize         = 0;
    float currentBeatPeriodF = 22050.0f;
    float density        = 1.0f;
    bool  cycleActive    = true;
    int   samplesIntoLoop = 0;
};
