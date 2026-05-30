#include "PluginEditor.h"

static void mkSlider(juce::Slider& s, juce::Label& l,
                     const juce::String& name, juce::Component* p) {
    s.setSliderStyle(juce::Slider::LinearHorizontal);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    s.setColour(juce::Slider::trackColourId,  juce::Colour(0xff2a6fa8));
    s.setColour(juce::Slider::thumbColourId,  juce::Colours::white);
    p->addAndMakeVisible(s);
    l.setText(name, juce::dontSendNotification);
    l.setFont(juce::Font(9.0f));
    l.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    l.setJustificationType(juce::Justification::centred);
    p->addAndMakeVisible(l);
}

static void mkCombo(juce::ComboBox& c, juce::Component* p) {
    c.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff181c24));
    c.setColour(juce::ComboBox::textColourId, juce::Colours::lightgrey);
    p->addAndMakeVisible(c);
}

SolarDroneAudioProcessorEditor::SolarDroneAudioProcessorEditor(
    SolarDroneAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
    , visualRenderer(&p)
    , sunDisc(p.apvts)
{
    // ── Right-panel sliders ────────────────────────────────────────────────
    mkSlider(slGlide,    lblGlide,    "Glide",  this);
    mkSlider(slDynamics, lblDynamics, "Dyn",    this);
    mkSlider(slBalance,  lblBalance,  "Bal",    this);
    mkSlider(slSpread,   lblSpread,   "Sprd",   this);
    mkSlider(slInterval, lblInterval, "Ivl",    this);
    mkSlider(slVisLiss,  lblVisLiss,  "Liss",   this);
    mkSlider(slVisPart,  lblVisPart,  "Part",   this);
    mkSlider(slVisSpec,  lblVisSpec,  "Spec",   this);
    cmbHarmony.addItem("Just",  1);
    cmbHarmony.addItem("Equal", 2);
    mkCombo(cmbHarmony, this);

    // ── Repeater strip ────────────────────────────────────────────────────
    mkSlider(slRepBPM,      lblRepBPM,      "BPM",  this);
    slRepBPM.setTextBoxStyle(juce::Slider::TextBoxRight, false, 38, 16);
    slRepBPM.setColour(juce::Slider::textBoxTextColourId,
                       juce::Colours::white);
    slRepBPM.setColour(juce::Slider::textBoxOutlineColourId,
                       juce::Colours::transparentBlack);
    mkSlider(slRepFeedback, lblRepFeedback, "FB",   this);
    mkSlider(slRepWet,      lblRepWet,      "Wet",  this);
    cmbRepBars.addItemList({"1/16","1/8","1/4","1/2","1 bar","2 bars"}, 1);
    mkCombo(cmbRepBars, this);
    addAndMakeVisible(lblRepBars);
    lblRepBars.setText("Bars", juce::dontSendNotification);
    lblRepBars.setFont(juce::Font(9.f));
    lblRepBars.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    btnRepOn.setButtonText("REP");
    btnRepOn.setColour(juce::ToggleButton::textColourId,
                       juce::Colours::steelblue);
    addAndMakeVisible(btnRepOn);

    // ── Chopper strip ─────────────────────────────────────────────────────
    mkSlider(slChopRate,  lblChopRate,  "Rate",  this);
    mkSlider(slChopDepth, lblChopDepth, "Depth", this);
    cmbChopShape.addItemList({"Sine","Square","Saw"}, 1);
    mkCombo(cmbChopShape, this);
    addAndMakeVisible(lblChopShape);
    lblChopShape.setText("Shape", juce::dontSendNotification);
    lblChopShape.setFont(juce::Font(9.f));
    lblChopShape.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    cmbChopDiv.addItemList({"1/16","1/8","1/4","1/2","1 bar"}, 1);
    mkCombo(cmbChopDiv, this);
    addAndMakeVisible(lblChopDiv);
    lblChopDiv.setText("Div", juce::dontSendNotification);
    lblChopDiv.setFont(juce::Font(9.f));
    lblChopDiv.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    btnChopOn.setButtonText("CHOP");
    btnChopOn.setColour(juce::ToggleButton::textColourId, juce::Colours::orange);
    addAndMakeVisible(btnChopOn);
    btnChopSync.setButtonText("Sync");
    btnChopSync.setColour(juce::ToggleButton::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(btnChopSync);

    // ── APVTS attachments ─────────────────────────────────────────────────
    auto& apvts = processorRef.apvts;
    attGlide    = std::make_unique<SliderAttachment>(apvts, "glide_time",     slGlide);
    attDynamics = std::make_unique<SliderAttachment>(apvts, "dynamics_range", slDynamics);
    attBalance  = std::make_unique<SliderAttachment>(apvts, "layer_balance",  slBalance);
    attSpread   = std::make_unique<SliderAttachment>(apvts, "stereo_spread",  slSpread);
    attInterval = std::make_unique<SliderAttachment>(apvts, "interval",       slInterval);
    attVisLiss  = std::make_unique<SliderAttachment>(apvts, "vis_lissajous",  slVisLiss);
    attVisPart  = std::make_unique<SliderAttachment>(apvts, "vis_particles",  slVisPart);
    attVisSpec  = std::make_unique<SliderAttachment>(apvts, "vis_spectral",   slVisSpec);
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
    addAndMakeVisible(sunDisc);

    startTimerHz(10);
    setSize(900, 600);
}

SolarDroneAudioProcessorEditor::~SolarDroneAudioProcessorEditor() {
    stopTimer();
}

void SolarDroneAudioProcessorEditor::timerCallback() {
    const float kp = processorRef.getLatestSpaceWeatherState().kp;
    if (kp != currentKp) {
        currentKp = kp;
        sunDisc.setKp(kp);
        repaint();
    }
}

void SolarDroneAudioProcessorEditor::paint(juce::Graphics& g) {
    const auto bg     = ColourScheme::background(currentKp);
    const auto accent = ColourScheme::kpToAccent(currentKp);
    const auto grid   = ColourScheme::grid(currentKp);

    g.fillAll(bg);

    // Space grid
    g.setColour(grid);
    for (int x = 0; x < 900; x += 40) g.drawVerticalLine(x, 0.f, 600.f);
    for (int y = 0; y < 600; y += 40) g.drawHorizontalLine(y, 0.f, 900.f);

    // Right panel tint
    g.setColour(accent.withAlpha(0.04f));
    g.fillRect(582, 0, 318, 520);

    // Bottom strip
    g.setColour(juce::Colour(0xff060c18));
    g.fillRect(0, 520, 900, 80);

    // Dividers
    g.setColour(accent.withAlpha(0.18f));
    g.drawVerticalLine(581, 0.f, 521.f);
    g.drawHorizontalLine(520, 0.f, 900.f);
    g.drawVerticalLine(449, 521.f, 600.f);  // REP | CHOP divider

    // Labels
    g.setFont(8.0f);
    g.setColour(accent.withAlpha(0.55f));
    g.drawText("REPEATER", 8,   522, 70, 10, juce::Justification::left, false);
    g.drawText("CHOPPER",  456, 522, 70, 10, juce::Justification::left, false);

    // Right panel title
    g.setFont(9.0f);
    g.setColour(accent.withAlpha(0.35f));
    g.drawText("PARAMETERS", 590, 4, 300, 10, juce::Justification::left, false);
}

void SolarDroneAudioProcessorEditor::resized() {
    // ── Visual zone ───────────────────────────────────────────────────────
    visualRenderer.setBounds(0, 0, 580, 520);
    // Sun-disc: centered on visual
    sunDisc.setBounds(220, 190, 140, 140);

    // ── Right panel ───────────────────────────────────────────────────────
    const int rx = 590, lh = 12, sh = 16, pad = 4, rw = 300;
    const int colW = (rw - pad) / 2;

    // Row 0: Harmony combo + Interval
    cmbHarmony.setBounds(rx, 20, 80, sh);
    lblInterval.setBounds(rx + 88, 20, colW - 88, lh);
    slInterval.setBounds( rx + 88, 20 + lh, rw - 88, sh);

    // Row 1: Glide + Dynamics
    int y = 58;
    lblGlide.setBounds(   rx,        y, colW, lh);
    slGlide.setBounds(    rx,        y + lh, colW, sh);
    lblDynamics.setBounds(rx + colW + pad, y, colW, lh);
    slDynamics.setBounds( rx + colW + pad, y + lh, colW, sh);

    // Row 2: Balance + Spread
    y = 98;
    lblBalance.setBounds(rx,              y, colW, lh);
    slBalance.setBounds( rx,              y + lh, colW, sh);
    lblSpread.setBounds( rx + colW + pad, y, colW, lh);
    slSpread.setBounds(  rx + colW + pad, y + lh, colW, sh);

    // Row 3-5: Visual blends
    y = 138;
    lblVisLiss.setBounds(rx,              y, colW, lh);
    slVisLiss.setBounds( rx,              y + lh, colW, sh);
    lblVisPart.setBounds(rx + colW + pad, y, colW, lh);
    slVisPart.setBounds( rx + colW + pad, y + lh, colW, sh);
    y = 178;
    lblVisSpec.setBounds(rx, y, colW, lh);
    slVisSpec.setBounds( rx, y + lh, colW, sh);

    // ── Bottom strip ──────────────────────────────────────────────────────
    const int sy = 524, sbh = 18, slbh = 12;

    // Repeater (left half, x=8 to x=445)
    int bx = 8;
    const int btnW = 42, comboW = 56;
    btnRepOn.setBounds(bx, sy + slbh, btnW, sbh);      bx += btnW + 2;
    lblRepBars.setBounds(bx, sy, comboW, slbh);
    cmbRepBars.setBounds(bx, sy + slbh, comboW, sbh);  bx += comboW + 2;
    const int repSlW = (437 - bx) / 3;
    lblRepBPM.setBounds(     bx,               sy, repSlW, slbh);
    slRepBPM.setBounds(      bx,               sy + slbh, repSlW, sbh);  bx += repSlW + 2;
    lblRepFeedback.setBounds(bx,               sy, repSlW, slbh);
    slRepFeedback.setBounds( bx,               sy + slbh, repSlW, sbh);  bx += repSlW + 2;
    lblRepWet.setBounds(     bx,               sy, repSlW, slbh);
    slRepWet.setBounds(      bx,               sy + slbh, repSlW, sbh);

    // Chopper (right half, x=456 to x=892)
    bx = 456;
    btnChopOn.setBounds(   bx, sy + slbh, btnW, sbh);     bx += btnW + 2;
    btnChopSync.setBounds( bx, sy + slbh, btnW - 4, sbh); bx += btnW - 2;
    lblChopShape.setBounds(bx, sy, comboW, slbh);
    cmbChopShape.setBounds(bx, sy + slbh, comboW, sbh);   bx += comboW + 2;
    lblChopDiv.setBounds(  bx, sy, comboW - 8, slbh);
    cmbChopDiv.setBounds(  bx, sy + slbh, comboW - 8, sbh); bx += comboW - 6;
    const int chopSlW = (892 - bx) / 2;
    lblChopRate.setBounds( bx,             sy, chopSlW, slbh);
    slChopRate.setBounds(  bx,             sy + slbh, chopSlW, sbh); bx += chopSlW + 2;
    lblChopDepth.setBounds(bx,             sy, chopSlW, slbh);
    slChopDepth.setBounds( bx,             sy + slbh, chopSlW, sbh);
}
