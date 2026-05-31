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

    addAndMakeVisible(spatialDisplay);
    addAndMakeVisible(probDial);
    addAndMakeVisible(macroOrb);
    addAndMakeVisible(visualRenderer);
    addAndMakeVisible(sunDisc);

    startTimerHz(10);
    setSize(1050, 760);
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
    for (int x = 0; x < 1050; x += 40) g.drawVerticalLine(x, 0.f, 760.f);
    for (int y = 0; y < 760; y += 40) g.drawHorizontalLine(y, 0.f, 1050.f);

    // Right panel tint
    g.setColour(accent.withAlpha(0.04f));
    g.fillRect(662, 0, 388, 600);

    // Bottom strip (160px)
    g.setColour(juce::Colour(0xff060c18));
    g.fillRect(0, 600, 1050, 160);

    // Dividers
    g.setColour(accent.withAlpha(0.18f));
    g.drawVerticalLine(661, 0.f, 601.f);
    g.drawHorizontalLine(599, 0.f, 1050.f);
    g.drawVerticalLine(522, 600.f, 760.f);  // REP | CHOP divider

    // Section labels
    g.setFont(12.0f);
    g.setColour(accent.withAlpha(0.65f));
    g.drawText("REPEATER", 8,   602, 90, 14, juce::Justification::left, false);
    g.drawText("CHOPPER",  528, 602, 90, 14, juce::Justification::left, false);

    // Right panel title
    g.setFont(11.0f);
    g.setColour(accent.withAlpha(0.4f));
    g.drawText("PARAMETERS", 670, 4, 340, 14, juce::Justification::left, false);

    // OSC/MIDI activity indicator
    if (oscActivityAlpha > 0.01f) {
        g.setColour(juce::Colours::cyan.withAlpha(oscActivityAlpha));
        g.fillEllipse(1034, 596, 8, 8);
    }
    g.setFont(9.0f);
    g.setColour(accent.withAlpha(0.4f));
    g.drawText("OSC / MIDI", 670, 580, 70, 14, juce::Justification::left, false);
}

void SolarDroneAudioProcessorEditor::resized() {
    // ── Visual zone (660×600) ─────────────────────────────────────────────
    visualRenderer.setBounds(0, 0, 660, 600);
    sunDisc.setBounds(260, 230, 140, 140);
    btnFreeze.setBounds(548, 6, 100, 24);

    // ── Right panel (rx=670, rw=360, height=600) ──────────────────────────
    const int rx = 670, lh = 14, sh = 22, pad = 6, rw = 360;
    const int colW = (rw - pad) / 2;

    // Row 0: Harmony combo (100px) + Interval slider
    cmbHarmony.setBounds(rx, 18, 100, sh);
    lblInterval.setBounds(rx + 108, 18,      rw - 108, lh);
    slInterval.setBounds( rx + 108, 18 + lh, rw - 108, sh);

    // Rows 1-4: sliders in pairs
    auto placeRow = [&](juce::Slider& sL, juce::Label& lL,
                        juce::Slider& sR, juce::Label& lR, int y) {
        lL.setBounds(rx,            y, colW, lh);
        sL.setBounds(rx,            y+lh, colW, sh);
        lR.setBounds(rx+colW+pad,   y, colW, lh);
        sR.setBounds(rx+colW+pad,   y+lh, colW, sh);
    };
    placeRow(slGlide,   lblGlide,   slDynamics, lblDynamics, 60);
    placeRow(slBalance, lblBalance, slSpread,   lblSpread,   106);
    placeRow(slVisLiss, lblVisLiss, slVisPart,  lblVisPart,  152);
    lblVisSpec.setBounds(rx, 198, colW, lh);
    slVisSpec.setBounds( rx, 198+lh, colW, sh);

    // Spatial display (150×96 centered, y=234)
    spatialDisplay.setBounds(rx + 105, 234, 150, 96);

    // Mapping curves (y=342, full width, 90px tall)
    mappingDisplay.setBounds(rx, 342, rw, 90);
    // 4 mapping sliders below (y=436)
    const int mSlW = rw / 4;
    const int mY   = 436;
    lblMapVelLo.setBounds(   rx+0*mSlW, mY,    mSlW, lh);
    slMapVelLo.setBounds(    rx+0*mSlW, mY+lh, mSlW, sh);
    lblMapVelHi.setBounds(   rx+1*mSlW, mY,    mSlW, lh);
    slMapVelHi.setBounds(    rx+1*mSlW, mY+lh, mSlW, sh);
    lblMapBzThresh.setBounds(rx+2*mSlW, mY,    mSlW, lh);
    slMapBzThresh.setBounds( rx+2*mSlW, mY+lh, mSlW, sh);
    lblMapKpDens.setBounds(  rx+3*mSlW, mY,    mSlW, lh);
    slMapKpDens.setBounds(   rx+3*mSlW, mY+lh, mSlW, sh);

    // MacroOrb (y=476, 280×110)
    macroOrb.setBounds(rx + 40, 476, 280, 110);

    // OSC/MIDI section (y=594)
    btnOSC.setBounds(    rx,       594, 52, 20);
    txtOSCPort.setBounds(rx + 56,  594, 90, 20);
    btnMIDICC.setBounds( rx + 150, 594, 68, 20);

    // ── Bottom strip (160px, y=600-760) ──────────────────────────────────
    const int stripTop = 600;
    const int lbH = 14, ctH = 26, gap2 = 5;
    const int row1L = stripTop + 16;
    const int row1C = row1L + lbH;
    const int row2L = row1C + ctH + gap2;
    const int row2C = row2L + lbH;

    // Left half REPEATER: x=8 to x=520
    {
        int bx = 8;
        const int dialSz = 80;
        probDial.setBounds(bx, stripTop + 4, dialSz, dialSz); bx += dialSz + 6;

        btnRepOn.setBounds(bx, row1C, 56, ctH); bx += 60;

        const int comboW = 90;
        lblRepBars.setBounds(bx, row1L, comboW, lbH);
        cmbRepBars.setBounds(bx, row1C, comboW, ctH); bx += comboW + 6;

        const int bpmW = 130;
        lblRepBPM.setBounds(bx, row1L, bpmW, lbH);
        slRepBPM.setBounds( bx, row1C, bpmW, ctH); bx += bpmW + 6;

        int bx2 = 8 + dialSz + 6 + 60 + comboW + 6;
        const int slW = (515 - bx2) / 2;
        lblRepFeedback.setBounds(bx2, row2L, slW, lbH);
        slRepFeedback.setBounds( bx2, row2C, slW, ctH); bx2 += slW + 6;
        lblRepWet.setBounds(bx2, row2L, slW, lbH);
        slRepWet.setBounds( bx2, row2C, slW, ctH);
    }

    // Right half CHOPPER: x=528 to x=1042
    {
        int bx = 528;
        btnChopOn.setBounds(  bx, row1C, 62, ctH); bx += 66;
        btnChopSync.setBounds(bx, row1C, 52, ctH); bx += 56;

        const int comboW = 88;
        lblChopShape.setBounds(bx, row1L, comboW, lbH);
        cmbChopShape.setBounds(bx, row1C, comboW, ctH); bx += comboW + 6;
        lblChopDiv.setBounds(  bx, row1L, comboW, lbH);
        cmbChopDiv.setBounds(  bx, row1C, comboW, ctH); bx += comboW + 6;

        const int chopSlW = (1040 - bx) / 2;
        lblChopRate.setBounds( bx,            row2L, chopSlW, lbH);
        slChopRate.setBounds(  bx,            row2C, chopSlW, ctH); bx += chopSlW + 6;
        lblChopDepth.setBounds(bx,            row2L, chopSlW, lbH);
        slChopDepth.setBounds( bx,            row2C, chopSlW, ctH);
    }
}
