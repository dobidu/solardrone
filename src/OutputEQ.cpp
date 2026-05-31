#include "OutputEQ.h"

void OutputEQ::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = spec.sampleRate;
    lowShelfL.reset();  lowShelfR.reset();
    midPeakL.reset();   midPeakR.reset();
    highShelfL.reset(); highShelfR.reset();
    setLowShelf(0.f);
    setMidPeak(0.f);
    setHighShelf(0.f);
}

void OutputEQ::setLowShelf(float db) {
    const auto c = Coefficients::makeLowShelf(
        (float)sampleRate, 200.f, 0.707f,
        juce::Decibels::decibelsToGain(db));
    *lowShelfL.coefficients = *c;
    *lowShelfR.coefficients = *c;
}

void OutputEQ::setMidPeak(float db) {
    const auto c = Coefficients::makePeakFilter(
        (float)sampleRate, 1000.f, 1.0f,
        juce::Decibels::decibelsToGain(db));
    *midPeakL.coefficients = *c;
    *midPeakR.coefficients = *c;
}

void OutputEQ::setHighShelf(float db) {
    const auto c = Coefficients::makeHighShelf(
        (float)sampleRate, 5000.f, 0.707f,
        juce::Decibels::decibelsToGain(db));
    *highShelfL.coefficients = *c;
    *highShelfR.coefficients = *c;
}

void OutputEQ::process(juce::AudioBuffer<float>& buffer) {
    const int n = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();
    if (ch < 1) return;

    auto* L = buffer.getWritePointer(0);
    auto* R = (ch > 1) ? buffer.getWritePointer(1) : L;

    for (int i = 0; i < n; ++i) {
        L[i] = lowShelfL.processSample(midPeakL.processSample(highShelfL.processSample(L[i])));
        R[i] = lowShelfR.processSample(midPeakR.processSample(highShelfR.processSample(R[i])));
    }
}
