#include "PluginEditor.h"

static void setupSlider(juce::Slider& s, juce::Label& l,
                        const juce::String& name, juce::Component* parent) {
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

static void setupCombo(juce::ComboBox& c, juce::Label& l,
                       const juce::String& name, juce::Component* parent) {
    c.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff222222));
    c.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    parent->addAndMakeVisible(c);
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
    // ── Existing controls ──────────────────────────────────────────────────
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

    // ── Repeater controls ─────────────────────────────────────────────────
    setupSlider(slRepBPM, lblRepBPM, "BPM", this);
    slRepBPM.setTextBoxStyle(juce::Slider::TextBoxRight, false, 38, 16);
    slRepBPM.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slRepBPM.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setupSlider(slRepFeedback, lblRepFeedback, "FB",   this);
    setupSlider(slRepWet,      lblRepWet,      "Wet",  this);
    cmbRepBars.addItemList({"1/16","1/8","1/4","1/2","1 bar","2 bars"}, 1);
    setupCombo(cmbRepBars, lblRepBars, "Bars", this);
    btnRepOn.setButtonText("REP");
    btnRepOn.setColour(juce::ToggleButton::textColourId, juce::Colours::steelblue);
    addAndMakeVisible(btnRepOn);

    // ── Chopper controls ──────────────────────────────────────────────────
    setupSlider(slChopRate,  lblChopRate,  "Rate", this);
    setupSlider(slChopDepth, lblChopDepth, "Depth",this);
    cmbChopShape.addItemList({"Sine","Square","Saw"}, 1);
    setupCombo(cmbChopShape, lblChopShape, "Shape", this);
    cmbChopDiv.addItemList({"1/16","1/8","1/4","1/2","1 bar"}, 1);
    setupCombo(cmbChopDiv, lblChopDiv, "Div", this);
    btnChopOn.setButtonText("CHOP");
    btnChopOn.setColour(juce::ToggleButton::textColourId, juce::Colours::orange);
    addAndMakeVisible(btnChopOn);
    btnChopSync.setButtonText("Sync");
    btnChopSync.setColour(juce::ToggleButton::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(btnChopSync);

    // ── APVTS attachments ─────────────────────────────────────────────────
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

    attRepBPM      = std::make_unique<SliderAttachment>(apvts, "repeater_bpm",      slRepBPM);
    attRepFeedback = std::make_unique<SliderAttachment>(apvts, "repeater_feedback", slRepFeedback);
    attRepWet      = std::make_unique<SliderAttachment>(apvts, "repeater_wet",      slRepWet);
    attRepBars     = std::make_unique<ComboBoxAttachment>(apvts, "repeater_bars",   cmbRepBars);
    attRepOn       = std::make_unique<ButtonAttachment>(apvts, "repeater_on",       btnRepOn);

    attChopRate    = std::make_unique<SliderAttachment>(apvts, "chopper_rate",  slChopRate);
    attChopDepth   = std::make_unique<SliderAttachment>(apvts, "chopper_depth", slChopDepth);
    attChopShape   = std::make_unique<ComboBoxAttachment>(apvts, "chopper_shape", cmbChopShape);
    attChopDiv     = std::make_unique<ComboBoxAttachment>(apvts, "chopper_div",   cmbChopDiv);
    attChopOn      = std::make_unique<ButtonAttachment>(apvts, "chopper_on",   btnChopOn);
    attChopSync    = std::make_unique<ButtonAttachment>(apvts, "chopper_sync", btnChopSync);

    addAndMakeVisible(visualRenderer);
    setSize(600, 560);
}

SolarDroneAudioProcessorEditor::~SolarDroneAudioProcessorEditor() {}

void SolarDroneAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    auto ctrl = getLocalBounds().removeFromBottom(160);
    g.setColour(juce::Colour(0xff0e0e0e));
    g.fillRect(ctrl);

    // Section labels
    g.setFont(8.0f);
    g.setColour(juce::Colours::grey);
    g.drawText("REPEATER", ctrl.getX() + 4, ctrl.getY() + 80, 60, 12,
               juce::Justification::left, false);
    g.drawText("CHOPPER",  ctrl.getX() + 4, ctrl.getY() + 120, 60, 12,
               juce::Justification::left, false);

    // Harmony label above combo (row 2)
    g.drawText("Harm", ctrl.getX() + 4, ctrl.getY() + 4, 60, 12,
               juce::Justification::centred, false);
}

void SolarDroneAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();
    auto ctrl   = bounds.removeFromBottom(160);
    visualRenderer.setBounds(bounds);

    const int lh = 12, sh = 18, pad = 2;
    const int ctrlY = ctrl.getY();

    // ── Row 1 (y=ctrlY+4): Vol Glide Dyn Bal Sprd | ON ─────────────────
    const int btnW = 40;
    const int slW1 = (ctrl.getWidth() - btnW - 12) / 5;
    int x = ctrl.getX() + 4;
    auto place = [&](juce::Slider& s, juce::Label& l, int row, int w) {
        const int ry = ctrlY + row * 40 + 4;
        l.setBounds(x, ry, w, lh);
        s.setBounds(x, ry + lh, w, sh);
        x += w + pad;
    };
    place(slVolume,   lblVolume,   0, slW1);
    place(slGlide,    lblGlide,    0, slW1);
    place(slDynamics, lblDynamics, 0, slW1);
    place(slBalance,  lblBalance,  0, slW1);
    place(slSpread,   lblSpread,   0, slW1);
    btnDroneOn.setBounds(x, ctrlY + 4 + lh, btnW, sh);

    // ── Row 2 (y=ctrlY+44): Harm | Ivl Liss Part Spec ────────────────────
    const int comboW = 60;
    x = ctrl.getX() + 4;
    cmbHarmony.setBounds(x, ctrlY + 44 + lh, comboW, sh);
    x += comboW + pad;
    const int slW2 = (ctrl.getWidth() - comboW - 16) / 4;
    place(slInterval, lblInterval, 1, slW2);
    place(slVisLiss,  lblVisLiss,  1, slW2);
    place(slVisPart,  lblVisPart,  1, slW2);
    place(slVisSpec,  lblVisSpec,  1, slW2);

    // ── Row 3 (y=ctrlY+84): REP | Bars BPM FB Wet ────────────────────────
    x = ctrl.getX() + 4;
    btnRepOn.setBounds(x, ctrlY + 84 + lh, btnW, sh);
    x += btnW + pad;
    cmbRepBars.setBounds(x, ctrlY + 84 + lh, comboW, sh);
    lblRepBars.setBounds(x, ctrlY + 84, comboW, lh);
    x += comboW + pad;
    const int slW3 = (ctrl.getWidth() - btnW - comboW - 20) / 3;
    place(slRepBPM,      lblRepBPM,      2, slW3);
    place(slRepFeedback, lblRepFeedback, 2, slW3);
    place(slRepWet,      lblRepWet,      2, slW3);

    // ── Row 4 (y=ctrlY+124): CHOP Sync | Shape Div Rate Depth ────────────
    x = ctrl.getX() + 4;
    btnChopOn.setBounds(x, ctrlY + 124 + lh, btnW, sh);
    x += btnW + pad;
    btnChopSync.setBounds(x, ctrlY + 124 + lh, btnW, sh);
    x += btnW + pad;
    cmbChopShape.setBounds(x, ctrlY + 124 + lh, comboW, sh);
    lblChopShape.setBounds(x, ctrlY + 124, comboW, lh);
    x += comboW + pad;
    cmbChopDiv.setBounds(x, ctrlY + 124 + lh, comboW, sh);
    lblChopDiv.setBounds(x, ctrlY + 124, comboW, lh);
    x += comboW + pad;
    const int slW4 = (ctrl.getWidth() - 2*btnW - 2*comboW - 24) / 2;
    place(slChopRate,  lblChopRate,  3, slW4);
    place(slChopDepth, lblChopDepth, 3, slW4);
}
