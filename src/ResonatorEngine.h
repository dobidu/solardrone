#pragma once
#include <juce_dsp/juce_dsp.h>
#include "ModalBank.h"
#include "FeedbackDelayNetwork.h"
#include "SympatheticStrings.h"
#include "SpaceWeatherState.h"

class ResonatorEngine {
public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    // Call from message thread (timerCallback) — solar data mappings
    void updateFromSolarData(const SpaceWeatherState& sw, float l1FundamentalHz);

    // APVTS-driven setters (audio thread or message thread — just float stores)
    void setModalEnabled(bool e)   { modalEnabled   = e; }
    void setFDNEnabled(bool e)     { fdnEnabled     = e; }
    void setStringsEnabled(bool e) { stringsEnabled = e; }
    void setModalWet(float w)      { modalWet   = w; modalBank.setWet(e_w(modalEnabled, w)); }
    void setFDNWet(float w)        { fdnWet     = w; fdn.setWet(e_w(fdnEnabled, w)); }
    void setStringsWet(float w)    { stringsWet = w; strings.setWet(e_w(stringsEnabled, w)); }
    void setModalDecay(float s)    { modalDecayUser = s; }
    void setFDNSize(float v)       { fdn.setSize(v); }
    void setStringCount(int n)     { strings.setStringCount(n); }

    // In-place: buffer = dry + modal_wet + fdn_wet + string_wet
    void process(juce::AudioBuffer<float>& buffer);

    // For PluginEditor timerCallback + updateResonatorSolarData
    bool isEnabled() const { return modalEnabled || fdnEnabled || stringsEnabled; }

private:
    static float e_w(bool enabled, float w) { return enabled ? w : 0.f; }

    ModalBank            modalBank;
    FeedbackDelayNetwork fdn;
    SympatheticStrings   strings;

    // Pre-allocated temp buffers (one per resonator)
    juce::AudioBuffer<float> modalBuf, fdnBuf, stringBuf;

    bool  modalEnabled   = false;
    bool  fdnEnabled     = false;
    bool  stringsEnabled = false;
    float modalWet       = 0.10f;
    float fdnWet         = 0.08f;
    float stringsWet     = 0.10f;
    float modalDecayUser = 2.0f;
    float fdnT60User     = 2.0f;
};
