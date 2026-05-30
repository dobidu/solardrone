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

    // Existing controls
    juce::Slider       slVolume, slGlide, slDynamics, slBalance, slSpread;
    juce::Slider       slInterval, slVisLiss, slVisPart, slVisSpec;
    juce::ToggleButton btnDroneOn;
    juce::ComboBox     cmbHarmony;
    juce::Label        lblVolume, lblGlide, lblDynamics, lblBalance, lblSpread;
    juce::Label        lblInterval, lblVisLiss, lblVisPart, lblVisSpec;

    // Beat Repeater controls
    juce::Slider       slRepBPM, slRepFeedback, slRepWet;
    juce::ComboBox     cmbRepBars;
    juce::ToggleButton btnRepOn;
    juce::Label        lblRepBPM, lblRepFeedback, lblRepWet, lblRepBars;

    // Chopper controls
    juce::Slider       slChopRate, slChopDepth;
    juce::ComboBox     cmbChopShape, cmbChopDiv;
    juce::ToggleButton btnChopOn, btnChopSync;
    juce::Label        lblChopRate, lblChopDepth, lblChopShape, lblChopDiv;

    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment   = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    // Existing attachments
    std::unique_ptr<SliderAttachment>    attVolume, attGlide, attDynamics;
    std::unique_ptr<SliderAttachment>    attBalance, attSpread, attInterval;
    std::unique_ptr<SliderAttachment>    attVisLiss, attVisPart, attVisSpec;
    std::unique_ptr<ButtonAttachment>    attDroneOn;
    std::unique_ptr<ComboBoxAttachment>  attHarmony;

    // Repeater attachments
    std::unique_ptr<SliderAttachment>    attRepBPM, attRepFeedback, attRepWet;
    std::unique_ptr<ComboBoxAttachment>  attRepBars;
    std::unique_ptr<ButtonAttachment>    attRepOn;

    // Chopper attachments
    std::unique_ptr<SliderAttachment>    attChopRate, attChopDepth;
    std::unique_ptr<ComboBoxAttachment>  attChopShape, attChopDiv;
    std::unique_ptr<ButtonAttachment>    attChopOn, attChopSync;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SolarDroneAudioProcessorEditor)
};
