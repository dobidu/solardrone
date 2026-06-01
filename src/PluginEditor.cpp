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
    , resonatorPanel(p.apvts)
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
    btnRepOn.setColour(juce::ToggleButton::textColourId, juce::Colours::steelblue);
    addAndMakeVisible(btnRepOn);
    // Row 3 REP
    mkSlider(slRepPan,  lblRepPan,  "Pan",  this);
    cmbRepStutter.addItemList({"x1","x2","x4","x8"}, 1);
    mkCombo(cmbRepStutter, this);
    addAndMakeVisible(lblRepStutter);
    lblRepStutter.setText("Stutter", juce::dontSendNotification);
    lblRepStutter.setFont(juce::Font(9.f));
    lblRepStutter.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    btnRepReverse.setButtonText("REV");
    btnRepReverse.setColour(juce::ToggleButton::textColourId, juce::Colours::salmon);
    addAndMakeVisible(btnRepReverse);

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
    // Row 3 CHOP
    mkSlider(slChopAttack,  lblChopAttack,  "Atk",   this);
    mkSlider(slChopRelease, lblChopRelease, "Rel",   this);
    mkSlider(slChopPhase,   lblChopPhase,   "Phase", this);

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
    attRepPan      = std::make_unique<SliderAttachment>(apvts, "repeater_pan",      slRepPan);
    attRepBars     = std::make_unique<ComboBoxAttachment>(apvts, "repeater_bars",   cmbRepBars);
    attRepStutter  = std::make_unique<ComboBoxAttachment>(apvts, "repeater_stutter",cmbRepStutter);
    attRepOn       = std::make_unique<ButtonAttachment>(apvts, "repeater_on",       btnRepOn);
    attRepReverse  = std::make_unique<ButtonAttachment>(apvts, "repeater_reverse",  btnRepReverse);

    attChopRate    = std::make_unique<SliderAttachment>(apvts, "chopper_rate",    slChopRate);
    attChopDepth   = std::make_unique<SliderAttachment>(apvts, "chopper_depth",   slChopDepth);
    attChopAttack  = std::make_unique<SliderAttachment>(apvts, "chopper_attack",  slChopAttack);
    attChopRelease = std::make_unique<SliderAttachment>(apvts, "chopper_release", slChopRelease);
    attChopPhase   = std::make_unique<SliderAttachment>(apvts, "chopper_phase",   slChopPhase);
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
    // OSC port as editable text field
    txtOSCPort.setText("9000", false);
    txtOSCPort.setInputRestrictions(5, "0123456789");
    txtOSCPort.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff181c24));
    txtOSCPort.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    txtOSCPort.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff2a6fa8));
    txtOSCPort.onReturnKey = [this]() {
        const int p = std::max(1024, std::min(65535, txtOSCPort.getText().getIntValue()));
        txtOSCPort.setText(juce::String(p), false);
        if (auto* param = processorRef.apvts.getParameter("osc_port"))
            param->setValueNotifyingHost(param->convertTo0to1((float)p));
    };
    txtOSCPort.onFocusLost = txtOSCPort.onReturnKey;
    addAndMakeVisible(txtOSCPort);
    attOSC    = std::make_unique<ButtonAttachment>(apvts, "osc_enabled",     btnOSC);
    attMIDICC = std::make_unique<ButtonAttachment>(apvts, "midi_cc_enabled", btnMIDICC);

    // Visual toggle button (always visible in bottom strip)
    btnToggleVisual.setButtonText("HIDE VISUAL");
    btnToggleVisual.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a2a3a));
    btnToggleVisual.setColour(juce::TextButton::textColourOffId, juce::Colours::lightgrey);
    btnToggleVisual.onClick = [this]() {
        showVisual = !showVisual;
        visualRenderer.setVisible(showVisual);
        sunDisc.setVisible(showVisual);
        btnFreeze.setVisible(showVisual);
        btnToggleVisual.setButtonText(showVisual ? "HIDE VISUAL" : "SHOW VISUAL");
        setSize(showVisual ? 1350 : 690, showVisual ? 780 : 890);
    };
    addAndMakeVisible(btnToggleVisual);
    addAndMakeVisible(resonatorPanel);
    addAndMakeVisible(spatialDisplay);
    addAndMakeVisible(probDial);
    addAndMakeVisible(macroOrb);
    addAndMakeVisible(visualRenderer);
    addAndMakeVisible(sunDisc);

    startTimerHz(10);
    setSize(1350, 780);
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

        // Resonator solar update + panel live data
        processorRef.updateResonatorSolarData();
        resonatorPanel.setKp(kp);
        resonatorPanel.setLiveData(
            processorRef.getLatestSpaceWeatherState().velocity,
            processorRef.getLatestSpaceWeatherState().temperature,
            processorRef.getLatestSpaceWeatherState().dst_index,
            processorRef.getLatestSpaceWeatherState().proton_flux_10mev);
        resonatorPanel.updateTerminal(
            processorRef.getLatestSpaceWeatherState(),
            *processorRef.apvts.getRawParameterValue("res_modal_on")   > 0.5f,
            *processorRef.apvts.getRawParameterValue("res_fdn_on")     > 0.5f,
            *processorRef.apvts.getRawParameterValue("res_strings_on") > 0.5f);
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

    // OSC/MIDI — always runs every timer tick (not gated by kp change)
    {
        const int oscPort = std::max(1024, std::min(65535,
            txtOSCPort.getText().isEmpty() ? 9000 : txtOSCPort.getText().getIntValue()));
        const auto& sw = processorRef.getLatestSpaceWeatherState();
        float fl2 = 0.f;
        if (sw.x_ray_flux >= 1e-5f)
            fl2 = std::min(1.f, (std::log10(sw.x_ray_flux) + 5.f) / 2.f);
        oscMidiBridge.setOSCEnabled(
            *processorRef.apvts.getRawParameterValue("osc_enabled") > 0.5f, oscPort);
        oscMidiBridge.setMIDIEnabled(
            *processorRef.apvts.getRawParameterValue("midi_cc_enabled") > 0.5f);
        oscMidiBridge.send(processorRef.getCurrentSynthParams(), sw, fl2,
                           *processorRef.apvts.getRawParameterValue("volume"),
                           *processorRef.apvts.getRawParameterValue("layer_balance"));
        if (oscMidiBridge.wasRecentlySent())
            oscActivityAlpha = 1.0f;
        else
            oscActivityAlpha = std::max(0.f, oscActivityAlpha - 0.15f);
    }
}

void SolarDroneAudioProcessorEditor::paint(juce::Graphics& g) {
    const auto bg     = ColourScheme::background(currentKp);
    const auto accent = ColourScheme::kpToAccent(currentKp);
    const auto grid   = ColourScheme::grid(currentKp);

    g.fillAll(bg);

    // Space grid
    g.setColour(grid);
    const int W = getWidth(), H = getHeight();
    for (int x = 0; x < W; x += 40) g.drawVerticalLine(x, 0.f, (float)H);
    for (int y = 0; y < H; y += 40) g.drawHorizontalLine(y, 0.f, (float)W);

    const int TW  = getWidth();
    const int rx  = showVisual ? 660  : 0;    // params column x
    const int resx= showVisual ? 1020 : 360;  // resonator column x
    const int chopX = TW / 2;                 // REP|CHOP divider

    // Right panel tint
    g.setColour(accent.withAlpha(0.04f));
    g.fillRect(rx, 0, 360, 600);
    // Resonator column tint
    g.setColour(accent.withAlpha(0.025f));
    g.fillRect(resx, 0, TW - resx, 600);

    // Bottom strip
    const int stripH = showVisual ? 180 : 290;
    g.setColour(juce::Colour(0xff060c18));
    g.fillRect(0, 600, TW, stripH);

    // Dividers
    g.setColour(accent.withAlpha(0.18f));
    if (showVisual) g.drawVerticalLine(659, 0.f, 601.f);
    g.drawVerticalLine(rx + 360, 0.f, 601.f);
    g.drawHorizontalLine(599, 0.f, (float)TW);

    // Section labels + divider (layout-dependent)
    g.setFont(12.0f);
    g.setColour(accent.withAlpha(0.65f));
    // REPEATER label always at x=104 (after ProbDensityDial which occupies x=8..98)
    if (showVisual) {
        // wide: vertical divider, labels side by side
        g.drawVerticalLine(chopX, 600.f, (float)(600 + stripH));
        g.drawText("REPEATER", 104,        602, 110, 14, juce::Justification::left, false);
        g.drawText("CHOPPER",  chopX + 8,  602, 90,  14, juce::Justification::left, false);
    } else {
        // compact: horizontal divider, labels stacked
        g.drawHorizontalLine(750, 0.f, (float)TW);
        g.drawText("REPEATER", 104, 602, 110, 14, juce::Justification::left, false);
        g.drawText("CHOPPER",  8,   752, 90,  14, juce::Justification::left, false);
    }

    // Column titles
    g.setFont(11.0f);
    g.setColour(accent.withAlpha(0.4f));
    g.drawText("PARAMETERS", rx + 6,   4, 320, 14, juce::Justification::left, false);
    g.drawText("RESONATORS", resx + 6, 4, 300, 14, juce::Justification::left, false);

    // OSC/MIDI section label (below MacroOrb which ends at y=552)
    g.setFont(10.0f);
    g.setColour(accent.withAlpha(0.4f));
    g.drawText("OSC / MIDI", rx + 6, 556, 100, 12, juce::Justification::left, false);
    // Activity dot: right of MIDI button (ends at rx+218)
    if (oscActivityAlpha > 0.01f) {
        g.setColour(juce::Colours::cyan.withAlpha(oscActivityAlpha));
        g.fillEllipse((float)(rx + 226), 574.f, 9.f, 9.f);
    }
}

void SolarDroneAudioProcessorEditor::resized() {
    // ── Dynamic layout (showVisual flag) ─────────────────────────────────
    const int rx   = showVisual ? 660  : 0;    // params column x
    const int resx = showVisual ? 1020 : 360;  // resonator column x

    // Visual zone
    if (showVisual) {
        visualRenderer.setBounds(0, 0, 660, 600);
        sunDisc.setBounds(260, 230, 140, 140);
        btnFreeze.setBounds(548, 6, 100, 24);
    }
    resonatorPanel.setBounds(resx, 0, 330, 600);

    // ── Params panel (dynamic x=rx) ──────────────────────────────────────
    const int lh = 14, sh = 22, pad = 6, rw = 360;
    const int colW = (rw - pad) / 2;

    // Row 0: y=20 (gap after "PARAMETERS" label which ends at y=18)
    cmbHarmony.setBounds(rx, 20, 100, sh);
    lblInterval.setBounds(rx+108, 20,    rw-108, lh);
    slInterval.setBounds( rx+108, 20+lh, rw-108, sh);

    auto placeRow = [&](juce::Slider& sL, juce::Label& lL,
                        juce::Slider& sR, juce::Label& lR, int y) {
        lL.setBounds(rx,          y, colW, lh);   sL.setBounds(rx,          y+lh, colW, sh);
        lR.setBounds(rx+colW+pad, y, colW, lh);   sR.setBounds(rx+colW+pad, y+lh, colW, sh);
    };
    placeRow(slGlide,   lblGlide,   slDynamics, lblDynamics, 54);
    placeRow(slBalance, lblBalance, slSpread,   lblSpread,   94);
    placeRow(slVisLiss, lblVisLiss, slVisPart,  lblVisPart,  134);
    lblVisSpec.setBounds(rx, 174, colW, lh);
    slVisSpec.setBounds( rx, 174+lh, colW, sh);   // bottom=210

    // Spatial display: y=216, 150×86
    spatialDisplay.setBounds(rx+105, 216, 150, 86);  // bottom=302

    // Mapping curves: y=308, 360×80
    mappingDisplay.setBounds(rx, 308, rw, 80);        // bottom=388

    // 4 mapping sliders: y=392
    const int mSlW = rw / 4;
    const int mY   = 392;
    lblMapVelLo.setBounds(   rx+0*mSlW, mY,    mSlW, lh); slMapVelLo.setBounds(   rx+0*mSlW, mY+lh, mSlW, sh);
    lblMapVelHi.setBounds(   rx+1*mSlW, mY,    mSlW, lh); slMapVelHi.setBounds(   rx+1*mSlW, mY+lh, mSlW, sh);
    lblMapBzThresh.setBounds(rx+2*mSlW, mY,    mSlW, lh); slMapBzThresh.setBounds(rx+2*mSlW, mY+lh, mSlW, sh);
    lblMapKpDens.setBounds(  rx+3*mSlW, mY,    mSlW, lh); slMapKpDens.setBounds(  rx+3*mSlW, mY+lh, mSlW, sh);
    // bottom=392+14+22=428

    // MacroOrb: y=436, 280×116, bottom=552
    macroOrb.setBounds(rx+40, 436, 280, 116);

    // OSC/MIDI: label at y=556(paint), buttons at y=570, dot at rx+226,574
    btnOSC.setBounds(    rx,      570, 52, 24);
    txtOSCPort.setBounds(rx+56,   570, 90, 24);
    btnMIDICC.setBounds( rx+150,  570, 68, 24);

    // ── Bottom strip (dynamic layout, 3 rows per section) ─────────────────
    const int TW   = getWidth();
    const int TH   = getHeight();
    const int lh3  = 14, sh3 = 24, gap3 = 4;
    const int rowH = lh3 + sh3 + gap3;  // 42px per row

    auto layoutREP = [&](int secTop, int secW, int secX, bool withProbDial) {
        const int r1L = secTop + 10;
        const int r1C = r1L + lh3;
        const int r2L = r1C + sh3 + gap3;
        const int r2C = r2L + lh3;
        const int r3L = r2C + sh3 + gap3;
        const int r3C = r3L + lh3;

        int bx0 = secX;
        if (withProbDial) {
            probDial.setBounds(bx0, r1L, 90, r3C + sh3 - r1L);
            bx0 += 94;
        }
        const int avail = secX + secW - bx0 - 4;

        // Row 1: ON | BARS | BPM
        int bx = bx0;
        btnRepOn.setBounds(bx, r1C, 52, sh3); bx += 56;
        const int cbW = 82;
        lblRepBars.setBounds(bx, r1L, cbW, lh3);
        cmbRepBars.setBounds(bx, r1C, cbW, sh3); bx += cbW + 4;
        const int bpmW = avail - 56 - cbW - 4;
        lblRepBPM.setBounds(bx, r1L, bpmW, lh3);
        slRepBPM.setBounds( bx, r1C, bpmW, sh3);

        // Row 2: FB | WET
        const int slW2 = avail / 2;
        int bx2 = bx0;
        lblRepFeedback.setBounds(bx2, r2L, slW2-2, lh3);
        slRepFeedback.setBounds( bx2, r2C, slW2-2, sh3); bx2 += slW2 + 2;
        lblRepWet.setBounds(bx2, r2L, slW2-2, lh3);
        slRepWet.setBounds( bx2, r2C, slW2-2, sh3);

        // Row 3: REV | PAN | STUTTER
        int bx3 = bx0;
        btnRepReverse.setBounds(bx3, r3C, 56, sh3); bx3 += 60;
        const int panW = avail - 48 - cbW - 4;
        lblRepPan.setBounds(bx3, r3L, panW, lh3);
        slRepPan.setBounds( bx3, r3C, panW, sh3); bx3 += panW + 4;
        lblRepStutter.setBounds(bx3, r3L, cbW, lh3);
        cmbRepStutter.setBounds(bx3, r3C, cbW, sh3);
        juce::ignoreUnused(bx, r3L);
    };

    auto layoutCHOP = [&](int secTop, int secW, int secX) {
        const int r1L = secTop + 10;
        const int r1C = r1L + lh3;
        const int r2L = r1C + sh3 + gap3;
        const int r2C = r2L + lh3;
        const int r3L = r2C + sh3 + gap3;
        const int r3C = r3L + lh3;
        const int avail = secW - 4;

        // Row 1: ON | SYNC | SHAPE | DIV
        int bx = secX;
        btnChopOn.setBounds(  bx, r1C, 56, sh3); bx += 60;
        btnChopSync.setBounds(bx, r1C, 48, sh3); bx += 52;
        const int cbW = std::max(70, (secX + avail - bx - 4) / 2 - 2);
        lblChopShape.setBounds(bx, r1L, cbW, lh3);
        cmbChopShape.setBounds(bx, r1C, cbW, sh3); bx += cbW + 4;
        lblChopDiv.setBounds(  bx, r1L, cbW, lh3);
        cmbChopDiv.setBounds(  bx, r1C, cbW, sh3);

        // Row 2: RATE | DEPTH
        const int slW = avail / 2;
        int bx2 = secX;
        lblChopRate.setBounds( bx2, r2L, slW-2, lh3);
        slChopRate.setBounds(  bx2, r2C, slW-2, sh3); bx2 += slW + 2;
        lblChopDepth.setBounds(bx2, r2L, slW-2, lh3);
        slChopDepth.setBounds( bx2, r2C, slW-2, sh3);

        // Row 3: ATK | REL | PHASE
        const int slW3 = avail / 3;
        int bx3 = secX;
        lblChopAttack.setBounds( bx3, r3L, slW3-2, lh3);
        slChopAttack.setBounds(  bx3, r3C, slW3-2, sh3); bx3 += slW3 + 2;
        lblChopRelease.setBounds(bx3, r3L, slW3-2, lh3);
        slChopRelease.setBounds( bx3, r3C, slW3-2, sh3); bx3 += slW3 + 2;
        lblChopPhase.setBounds(  bx3, r3L, slW3-2, lh3);
        slChopPhase.setBounds(   bx3, r3C, slW3-2, sh3);
        juce::ignoreUnused(r3L);
    };

    if (showVisual) {
        // 1350×780: REP left half | CHOP right half
        const int halfW = TW / 2;
        layoutREP( 600, halfW - 8,      8, true);
        layoutCHOP(600, TW - halfW - 8, halfW + 4);
    } else {
        // 690×840: REP top | CHOP bottom
        const int repH = 10 + rowH * 3 + 10;   // ~146px
        layoutREP( 600,       TW - 16, 8, true);
        layoutCHOP(600 + repH + 4, TW - 16, 8);
    }

    // HIDE/SHOW VISUAL: bottom-right corner
    btnToggleVisual.setBounds(TW - 158, TH - 32, 150, 26);
}
