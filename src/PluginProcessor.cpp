#include "PluginProcessor.h"
#include "PluginEditor.h"

SolarDroneAudioProcessor::SolarDroneAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{}

SolarDroneAudioProcessor::~SolarDroneAudioProcessor() {}

void SolarDroneAudioProcessor::prepareToPlay(double, int) {}
void SolarDroneAudioProcessor::releaseResources() {}

void SolarDroneAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    buffer.clear();
}

juce::AudioProcessorEditor* SolarDroneAudioProcessor::createEditor() {
    return new SolarDroneAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new SolarDroneAudioProcessor();
}
