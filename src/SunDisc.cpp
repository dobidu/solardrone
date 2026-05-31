#include "SunDisc.h"
#include <algorithm>

SunDisc::SunDisc(juce::AudioProcessorValueTreeState& a) : apvts(a) {}

void SunDisc::setKp(float kp)          { currentKp   = kp;    repaint(); }
void SunDisc::setFlareLevel(float level){ flareLevel  = level;  repaint(); }

float SunDisc::getVolume()  const { return *apvts.getRawParameterValue("volume"); }
bool  SunDisc::getDroneOn() const { return *apvts.getRawParameterValue("drone_on") > 0.5f; }

void SunDisc::paint(juce::Graphics& g) {
    const float vol    = getVolume();
    const bool  on     = getDroneOn();
    const auto  accent = ColourScheme::kpToAccent(currentKp);
    const float w = (float)getWidth(), h = (float)getHeight();
    const float r = std::min(w, h) * 0.48f;
    const float cx = w * 0.5f, cy = h * 0.5f;

    // Outer glow ring (Kp severity colour)
    g.setColour(accent.withAlpha(on ? 0.85f : 0.25f));
    g.drawEllipse(cx - r, cy - r, r * 2.f, r * 2.f, 3.0f);

    // Volume arc (clockwise from bottom)
    if (on && vol > 0.01f) {
        const float two_pi = juce::MathConstants<float>::twoPi;
        const float start  = juce::MathConstants<float>::pi;
        const float end    = start + vol * two_pi;
        juce::Path arc;
        const float ar = r * 0.78f;
        arc.addArc(cx - ar, cy - ar, ar * 2.f, ar * 2.f, start, end, true);
        g.setColour(accent.withAlpha(0.40f));
        g.strokePath(arc, juce::PathStrokeType(5.0f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Inner disc fill
    const float ir = r * 0.62f;
    g.setColour(juce::Colour(0xff040810).withAlpha(on ? 0.95f : 0.65f));
    g.fillEllipse(cx - ir, cy - ir, ir * 2.f, ir * 2.f);

    // Flare corona ring
    if (flareLevel > 0.05f) {
        const float cr = r * (1.25f + flareLevel * 0.5f);
        g.setColour(juce::Colours::orange.withAlpha(flareLevel * 0.35f));
        g.drawEllipse(cx - cr, cy - cr, cr * 2.f, cr * 2.f, 2.5f);
        g.setColour(juce::Colours::white.withAlpha(flareLevel * 0.15f));
        g.drawEllipse(cx - cr*1.1f, cy - cr*1.1f, cr*2.2f, cr*2.2f, 1.0f);
    }

    // Symbol
    g.setFont(r * 0.38f);
    g.setColour(accent.withAlpha(on ? 0.90f : 0.28f));
    g.drawText(on ? "o" : "o", getLocalBounds(), juce::Justification::centred, false);

    // "VOL" micro-label
    g.setFont(8.0f);
    g.setColour(juce::Colours::white.withAlpha(0.25f));
    g.drawText("VOL", getLocalBounds().reduced(0, (int)(h * 0.5f - 12.f)),
               juce::Justification::centredBottom, false);
}

void SunDisc::mouseDown(const juce::MouseEvent& e) {
    dragStartY      = e.position.y;
    dragStartVolume = getVolume();
}

void SunDisc::mouseDrag(const juce::MouseEvent& e) {
    const float delta  = (dragStartY - e.position.y) / (float)getHeight() * 2.0f;
    const float newVol = std::max(0.f, std::min(1.f, dragStartVolume + delta));
    if (auto* p = apvts.getParameter("volume"))
        p->setValueNotifyingHost(p->convertTo0to1(newVol));
    repaint();
}

void SunDisc::mouseDoubleClick(const juce::MouseEvent&) {
    if (auto* p = apvts.getParameter("drone_on"))
        p->setValueNotifyingHost(getDroneOn() ? 0.f : 1.f);
}
