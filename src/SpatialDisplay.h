#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "ColourScheme.h"

class SpatialDisplay : public juce::Component {
public:
    explicit SpatialDisplay(juce::AudioProcessorValueTreeState& apvts);
    void setKp(float kp);
    void setFlareLevel(float level);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    struct Layer { const char* azId; const char* elId; juce::Colour col; const char* label; };
    static const Layer kLayers[3];

    juce::Point<float> dotPos(int i) const;
    int  hitTest(juce::Point<float> p) const;
    void applyPos(int i, juce::Point<float> p);

    juce::AudioProcessorValueTreeState& apvts;
    float currentKp  = 0.f;
    float flareLevel = 0.f;
    int   dragLayer  = -1;
};
