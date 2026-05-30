#include "PluginProcessor.h"
#include "PluginEditor.h"

SolarDroneAudioProcessor::SolarDroneAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    // DataFetcher starts background thread in its constructor automatically.
    // Engine seeded with defaults until first NOAA fetch (~30s on cold start).
    engine.setUserParams(userParams);
}

SolarDroneAudioProcessor::~SolarDroneAudioProcessor() {}

void SolarDroneAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    engine.prepare(sampleRate, samplesPerBlock);
}

void SolarDroneAudioProcessor::releaseResources() {}

void SolarDroneAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // Poll DataFetcher — cheap: mutex lock + struct copy
    auto state = fetcher.getState();
    if (state.timestamp != lastTimestamp) {
        lastTimestamp = state.timestamp;
        engine.setSynthParams(SynthParamMapper::map(state, userParams));
    }
    engine.processBlock(buffer);
}

SynthParams SolarDroneAudioProcessor::getCurrentSynthParams() const {
    return engine.getSmoothedParams();
}

SpaceWeatherState SolarDroneAudioProcessor::getLatestSpaceWeatherState() const {
    return fetcher.getState();
}

juce::AudioProcessorEditor* SolarDroneAudioProcessor::createEditor() {
    return new SolarDroneAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new SolarDroneAudioProcessor();
}
