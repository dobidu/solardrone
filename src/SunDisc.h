#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "ColourScheme.h"

class SunDisc : public juce::Component {
public:
    explicit SunDisc(juce::AudioProcessorValueTreeState& apvts);
    ~SunDisc() override = default;

    void setKp(float kp);

    void paint(juce::Graphics&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;

private:
    float getVolume()  const;
    bool  getDroneOn() const;

    juce::AudioProcessorValueTreeState& apvts;
    float currentKp       = 0.f;
    float dragStartY      = 0.f;
    float dragStartVolume = 0.7f;
};
