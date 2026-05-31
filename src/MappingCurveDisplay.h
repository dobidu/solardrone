#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ColourScheme.h"
#include <functional>

class MappingCurveDisplay : public juce::Component {
public:
    struct LiveValues { float velocity = 450.f; float bz = 0.f; float kp = 0.f; };

    void setParams(float velLo, float velHi, float bzThresh, float kpDens);
    void setLive(const LiveValues& v);
    void setKp(float kp);
    void paint(juce::Graphics&) override;

private:
    void drawCurve(juce::Graphics&, juce::Rectangle<float> area,
                   const juce::String& label,
                   std::function<float(float)> fn,
                   float liveInput);

    float velLo = 55.f, velHi = 220.f;
    float bzThresh = -10.f, kpDens = 4.f;
    LiveValues live;
    float currentKp = 0.f;
};
