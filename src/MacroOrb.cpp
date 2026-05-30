#include "MacroOrb.h"
#include <algorithm>

const MacroOrb::Preset MacroOrb::kCorners[4] = {
    {0.0f, 0.5f, 0.0f, 0.0f},   // TL: Solar Wind
    {1.0f, 1.0f, 0.0f, 0.0f},   // TR: Geomagnetic
    {0.2f, 0.4f, 0.85f, 0.3f},  // BL: Ghost Loop
    {0.9f, 1.0f, 0.8f, 1.0f},   // BR: Storm Gate
};

static const char* kLabels[4] = {
    "Solar Wind", "Geomagnetic", "Ghost Loop", "Storm Gate"
};

MacroOrb::MacroOrb(juce::AudioProcessorValueTreeState& a) : apvts(a) {}

void MacroOrb::setKp(float kp) { currentKp = kp; repaint(); }

void MacroOrb::updateFromEvent(const juce::MouseEvent& e) {
    cursor.x = std::max(0.f, std::min(1.f, e.position.x / (float)getWidth()));
    cursor.y = std::max(0.f, std::min(1.f, e.position.y / (float)getHeight()));
    applyBlend();
    repaint();
}

void MacroOrb::mouseDown(const juce::MouseEvent& e) { updateFromEvent(e); }
void MacroOrb::mouseDrag(const juce::MouseEvent& e) { updateFromEvent(e); }

void MacroOrb::applyBlend() {
    const float x = cursor.x, y = cursor.y;
    const float wTL = (1-x)*(1-y), wTR = x*(1-y);
    const float wBL = (1-x)*y,     wBR = x*y;

    auto blend = [&](float tl, float tr, float bl, float br) {
        return tl*wTL + tr*wTR + bl*wBL + br*wBR;
    };
    auto setP = [&](const char* id, float v) {
        if (auto* p = apvts.getParameter(id))
            p->setValueNotifyingHost(p->convertTo0to1(v));
    };

    setP("layer_balance",  blend(kCorners[0].balance,   kCorners[1].balance,   kCorners[2].balance,   kCorners[3].balance));
    setP("dynamics_range", blend(kCorners[0].dynamics,  kCorners[1].dynamics,  kCorners[2].dynamics,  kCorners[3].dynamics));
    setP("repeater_wet",   blend(kCorners[0].repWet,    kCorners[1].repWet,    kCorners[2].repWet,    kCorners[3].repWet));
    setP("chopper_depth",  blend(kCorners[0].chopDepth, kCorners[1].chopDepth, kCorners[2].chopDepth, kCorners[3].chopDepth));
}

void MacroOrb::paint(juce::Graphics& g) {
    const float w = (float)getWidth(), h = (float)getHeight();
    const auto accent = ColourScheme::kpToAccent(currentKp);

    // Background
    g.setColour(ColourScheme::background(currentKp).withBrightness(0.10f));
    g.fillRoundedRectangle(0, 0, w, h, 6.f);

    // Border
    g.setColour(accent.withAlpha(0.20f));
    g.drawRoundedRectangle(0.5f, 0.5f, w-1.f, h-1.f, 6.f, 1.0f);

    // Crosshair
    g.setColour(accent.withAlpha(0.07f));
    g.drawHorizontalLine((int)(h * 0.5f), 0.f, w);
    g.drawVerticalLine(  (int)(w * 0.5f), 0.f, h);

    // Corner labels
    const juce::String labels[4] = {"Solar Wind","Geomagnetic","Ghost Loop","Storm Gate"};
    const juce::Justification just[4] = {
        juce::Justification::topLeft, juce::Justification::topRight,
        juce::Justification::bottomLeft, juce::Justification::bottomRight
    };
    const float lx[4] = {4.f, 4.f, 4.f, 4.f};
    const float ly[4] = {3.f, 3.f, h-13.f, h-13.f};
    const float lw[4] = {w-4.f, w-4.f, w-4.f, w-4.f};
    g.setFont(8.5f);
    g.setColour(accent.withAlpha(0.45f));
    for (int i = 0; i < 4; ++i)
        g.drawText(labels[i], (int)lx[i], (int)ly[i], (int)lw[i], 12, just[i], false);

    // Trail from center to cursor
    const float cx = cursor.x * w, cy = cursor.y * h;
    g.setColour(accent.withAlpha(0.15f));
    g.drawLine(w*0.5f, h*0.5f, cx, cy, 1.5f);

    // Cursor ring
    const float cr = 10.f;
    g.setColour(accent.withAlpha(0.5f));
    g.drawEllipse(cx - cr, cy - cr, cr*2.f, cr*2.f, 1.5f);

    // Cursor fill
    const float cfr = 5.f;
    g.setColour(accent.withAlpha(0.9f));
    g.fillEllipse(cx - cfr, cy - cfr, cfr*2.f, cfr*2.f);
}
