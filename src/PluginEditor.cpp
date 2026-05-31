#include "PluginEditor.h"

static void mkSlider(juce::Slider& s, juce::Label& l,
                     const juce::String& name, juce::Component* p) {
    s.setSliderStyle(juce::Slider::LinearHorizontal);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    s.setColour(juce::Slider::trackColourId,  juce::Colour(0xff2a6fa8));
    s.setColour(juce::Slider::thumbColourId,  juce::Colours::white);
    p->addAndMakeVisible(s);
    l.setText(name, juce::dontSendNotification);
    l.setFont(juce::Font(11.0f));
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
    , macroOrb(p.apvts)
    , probDial(p.apvts)
    , spatialDisplay(p.apvts)
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

    // Freeze button
    btnFreeze.setButtonText("FREEZE");
    btnFreeze.setColour(juce::ToggleButton::textColourId, juce::Colours::cyan);
    addAndMakeVisible(btnFreeze);
    attFreeze = std::make_unique<ButtonAttachment>(apvts, "freeze_on", btnFreeze);

    // Mapping sliders
    mkSlider(slMapVelLo,   lblMapVelLo,   "V.Lo",  this);
    mkSlider(slMapVelHi,   lblMapVelHi,   "V.Hi",  this);
    mkSlider(slMapBzThresh,lblMapBzThresh, "Bz thr", this);
    mkSlider(slMapKpDens,  lblMapKpDens,  "Kp act", this);
    attMapVelLo    = std::make_unique<SliderAttachment>(apvts, "map_vel_lo",   slMapVelLo);
    attMapVelHi    = std::make_unique<SliderAttachment>(apvts, "map_vel_hi",   slMapVelHi);
    attMapBzThresh = std::make_unique<SliderAttachment>(apvts, "map_bz_thresh",slMapBzThresh);
    attMapKpDens   = std::make_unique<SliderAttachment>(apvts, "map_kp_dens",  slMapKpDens);
    addAndMakeVisible(mappingDisplay);

    // OSC/MIDI controls
    btnOSC.setButtonText("OSC");
    btnOSC.setColour(juce::ToggleButton::textColourId, juce::Colours::cyan);
    addAndMakeVisible(btnOSC);
    btnMIDICC.setButtonText("MIDI");
    btnMIDICC.setColour(juce::ToggleButton::textColourId, juce::Colours::plum);
    addAndMakeVisible(btnMIDICC);
    mkSlider(slOSCPort, lblOSCPort, "Port", this);
    slOSCPort.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 16);
    slOSCPort.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slOSCPort.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    attOSC     = std::make_unique<ButtonAttachment>(apvts, "osc_enabled",     btnOSC);
    attMIDICC  = std::make_unique<ButtonAttachment>(apvts, "midi_cc_enabled", btnMIDICC);
    attOSCPort = std::make_unique<SliderAttachment>(apvts, "osc_port",        slOSCPort);

    addAndMakeVisible(spatialDisplay);
    addAndMakeVisible(probDial);
    addAndMakeVisible(macroOrb);
    addAndMakeVisible(visualRenderer);
    addAndMakeVisible(sunDisc);

    startTimerHz(10);
    setSize(900, 640);
}

SolarDroneAudioProcessorEditor::~SolarDroneAudioProcessorEditor() {
    stopTimer();
}

void SolarDroneAudioProcessorEditor::timerCallback() {
    const float kp = processorRef.getLatestSpaceWeatherState().kp;
    if (kp != currentKp) {
        currentKp = kp;
        sunDisc.setKp(kp);
        macroOrb.setKp(kp);
        probDial.setKp(kp);
        mappingDisplay.setKp(kp);
        spatialDisplay.setKp(kp);
        // Flare level for corona
        const auto& sw2 = processorRef.getLatestSpaceWeatherState();
        float fl = 0.0f;
        if (sw2.x_ray_flux >= 1e-5f)
            fl = std::min(1.0f, (std::log10(sw2.x_ray_flux) + 5.0f) / 2.0f);
        sunDisc.setFlareLevel(fl);
        spatialDisplay.setFlareLevel(fl);
        // Activity indicator
        if (processorRef.isOSCRecentlySent())
            oscActivityAlpha = 1.0f;
        else
            oscActivityAlpha = std::max(0.f, oscActivityAlpha - 0.15f);
        mappingDisplay.setLive({
            processorRef.getLatestSpaceWeatherState().velocity,
            processorRef.getLatestSpaceWeatherState().bz_gsm,
            kp
        });
        mappingDisplay.setParams(
            *processorRef.apvts.getRawParameterValue("map_vel_lo"),
            *processorRef.apvts.getRawParameterValue("map_vel_hi"),
            *processorRef.apvts.getRawParameterValue("map_bz_thresh"),
            *processorRef.apvts.getRawParameterValue("map_kp_dens"));
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
    for (int x = 0; x < 900; x += 40) g.drawVerticalLine(x, 0.f, 640.f);
    for (int y = 0; y < 640; y += 40) g.drawHorizontalLine(y, 0.f, 900.f);

    // Right panel tint
    g.setColour(accent.withAlpha(0.04f));
    g.fillRect(582, 0, 318, 520);

    // Bottom strip (120px)
    g.setColour(juce::Colour(0xff060c18));
    g.fillRect(0, 520, 900, 120);

    // Dividers
    g.setColour(accent.withAlpha(0.18f));
    g.drawVerticalLine(581, 0.f, 521.f);
    g.drawHorizontalLine(519, 0.f, 900.f);
    g.drawVerticalLine(449, 520.f, 640.f);  // REP | CHOP divider

    // Section labels — 11pt, bright
    g.setFont(11.0f);
    g.setColour(accent.withAlpha(0.65f));
    g.drawText("REPEATER", 8,   522, 80, 14, juce::Justification::left, false);
    g.drawText("CHOPPER",  456, 522, 80, 14, juce::Justification::left, false);

    // Right panel title
    g.setFont(10.0f);
    g.setColour(accent.withAlpha(0.4f));
    g.drawText("PARAMETERS", 590, 4, 300, 12, juce::Justification::left, false);

    // OSC/MIDI activity indicator dot
    if (oscActivityAlpha > 0.01f) {
        g.setColour(juce::Colours::cyan.withAlpha(oscActivityAlpha));
        g.fillEllipse(882, 502, 6, 6);
    }
    // OSC/MIDI section label
    g.setFont(8.0f);
    g.setColour(accent.withAlpha(0.4f));
    g.drawText("OSC / MIDI", 590, 487, 60, 12, juce::Justification::left, false);
}

void SolarDroneAudioProcessorEditor::resized() {
    // ── Visual zone ───────────────────────────────────────────────────────
    visualRenderer.setBounds(0, 0, 580, 520);
    sunDisc.setBounds(220, 190, 140, 140);
    // Freeze button — overlaid top-right of visual
    btnFreeze.setBounds(468, 6, 100, 22);

    // ── Right panel ───────────────────────────────────────────────────────
    const int rx = 590, lh = 12, sh = 16, pad = 4, rw = 300;
    const int colW = (rw - pad) / 2;

    // Row 0: Harmony combo + Interval
    cmbHarmony.setBounds(rx, 20, 80, sh);
    lblInterval.setBounds(rx + 88, 20, colW - 88, lh);
    slInterval.setBounds( rx + 88, 20 + lh, rw - 88, sh);

    // Row 1: Glide + Dynamics
    int y = 54;
    lblGlide.setBounds(   rx,              y, colW, lh);
    slGlide.setBounds(    rx,              y + lh, colW, sh);
    lblDynamics.setBounds(rx + colW + pad, y, colW, lh);
    slDynamics.setBounds( rx + colW + pad, y + lh, colW, sh);

    // Row 2: Balance + Spread
    y = 88;
    lblBalance.setBounds(rx,              y, colW, lh);
    slBalance.setBounds( rx,              y + lh, colW, sh);
    lblSpread.setBounds( rx + colW + pad, y, colW, lh);
    slSpread.setBounds(  rx + colW + pad, y + lh, colW, sh);

    // Row 3: Visual blends
    y = 122;
    lblVisLiss.setBounds(rx,              y, colW, lh);
    slVisLiss.setBounds( rx,              y + lh, colW, sh);
    lblVisPart.setBounds(rx + colW + pad, y, colW, lh);
    slVisPart.setBounds( rx + colW + pad, y + lh, colW, sh);
    y = 155;
    lblVisSpec.setBounds(rx, y, colW, lh);
    slVisSpec.setBounds( rx, y + lh, colW, sh);

    // Spatial display (y=195, 120×80)
    spatialDisplay.setBounds(rx + 90, 195, 120, 80);

    // Mapping section (y=283 to y=373)
    mappingDisplay.setBounds(rx, 283, rw, 72);   // 2×2 mini-curves
    const int mSlW = rw / 4;
    const int mY   = 359;
    lblMapVelLo.setBounds(   rx + 0*mSlW, mY,    mSlW, lh);
    slMapVelLo.setBounds(    rx + 0*mSlW, mY+lh, mSlW, sh);
    lblMapVelHi.setBounds(   rx + 1*mSlW, mY,    mSlW, lh);
    slMapVelHi.setBounds(    rx + 1*mSlW, mY+lh, mSlW, sh);
    lblMapBzThresh.setBounds(rx + 2*mSlW, mY,    mSlW, lh);
    slMapBzThresh.setBounds( rx + 2*mSlW, mY+lh, mSlW, sh);
    lblMapKpDens.setBounds(  rx + 3*mSlW, mY,    mSlW, lh);
    slMapKpDens.setBounds(   rx + 3*mSlW, mY+lh, mSlW, sh);

    // MacroOrb (compact, y=385)
    macroOrb.setBounds(rx + 30, 385, 240, 110);

    // OSC/MIDI section (y=500)
    btnOSC.setBounds(   rx,       500, 44, 18);
    slOSCPort.setBounds(rx + 48,  500, 130, 18);
    lblOSCPort.setBounds(rx + 48, 487, 130, 12);
    btnMIDICC.setBounds(rx + 182, 500, 52, 18);

    // ── Bottom strip (120px, y=520-640) ──────────────────────────────────
    // Two rows: label row (y=538, 14px) + control row (y=554, 22px)
    // + second label row (y=582, 14px) + second control row (y=598, 22px)
    const int stripTop = 520;
    const int lbH = 14, ctH = 22, gap = 4;
    const int row1L = stripTop + 18;   // label y row 1
    const int row1C = row1L + lbH;     // control y row 1
    const int row2L = row1C + ctH + 6; // label y row 2
    const int row2C = row2L + lbH;     // control y row 2

    // Left half REPEATER: x=8 to x=444
    {
        int bx = 8;
        const int dialSz = 72;
        // ProbDial spans both rows
        probDial.setBounds(bx, stripTop + 4, dialSz, dialSz); bx += dialSz + 4;

        // btnRepOn
        btnRepOn.setBounds(bx, row1C, 48, ctH); bx += 50;

        // Bars combo
        lblRepBars.setBounds(bx, row1L, 50, lbH);
        cmbRepBars.setBounds(bx, row1C, 50, ctH); bx += 52;

        // BPM slider (row 1 remainder)
        const int bpmW = 110;
        lblRepBPM.setBounds(bx, row1L, bpmW, lbH);
        slRepBPM.setBounds( bx, row1C, bpmW, ctH); bx += bpmW + 4;

        // FB + Wet on row 2 (from dialSz start)
        int bx2 = 8 + dialSz + 4 + 50 + 52;
        const int slW = (440 - bx2) / 2;
        lblRepFeedback.setBounds(bx2, row2L, slW, lbH);
        slRepFeedback.setBounds( bx2, row2C, slW, ctH); bx2 += slW + 4;
        lblRepWet.setBounds(bx2, row2L, slW, lbH);
        slRepWet.setBounds( bx2, row2C, slW, ctH);
    }

    // Right half CHOPPER: x=452 to x=892
    {
        int bx = 452;
        btnChopOn.setBounds(  bx, row1C, 52, ctH); bx += 54;
        btnChopSync.setBounds(bx, row1C, 44, ctH); bx += 46;

        const int comboW = 64;
        lblChopShape.setBounds(bx, row1L, comboW, lbH);
        cmbChopShape.setBounds(bx, row1C, comboW, ctH); bx += comboW + 4;
        lblChopDiv.setBounds(  bx, row1L, comboW, lbH);
        cmbChopDiv.setBounds(  bx, row1C, comboW, ctH); bx += comboW + 4;

        const int chopSlW = (890 - bx) / 2;
        lblChopRate.setBounds( bx,             row2L, chopSlW, lbH);
        slChopRate.setBounds(  bx,             row2C, chopSlW, ctH); bx += chopSlW + 4;
        lblChopDepth.setBounds(bx,             row2L, chopSlW, lbH);
        slChopDepth.setBounds( bx,             row2C, chopSlW, ctH);
    }
}
