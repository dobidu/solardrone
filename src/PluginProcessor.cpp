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

    // Beat Repeater
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "repeater_on",       "Repeater On",    false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "repeater_bpm",      "Repeater BPM",   30.0f, 300.0f, 120.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "repeater_bars",     "Loop Length",
        juce::StringArray{"1/16","1/8","1/4","1/2","1 bar","2 bars"}, 2));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "repeater_feedback", "Feedback",       0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "repeater_wet",      "Repeater Wet",   0.0f, 1.0f, 0.7f));

    // Chopper
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "chopper_on",    "Chopper On",    false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "chopper_rate",  "Chopper Rate",  0.1f, 20.0f, 4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "chopper_depth", "Chopper Depth", 0.0f, 1.0f,  1.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "chopper_shape", "Chopper Shape",
        juce::StringArray{"Sine","Square","Saw"}, 1));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "chopper_sync",  "Chopper Sync",  true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "chopper_div",   "Chopper Div",
        juce::StringArray{"1/16","1/8","1/4","1/2","1 bar"}, 2));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "repeater_density", "Repeat Density", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "freeze_on",     "Freeze",        false));

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
    tempoTracker.prepare(sampleRate);
    beatRepeater.prepare(sampleRate, samplesPerBlock);
    chopper.prepare(sampleRate);
}

void SolarDroneAudioProcessor::releaseResources() {}

void SolarDroneAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
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

    // Poll DataFetcher — skip when frozen
    const bool frozen = *apvts.getRawParameterValue("freeze_on") > 0.5f;
    if (!frozen) {
        auto state = fetcher.getState();
        if (state.timestamp != lastTimestamp) {
            lastTimestamp = state.timestamp;
            engine.setSynthParams(SynthParamMapper::map(state, userParams));
        }
    }

    // Tempo tracking (MIDI clock + internal BPM fallback)
    tempoTracker.setInternalBPM(*apvts.getRawParameterValue("repeater_bpm"));
    tempoTracker.process(midi, buffer.getNumSamples());

    // Beat Repeater params
    const int beatPeriod = (int)(getSampleRate() * 60.0 / tempoTracker.getCurrentBPM());
    beatRepeater.setBeatPeriodSamples(beatPeriod);
    beatRepeater.setEnabled(*apvts.getRawParameterValue("repeater_on") > 0.5f);
    beatRepeater.setLoopLength(
        (BeatRepeater::LoopLength)(int)*apvts.getRawParameterValue("repeater_bars"));
    beatRepeater.setFeedback(*apvts.getRawParameterValue("repeater_feedback"));
    beatRepeater.setWet(*apvts.getRawParameterValue("repeater_wet"));
    beatRepeater.setDensity(*apvts.getRawParameterValue("repeater_density"));

    const bool droneOn = *apvts.getRawParameterValue("drone_on") > 0.5f;
    const float volume = *apvts.getRawParameterValue("volume");

    // Chopper params
    static const float divBeats[] = {0.25f, 0.5f, 1.0f, 2.0f, 4.0f};
    const int divIdx = std::min((int)*apvts.getRawParameterValue("chopper_div"), 4);
    const bool chopSync = *apvts.getRawParameterValue("chopper_sync") > 0.5f;
    chopper.setEnabled(*apvts.getRawParameterValue("chopper_on") > 0.5f);
    chopper.setDepth(*apvts.getRawParameterValue("chopper_depth"));
    chopper.setShape((Chopper::Shape)(int)*apvts.getRawParameterValue("chopper_shape"));
    chopper.setBPMSync(chopSync, tempoTracker.getCurrentBPM(), divBeats[divIdx]);
    if (!chopSync)
        chopper.setRate(*apvts.getRawParameterValue("chopper_rate"));

    if (droneOn) {
        engine.processBlock(buffer);
        beatRepeater.process(buffer);
        chopper.process(buffer);
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
