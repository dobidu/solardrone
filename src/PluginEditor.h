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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SolarDroneAudioProcessorEditor)
};
