#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "VisualRenderer.h"

class SolarDroneAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit SolarDroneAudioProcessorEditor(SolarDroneAudioProcessor&);
    ~SolarDroneAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SolarDroneAudioProcessor& processorRef;
    VisualRenderer visualRenderer;

    // Controls
    juce::Slider      slVolume, slGlide, slDynamics, slBalance, slSpread;
    juce::Slider      slInterval, slVisLiss, slVisPart, slVisSpec;
    juce::ToggleButton btnDroneOn;
    juce::ComboBox    cmbHarmony;

    // Labels
    juce::Label lblVolume, lblGlide, lblDynamics, lblBalance, lblSpread;
    juce::Label lblInterval, lblVisLiss, lblVisPart, lblVisSpec;

    // APVTS attachments
    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment   = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment>    attVolume, attGlide, attDynamics;
    std::unique_ptr<SliderAttachment>    attBalance, attSpread, attInterval;
    std::unique_ptr<SliderAttachment>    attVisLiss, attVisPart, attVisSpec;
    std::unique_ptr<ButtonAttachment>    attDroneOn;
    std::unique_ptr<ComboBoxAttachment>  attHarmony;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SolarDroneAudioProcessorEditor)
};
