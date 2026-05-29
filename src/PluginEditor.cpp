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
    // Background and minimal label — visual fills the rest
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white.withAlpha(0.35f));
    g.setFont(11.0f);
    g.drawText("SolarDrone", getLocalBounds().reduced(8),
               juce::Justification::topLeft, false);
}

void SolarDroneAudioProcessorEditor::resized() {
    visualRenderer.setBounds(getLocalBounds());
}
