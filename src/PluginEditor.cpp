#include "PluginEditor.h"

SolarDroneAudioProcessorEditor::SolarDroneAudioProcessorEditor(
    SolarDroneAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(400, 300);
}

SolarDroneAudioProcessorEditor::~SolarDroneAudioProcessorEditor() {}

void SolarDroneAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawFittedText("SolarDrone", getLocalBounds(),
                      juce::Justification::centred, 1);
}

void SolarDroneAudioProcessorEditor::resized() {}
