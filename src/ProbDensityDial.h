#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "ColourScheme.h"

class ProbDensityDial : public juce::Component, public juce::Timer {
public:
    explicit ProbDensityDial(juce::AudioProcessorValueTreeState& apvts);
    ~ProbDensityDial() override;

    void setKp(float kp);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void timerCallback() override;

private:
    float getDensity() const;
    void  regenerateDots();

    juce::AudioProcessorValueTreeState& apvts;
    float currentKp         = 0.f;
    bool  dots[16]          = {};
    float dragStartY        = 0.f;
    float dragStartDensity  = 1.0f;
};
