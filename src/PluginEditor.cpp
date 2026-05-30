#include "PluginEditor.h"

SolarDroneAudioProcessorEditor::SolarDroneAudioProcessorEditor(
    SolarDroneAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), visualRenderer(&p)
{
    addAndMakeVisible(visualRenderer);
    setSize(600, 400);
}

SolarDroneAudioProcessorEditor::~SolarDroneAudioProcessorEditor() {}

void SolarDroneAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
}

void SolarDroneAudioProcessorEditor::resized() {
    visualRenderer.setBounds(getLocalBounds());
}
