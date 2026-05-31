#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "AdditiveEngine.h"
#include "SynthParamMapper.h"
#include "DataFetcher.h"
#include "UserParams.h"
#include "OutputEQ.h"
#include "TempoTracker.h"
#include "BeatRepeater.h"
#include "Chopper.h"

class SolarDroneAudioProcessor : public juce::AudioProcessor {
public:
    SolarDroneAudioProcessor();
    ~SolarDroneAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    SynthParams         getCurrentSynthParams() const;
    SpaceWeatherState   getLatestSpaceWeatherState() const;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    DataFetcher    fetcher;
    UserParams     userParams;
    juce::String   lastTimestamp;
    AdditiveEngine engine;
    TempoTracker   tempoTracker;
    BeatRepeater   beatRepeater;
    Chopper        chopper;
    OutputEQ       outputEq;

    juce::SmoothedValue<float> smoothedVolume  {0.7f};
    juce::SmoothedValue<float> smoothedBalance {0.5f};
    juce::SmoothedValue<float> smoothedDynamics{1.0f};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SolarDroneAudioProcessor)
};
