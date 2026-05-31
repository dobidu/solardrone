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
    // Mapping thresholds
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "map_vel_lo", "Map Vel Lo Hz",  30.0f,  200.0f,  55.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "map_vel_hi", "Map Vel Hi Hz", 100.0f,  440.0f, 220.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "map_bz_thresh", "Bz Thresh nT", -30.0f,  -0.5f, -10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "map_kp_dens",  "Kp Dens Start",  0.0f,    8.0f,   4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "flare_sensitivity", "Flare Sensitivity", 0.0f, 1.0f, 0.7f));
    // Output EQ
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "eq_low",  "EQ Low (dB)",  -12.f, 12.f, 0.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "eq_mid",  "EQ Mid (dB)",  -12.f, 12.f, 0.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "eq_high", "EQ High (dB)", -12.f, 12.f, 0.f));
    // Spatial positioning
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "l1_azimuth",   "L1 Azimuth",   -90.f, 90.f,   0.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "l1_elevation", "L1 Elevation", -45.f, 45.f,  15.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "l2_azimuth",   "L2 Azimuth",   -90.f, 90.f,  30.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "l2_elevation", "L2 Elevation", -45.f, 45.f,   0.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "l3_azimuth",   "L3 Azimuth",   -90.f, 90.f, -30.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "l3_elevation", "L3 Elevation", -45.f, 45.f,  30.f));

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
    outputEq.prepare({sampleRate, (juce::uint32)samplesPerBlock, 2});

    // SmoothedValues: 50ms ramp — eliminates slider crackling
    smoothedVolume.reset(sampleRate, 0.05);
    smoothedBalance.reset(sampleRate, 0.05);
    smoothedDynamics.reset(sampleRate, 0.05);
    smoothedVolume.setCurrentAndTargetValue(0.7f);
    smoothedBalance.setCurrentAndTargetValue(0.5f);
    smoothedDynamics.setCurrentAndTargetValue(1.0f);
}

void SolarDroneAudioProcessor::releaseResources() {}

void SolarDroneAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    // Smoothed critical params (eliminates crackling on rapid slider movement)
    smoothedVolume.setTargetValue(*apvts.getRawParameterValue("volume"));
    smoothedBalance.setTargetValue(*apvts.getRawParameterValue("layer_balance"));
    smoothedDynamics.setTargetValue(*apvts.getRawParameterValue("dynamics_range"));

    // Read APVTS params into UserParams
    userParams.dynamics_range            = smoothedDynamics.getNextValue();
    userParams.layer_balance             = smoothedBalance.getNextValue();
    userParams.stereo_spread             = *apvts.getRawParameterValue("stereo_spread");
    userParams.glide_time_secs           = *apvts.getRawParameterValue("glide_time");
    userParams.visual_lissajous          = *apvts.getRawParameterValue("vis_lissajous");
    userParams.visual_particles          = *apvts.getRawParameterValue("vis_particles");
    userParams.visual_spectral           = *apvts.getRawParameterValue("vis_spectral");
    userParams.l1_l2_interval_semitones  = (int)*apvts.getRawParameterValue("interval");
    userParams.harmony_mode = (*apvts.getRawParameterValue("harmony_mode") < 0.5f)
        ? UserParams::HarmonyMode::just
        : UserParams::HarmonyMode::equalTemperament;
    // Mapping thresholds
    userParams.map_vel_lo_hz     = *apvts.getRawParameterValue("map_vel_lo");
    userParams.map_vel_hi_hz     = *apvts.getRawParameterValue("map_vel_hi");
    userParams.map_bz_thresh     = *apvts.getRawParameterValue("map_bz_thresh");
    userParams.map_kp_dens_start = *apvts.getRawParameterValue("map_kp_dens");
    // Spatial positioning
    userParams.l1_azimuth   = *apvts.getRawParameterValue("l1_azimuth");
    userParams.l1_elevation = *apvts.getRawParameterValue("l1_elevation");
    userParams.l2_azimuth   = *apvts.getRawParameterValue("l2_azimuth");
    userParams.l2_elevation = *apvts.getRawParameterValue("l2_elevation");
    userParams.l3_azimuth   = *apvts.getRawParameterValue("l3_azimuth");
    userParams.l3_elevation = *apvts.getRawParameterValue("l3_elevation");

    engine.setUserParams(userParams);

    // L3 flare burst
    {
        const float flareSens = *apvts.getRawParameterValue("flare_sensitivity");
        const auto& sw = fetcher.getState();
        float flareLevel = 0.0f;
        if (sw.x_ray_flux >= 1e-5f)
            flareLevel = std::min(1.0f, (std::log10(sw.x_ray_flux) + 5.0f) / 2.0f);
        engine.setFlareLevel(flareLevel, flareSens);
    }

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
        // Smoothed volume (sample-accurate, no crackling)
        {
            const int n = buffer.getNumSamples();
            for (int i = 0; i < n; ++i) {
                const float g = smoothedVolume.getNextValue();
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                    buffer.getWritePointer(ch)[i] *= g;
            }
        }
        // Output EQ
        outputEq.setLowShelf( *apvts.getRawParameterValue("eq_low"));
        outputEq.setMidPeak(  *apvts.getRawParameterValue("eq_mid"));
        outputEq.setHighShelf(*apvts.getRawParameterValue("eq_high"));
        outputEq.process(buffer);
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
