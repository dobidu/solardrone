#include "PluginProcessor.h"
#include "PluginEditor.h"

SolarDroneAudioProcessor::SolarDroneAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    // Seed engine with a quiet default state so it drones immediately on open
    SpaceWeatherState defaultState;
    defaultState.velocity = 450.0f;
    defaultState.kp       = 2.0f;
    engine.setSynthParams(SynthParamMapper::map(defaultState));
}

SolarDroneAudioProcessor::~SolarDroneAudioProcessor() {}

void SolarDroneAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    engine.prepare(sampleRate, samplesPerBlock);
}

void SolarDroneAudioProcessor::releaseResources() {}

void SolarDroneAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    engine.processBlock(buffer);
}

SynthParams SolarDroneAudioProcessor::getCurrentSynthParams() const {
    return engine.getSmoothedParams();
}

juce::AudioProcessorEditor* SolarDroneAudioProcessor::createEditor() {
    return new SolarDroneAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new SolarDroneAudioProcessor();
}
