#include "ResonatorPanel.h"
#include <cmath>

void ResonatorPanel::setupSlider(juce::Slider& s, juce::Label& l,
                                  const juce::String& name, juce::Component* p) {
    s.setSliderStyle(juce::Slider::LinearHorizontal);
    s.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    s.setColour(juce::Slider::trackColourId, juce::Colour(0xff2a6fa8));
    s.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    p->addAndMakeVisible(s);
    l.setText(name, juce::dontSendNotification);
    l.setFont(juce::Font(11.f));
    l.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    l.setJustificationType(juce::Justification::centred);
    p->addAndMakeVisible(l);
}

ResonatorPanel::ResonatorPanel(juce::AudioProcessorValueTreeState& a) : apvts(a) {
    // Modal column
    btnModal.setButtonText("MODAL");
    btnModal.setColour(juce::ToggleButton::textColourId, juce::Colour(0xff88aaff));
    addAndMakeVisible(btnModal);
    setupSlider(slModalWet,   lblModalWet,   "Wet",   this);
    setupSlider(slModalDecay, lblModalDecay, "Decay", this);

    // FDN column
    btnFDN.setButtonText("FDN");
    btnFDN.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffffaa44));
    addAndMakeVisible(btnFDN);
    setupSlider(slFDNWet,  lblFDNWet,  "Wet",  this);
    setupSlider(slFDNSize, lblFDNSize, "Size", this);

    // Strings column
    btnStrings.setButtonText("STRINGS");
    btnStrings.setColour(juce::ToggleButton::textColourId, juce::Colour(0xff88ffcc));
    addAndMakeVisible(btnStrings);
    setupSlider(slStringsWet, lblStringsWet, "Wet",     this);
    setupSlider(slStringsN,   lblStringsN,   "Strings", this);

    // Attachments
    attModalOn   = std::make_unique<ButtonAttachment>(apvts, "res_modal_on",    btnModal);
    attModalWet  = std::make_unique<SliderAttachment>(apvts, "res_modal_wet",   slModalWet);
    attModalDecay= std::make_unique<SliderAttachment>(apvts, "res_modal_decay", slModalDecay);
    attFDNOn     = std::make_unique<ButtonAttachment>(apvts, "res_fdn_on",      btnFDN);
    attFDNWet    = std::make_unique<SliderAttachment>(apvts, "res_fdn_wet",     slFDNWet);
    attFDNSize   = std::make_unique<SliderAttachment>(apvts, "res_fdn_size",    slFDNSize);
    attStringsOn = std::make_unique<ButtonAttachment>(apvts, "res_strings_on",  btnStrings);
    attStringsWet= std::make_unique<SliderAttachment>(apvts, "res_strings_wet", slStringsWet);
    attStringsN  = std::make_unique<SliderAttachment>(apvts, "res_strings_n",   slStringsN);
}

void ResonatorPanel::setKp(float k) { currentKp = k; repaint(); }

void ResonatorPanel::setLiveData(float v, float t, float d, float p) {
    liveVel = v; liveTemp = t; liveDst = d; liveProton = p;
    repaint();
}

void ResonatorPanel::paint(juce::Graphics& g) {
    const float w = (float)getWidth(), h = (float)getHeight();
    const auto accent = ColourScheme::kpToAccent(currentKp);
    const float cw    = w / 3.f;
    const float stripH = 70.f;

    // Background
    g.setColour(ColourScheme::background(currentKp).withBrightness(0.09f));
    g.fillAll();

    // Column dividers
    g.setColour(accent.withAlpha(0.15f));
    g.drawVerticalLine((int)cw,       0.f, h - stripH);
    g.drawVerticalLine((int)(cw*2.f), 0.f, h - stripH);

    // Column headers
    const char* headers[3] = {"MODAL BANK", "FDN (8-tap)", "SYMPATH. STRINGS"};
    const char* drivers[3] = {
        "vel \xe2\x86\x92 decay  \xe2\x80\xa2  Kp \xe2\x86\x92 modes",
        "Dst \xe2\x86\x92 T60  \xe2\x80\xa2  pressure \xe2\x86\x92 cutoff",
        "proton flux \xe2\x86\x92 damping"
    };
    // Use ASCII fallbacks for headers to avoid JUCE String assertions
    const char* headersASCII[3] = {"MODAL BANK", "FDN (8-tap)", "SYMPATH. STRINGS"};
    const char* driversASCII[3] = {
        "vel -> decay  |  Kp -> modes",
        "Dst -> T60  |  pressure -> cutoff",
        "proton flux -> damping"
    };
    for (int col = 0; col < 3; ++col) {
        const float cx = cw * col;
        g.setFont(13.f);
        g.setColour(accent.withAlpha(0.85f));
        g.drawText(headersASCII[col], (int)(cx+8), 10, (int)(cw-16), 18,
                   juce::Justification::centred, false);
        g.setFont(9.f);
        g.setColour(accent.withAlpha(0.42f));
        g.drawText(driversASCII[col], (int)(cx+6), 32, (int)(cw-12), 14,
                   juce::Justification::centred, false);
    }

    // Live data strip
    const float sy = h - stripH;
    g.setColour(juce::Colour(0xff060c18));
    g.fillRect(0.f, sy, w, stripH);
    g.setColour(accent.withAlpha(0.18f));
    g.drawHorizontalLine((int)sy, 0.f, w);

    g.setFont(10.f);
    g.setColour(accent.withAlpha(0.5f));
    g.drawText("SOLAR DRIVERS (LIVE)", 8, (int)sy+4, 200, 14,
               juce::Justification::left, false);

    // 4 data boxes
    const int boxW = (int)(w / 4.f) - 4;
    const int bsy  = (int)sy + 22;
    struct { const char* label; float val; const char* unit; bool negative; } boxes[4] = {
        {"velocity",    liveVel,    "km/s",  false},
        {"temperature", liveTemp/1000.f, "kK", false},
        {"Dst",         liveDst,    "nT",    true},
        {"proton",      liveProton, "pfu",   false},
    };
    for (int i = 0; i < 4; ++i) {
        const int bx = 4 + i * ((int)(w/4.f));
        juce::Colour boxCol = accent;
        if (i == 2 && liveDst < -50.f) boxCol = juce::Colours::orangered;
        if (i == 3 && liveProton > 10.f) boxCol = juce::Colours::orange;
        g.setColour(boxCol.withAlpha(0.12f));
        g.fillRoundedRectangle((float)bx, (float)bsy, (float)boxW, 36.f, 4.f);
        g.setColour(boxCol.withAlpha(0.45f));
        g.drawRoundedRectangle((float)bx+0.5f, (float)bsy+0.5f,
                                (float)boxW-1.f, 35.f, 4.f, 0.8f);
        g.setFont(9.f);
        g.setColour(juce::Colours::lightgrey.withAlpha(0.5f));
        g.drawText(boxes[i].label, bx, bsy+2, boxW, 12, juce::Justification::centred, false);
        g.setFont(13.f);
        g.setColour(boxCol.withAlpha(0.9f));
        juce::String valStr = (std::abs(boxes[i].val) < 10.f)
            ? juce::String(boxes[i].val, 2)
            : juce::String((int)boxes[i].val);
        g.drawText(valStr + " " + boxes[i].unit, bx, bsy+14, boxW, 18,
                   juce::Justification::centred, false);
    }
}

void ResonatorPanel::resized() {
    const float w  = (float)getWidth();
    const float h  = (float)getHeight();
    const float cw = w / 3.f;
    const float stripH = 70.f;
    const float ctH = h - stripH;
    const int lh = 14, sh = 22, pad = 8;

    auto placeCol = [&](juce::ToggleButton& btn,
                        juce::Slider& s1, juce::Label& l1,
                        juce::Slider& s2, juce::Label& l2, int col) {
        const int x  = (int)(cw * col) + (int)pad;
        const int w2 = (int)cw - (int)(pad * 2);
        btn.setBounds(x, 52, w2, 24);
        l1.setBounds( x, 90,  w2, lh);
        s1.setBounds( x, 90+lh, w2, sh);
        l2.setBounds( x, 90+lh+sh+4, w2, lh);
        s2.setBounds( x, 90+lh+sh+4+lh, w2, sh);
    };

    placeCol(btnModal,   slModalWet,   lblModalWet,   slModalDecay, lblModalDecay, 0);
    placeCol(btnFDN,     slFDNWet,     lblFDNWet,     slFDNSize,    lblFDNSize,    1);
    placeCol(btnStrings, slStringsWet, lblStringsWet, slStringsN,   lblStringsN,   2);
    juce::ignoreUnused(ctH);
}
