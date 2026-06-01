#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "VisualRenderer.h"
#include "SunDisc.h"
#include "ColourScheme.h"
#include "MacroOrb.h"
#include "ProbDensityDial.h"
#include "MappingCurveDisplay.h"
#include "SpatialDisplay.h"
#include "OSCMIDIBridge.h"
#include "ResonatorPanel.h"

class SolarDroneAudioProcessorEditor
    : public juce::AudioProcessorEditor
    , public juce::Timer
{
public:
    explicit SolarDroneAudioProcessorEditor(SolarDroneAudioProcessor&);
    ~SolarDroneAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    SolarDroneAudioProcessor& processorRef;
    VisualRenderer  visualRenderer;
    SunDisc         sunDisc;
    MacroOrb        macroOrb;
    ProbDensityDial      probDial;
    MappingCurveDisplay  mappingDisplay;
    SpatialDisplay       spatialDisplay;
    ResonatorPanel       resonatorPanel;

    juce::TextButton btnToggleVisual;
    bool             showVisual = true;
    ResonatorPanel*  getResonatorPanel() { return &resonatorPanel; }
    float                currentKp = 0.f;

    juce::Slider slMapVelLo, slMapVelHi, slMapBzThresh, slMapKpDens;
    juce::Label  lblMapVelLo, lblMapVelHi, lblMapBzThresh, lblMapKpDens;

    juce::ToggleButton btnFreeze;

    // ── Right-panel controls ──────────────────────────────────────────────
    juce::Slider       slGlide, slDynamics, slBalance, slSpread;
    juce::Slider       slInterval, slVisLiss, slVisPart, slVisSpec;
    juce::ComboBox     cmbHarmony;
    juce::Label        lblGlide, lblDynamics, lblBalance, lblSpread;
    juce::Label        lblInterval, lblVisLiss, lblVisPart, lblVisSpec;

    // ── Bottom strip — Repeater ───────────────────────────────────────────
    juce::Slider       slRepBPM, slRepFeedback, slRepWet;
    juce::Slider       slRepPan;
    juce::ComboBox     cmbRepBars, cmbRepStutter;
    juce::ToggleButton btnRepOn, btnRepReverse;
    juce::Label        lblRepBPM, lblRepFeedback, lblRepWet, lblRepBars;
    juce::Label        lblRepPan, lblRepStutter;

    // ── Bottom strip — Chopper ────────────────────────────────────────────
    juce::Slider       slChopRate, slChopDepth;
    juce::Slider       slChopAttack, slChopRelease, slChopPhase;
    juce::ComboBox     cmbChopShape, cmbChopDiv;
    juce::ToggleButton btnChopOn, btnChopSync;
    juce::Label        lblChopRate, lblChopDepth, lblChopShape, lblChopDiv;
    juce::Label        lblChopAttack, lblChopRelease, lblChopPhase;

    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment   = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    // Right panel
    std::unique_ptr<SliderAttachment>    attGlide, attDynamics, attBalance, attSpread;
    std::unique_ptr<SliderAttachment>    attInterval, attVisLiss, attVisPart, attVisSpec;
    std::unique_ptr<ComboBoxAttachment>  attHarmony;

    // Repeater
    std::unique_ptr<SliderAttachment>    attRepBPM, attRepFeedback, attRepWet, attRepPan;
    std::unique_ptr<ComboBoxAttachment>  attRepBars, attRepStutter;
    std::unique_ptr<ButtonAttachment>    attRepOn, attRepReverse;

    // Chopper
    std::unique_ptr<SliderAttachment>    attChopRate, attChopDepth;
    std::unique_ptr<SliderAttachment>    attChopAttack, attChopRelease, attChopPhase;
    std::unique_ptr<ComboBoxAttachment>  attChopShape, attChopDiv;
    std::unique_ptr<ButtonAttachment>    attChopOn, attChopSync;
    std::unique_ptr<ButtonAttachment>    attFreeze;
    std::unique_ptr<SliderAttachment>    attMapVelLo, attMapVelHi, attMapBzThresh, attMapKpDens;

    // OSC/MIDI
    juce::ToggleButton btnOSC, btnMIDICC;
    juce::TextEditor   txtOSCPort;
    std::unique_ptr<ButtonAttachment>  attOSC, attMIDICC;
    float          oscActivityAlpha = 0.f;
    OSCMIDIBridge  oscMidiBridge;
    float          lastFlareLevelForOSC = 0.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SolarDroneAudioProcessorEditor)
};
