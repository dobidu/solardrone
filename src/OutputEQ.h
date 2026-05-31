#pragma once
#include <juce_dsp/juce_dsp.h>

class OutputEQ {
public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void setLowShelf(float gainDb);
    void setMidPeak(float gainDb);
    void setHighShelf(float gainDb);
    void process(juce::AudioBuffer<float>& buffer);

private:
    using Filter       = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    Filter lowShelfL,  lowShelfR;
    Filter midPeakL,   midPeakR;
    Filter highShelfL, highShelfR;
    double sampleRate = 44100.0;
};
