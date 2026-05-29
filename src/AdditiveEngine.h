#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "SynthParams.h"
#include "UserParams.h"
#include "Interpolator.h"
#include "OscillatorBank.h"

class AdditiveEngine {
public:
    void prepare(double sampleRate, int samplesPerBlock);
    void setSynthParams(const SynthParams& target);
    void setUserParams(const UserParams& params);
    void processBlock(juce::AudioBuffer<float>& buffer);

    SynthParams getSmoothedParams() const;

private:
    void updateControlRate();

    OscillatorBank l1Bank, l2Bank;
    Interpolator   interp;
    UserParams     userParams;

    mutable juce::CriticalSection smoothedLock;
    SynthParams    smoothed;

    int controlCounter = 0;
    int controlPeriod  = 441;   // ~100Hz at 44.1kHz
    bool prepared      = false;
};
