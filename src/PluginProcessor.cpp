#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout
SolarDroneAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "volume",         "Volume",            0.0f, 1.0f,   0.7f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "drone_on",       "Drone On",          true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "glide_time",     "Glide Time (s)",    30.0f, 300.0f, 120.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "dynamics_range", "Dynamics",          0.0f, 1.0f,   1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "layer_balance",  "Layer Balance",     0.0f, 1.0f,   0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "stereo_spread",  "Stereo Spread",     0.0f, 1.0f,   0.5f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "harmony_mode",   "Harmony Mode",
        juce::StringArray{"Just", "Equal Temp."}, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "interval",       "L1/L2 Interval",   0, 12, 7));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "vis_lissajous",  "Visual Lissajous",  0.0f, 1.0f,   0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "vis_particles",  "Visual Particles",  0.0f, 1.0f,   0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "vis_spectral",   "Visual Spectral",   0.0f, 1.0f,   0.7f));

    return { params.begin(), params.end() };
}

SolarDroneAudioProcessor::SolarDroneAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "SolarDroneParams", createParameterLayout())
{
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
    // Read APVTS params into UserParams
    userParams.dynamics_range            = *apvts.getRawParameterValue("dynamics_range");
    userParams.layer_balance             = *apvts.getRawParameterValue("layer_balance");
    userParams.stereo_spread             = *apvts.getRawParameterValue("stereo_spread");
    userParams.glide_time_secs           = *apvts.getRawParameterValue("glide_time");
    userParams.visual_lissajous          = *apvts.getRawParameterValue("vis_lissajous");
    userParams.visual_particles          = *apvts.getRawParameterValue("vis_particles");
    userParams.visual_spectral           = *apvts.getRawParameterValue("vis_spectral");
    userParams.l1_l2_interval_semitones  = (int)*apvts.getRawParameterValue("interval");
    userParams.harmony_mode = (*apvts.getRawParameterValue("harmony_mode") < 0.5f)
        ? UserParams::HarmonyMode::just
        : UserParams::HarmonyMode::equalTemperament;

    engine.setUserParams(userParams);

    // Poll DataFetcher — only re-map on new NOAA data
    auto state = fetcher.getState();
    if (state.timestamp != lastTimestamp) {
        lastTimestamp = state.timestamp;
        engine.setSynthParams(SynthParamMapper::map(state, userParams));
    }

    const bool droneOn = *apvts.getRawParameterValue("drone_on") > 0.5f;
    const float volume = *apvts.getRawParameterValue("volume");

    if (droneOn) {
        engine.processBlock(buffer);
        buffer.applyGain(volume);
    } else {
        buffer.clear();
    }
}

void SolarDroneAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SolarDroneAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
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
