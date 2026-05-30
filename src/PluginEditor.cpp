#include "PluginEditor.h"

static void setupSlider(juce::Slider& s, juce::Label& l,
                        const juce::String& name,
                        juce::Component* parent) {
    s.setSliderStyle(juce::Slider::LinearHorizontal);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    s.setColour(juce::Slider::trackColourId, juce::Colour(0xff2a6fa8));
    s.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    parent->addAndMakeVisible(s);

    l.setText(name, juce::dontSendNotification);
    l.setFont(juce::Font(9.0f));
    l.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    l.setJustificationType(juce::Justification::centred);
    parent->addAndMakeVisible(l);
}

SolarDroneAudioProcessorEditor::SolarDroneAudioProcessorEditor(
    SolarDroneAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), visualRenderer(&p)
{
    setupSlider(slVolume,   lblVolume,   "Vol",  this);
    setupSlider(slGlide,    lblGlide,    "Glide",this);
    setupSlider(slDynamics, lblDynamics, "Dyn",  this);
    setupSlider(slBalance,  lblBalance,  "Bal",  this);
    setupSlider(slSpread,   lblSpread,   "Sprd", this);
    setupSlider(slInterval, lblInterval, "Ivl",  this);
    setupSlider(slVisLiss,  lblVisLiss,  "Liss", this);
    setupSlider(slVisPart,  lblVisPart,  "Part", this);
    setupSlider(slVisSpec,  lblVisSpec,  "Spec", this);

    btnDroneOn.setButtonText("ON");
    btnDroneOn.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible(btnDroneOn);

    cmbHarmony.addItem("Just",  1);
    cmbHarmony.addItem("Equal", 2);
    cmbHarmony.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff222222));
    cmbHarmony.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    addAndMakeVisible(cmbHarmony);

    auto& apvts = processorRef.apvts;
    attVolume   = std::make_unique<SliderAttachment>(apvts, "volume",         slVolume);
    attGlide    = std::make_unique<SliderAttachment>(apvts, "glide_time",     slGlide);
    attDynamics = std::make_unique<SliderAttachment>(apvts, "dynamics_range", slDynamics);
    attBalance  = std::make_unique<SliderAttachment>(apvts, "layer_balance",  slBalance);
    attSpread   = std::make_unique<SliderAttachment>(apvts, "stereo_spread",  slSpread);
    attInterval = std::make_unique<SliderAttachment>(apvts, "interval",       slInterval);
    attVisLiss  = std::make_unique<SliderAttachment>(apvts, "vis_lissajous",  slVisLiss);
    attVisPart  = std::make_unique<SliderAttachment>(apvts, "vis_particles",  slVisPart);
    attVisSpec  = std::make_unique<SliderAttachment>(apvts, "vis_spectral",   slVisSpec);
    attDroneOn  = std::make_unique<ButtonAttachment>(apvts, "drone_on",       btnDroneOn);
    attHarmony  = std::make_unique<ComboBoxAttachment>(apvts, "harmony_mode", cmbHarmony);

    addAndMakeVisible(visualRenderer);
    setSize(600, 480);
}

SolarDroneAudioProcessorEditor::~SolarDroneAudioProcessorEditor() {}

void SolarDroneAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    // Controls strip background
    auto ctrl = getLocalBounds().removeFromBottom(80);
    g.setColour(juce::Colour(0xff0e0e0e));
    g.fillRect(ctrl);
    // "Harm" label above combo (row 2)
    g.setFont(9.0f);
    g.setColour(juce::Colours::lightgrey);
    g.drawText("Harm", ctrl.getX() + 4, ctrl.getY() + 4, 60, 12,
               juce::Justification::centred, false);
}

void SolarDroneAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();
    auto ctrl   = bounds.removeFromBottom(80);
    visualRenderer.setBounds(bounds);

    const int lh = 12;
    const int sh = 18;
    const int pad = 2;
    const int row1y = ctrl.getY() + 4;
    const int row2y = row1y + lh + sh + 4;

    // Row 1: Vol | Glide | Dyn | Bal | Sprd | [ON toggle]
    const int btnW  = 40;
    const int slW1  = (ctrl.getWidth() - btnW - 12) / 5;
    int x = ctrl.getX() + 4;
    auto placeSlider = [&](juce::Slider& s, juce::Label& l, int row, int w) {
        const int ry = (row == 1) ? row1y : row2y;
        l.setBounds(x, ry, w, lh);
        s.setBounds(x, ry + lh, w, sh);
        x += w + pad;
    };
    placeSlider(slVolume,   lblVolume,   1, slW1);
    placeSlider(slGlide,    lblGlide,    1, slW1);
    placeSlider(slDynamics, lblDynamics, 1, slW1);
    placeSlider(slBalance,  lblBalance,  1, slW1);
    placeSlider(slSpread,   lblSpread,   1, slW1);
    btnDroneOn.setBounds(x, row1y + lh, btnW, sh);

    // Row 2: Harm combo | Ivl | Liss | Part | Spec
    const int comboW = 60;
    x = ctrl.getX() + 4;
    cmbHarmony.setBounds(x, row2y + lh, comboW, sh);
    x += comboW + pad;
    const int slW2 = (ctrl.getWidth() - comboW - 16) / 4;
    placeSlider(slInterval, lblInterval, 2, slW2);
    placeSlider(slVisLiss,  lblVisLiss,  2, slW2);
    placeSlider(slVisPart,  lblVisPart,  2, slW2);
    placeSlider(slVisSpec,  lblVisSpec,  2, slW2);
}
