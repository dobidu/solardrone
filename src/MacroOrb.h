#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "ColourScheme.h"

class MacroOrb : public juce::Component {
public:
    explicit MacroOrb(juce::AudioProcessorValueTreeState& apvts);
    void setKp(float kp);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;

private:
    struct Preset { float balance, dynamics, repWet, chopDepth; };
    static const Preset kCorners[4];  // TL, TR, BL, BR

    void applyBlend();
    void updateFromEvent(const juce::MouseEvent& e);

    juce::AudioProcessorValueTreeState& apvts;
    juce::Point<float> cursor{0.5f, 0.5f};
    float currentKp = 0.f;
};
