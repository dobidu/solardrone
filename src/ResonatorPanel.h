#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "ColourScheme.h"
#include "SolarTerminal.h"
#include "SpaceWeatherState.h"

class ResonatorPanel : public juce::Component {
public:
    explicit ResonatorPanel(juce::AudioProcessorValueTreeState& apvts);
    void setKp(float kp);
    void setLiveData(float velocity, float temperature, float dst, float protonFlux);
    void updateTerminal(const SpaceWeatherState& sw, bool modal, bool fdn, bool str);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    static void setupSlider(juce::Slider& s, juce::Label& l,
                            const juce::String& name, juce::Component* p);

    // Modal Bank column
    juce::ToggleButton btnModal;
    juce::Slider       slModalWet, slModalDecay;
    juce::Label        lblModalWet, lblModalDecay;
    std::unique_ptr<ButtonAttachment> attModalOn;
    std::unique_ptr<SliderAttachment> attModalWet, attModalDecay;

    // FDN column
    juce::ToggleButton btnFDN;
    juce::Slider       slFDNWet, slFDNSize;
    juce::Label        lblFDNWet, lblFDNSize;
    std::unique_ptr<ButtonAttachment> attFDNOn;
    std::unique_ptr<SliderAttachment> attFDNWet, attFDNSize;

    // Sympathetic Strings column
    juce::ToggleButton btnStrings;
    juce::Slider       slStringsWet, slStringsN;
    juce::Label        lblStringsWet, lblStringsN;
    std::unique_ptr<ButtonAttachment> attStringsOn;
    std::unique_ptr<SliderAttachment> attStringsWet, attStringsN;

    // Solar terminal
    SolarTerminal terminal;

    // EQ section
    juce::Slider slEQLow, slEQMid, slEQHigh;
    juce::Label  lblEQLow, lblEQMid, lblEQHigh;
    std::unique_ptr<SliderAttachment> attEQLow, attEQMid, attEQHigh;

    juce::AudioProcessorValueTreeState& apvts;
    float currentKp  = 0.f;
    float liveVel    = 450.f, liveTemp = 80000.f;
    float liveDst    = 0.f,   liveProton = 0.3f;
};
