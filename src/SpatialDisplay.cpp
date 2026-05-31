#include "SpatialDisplay.h"
#include <cmath>
#include <algorithm>

const SpatialDisplay::Layer SpatialDisplay::kLayers[3] = {
    {"l1_azimuth","l1_elevation", juce::Colour(0xff4488ff), "L1"},
    {"l2_azimuth","l2_elevation", juce::Colours::orange,    "L2"},
    {"l3_azimuth","l3_elevation", juce::Colours::red,       "L3"},
};

SpatialDisplay::SpatialDisplay(juce::AudioProcessorValueTreeState& a) : apvts(a) {}

void SpatialDisplay::setKp(float k)         { currentKp  = k; repaint(); }
void SpatialDisplay::setFlareLevel(float l) { flareLevel = l; repaint(); }

// azimuth [-90,90] → x [0.1*w, 0.9*w]; elevation [-45,45] → y [0.9*h, 0.1*h]
juce::Point<float> SpatialDisplay::dotPos(int i) const {
    const float az = *apvts.getRawParameterValue(kLayers[i].azId);
    const float el = *apvts.getRawParameterValue(kLayers[i].elId);
    const float w  = (float)getWidth(), h = (float)getHeight();
    return { 0.5f * w + (az / 90.f) * 0.38f * w,
             0.5f * h - (el / 45.f) * 0.38f * h };
}

int SpatialDisplay::hitTest(juce::Point<float> p) const {
    for (int i = 0; i < 3; ++i)
        if (dotPos(i).getDistanceFrom(p) < 12.f) return i;
    return -1;
}

void SpatialDisplay::applyPos(int i, juce::Point<float> p) {
    const float w = (float)getWidth(), h = (float)getHeight();
    const float az = std::max(-90.f, std::min(90.f, (p.x - w*0.5f) / (0.38f*w) * 90.f));
    const float el = std::max(-45.f, std::min(45.f, -(p.y - h*0.5f) / (0.38f*h) * 45.f));
    if (auto* pa = apvts.getParameter(kLayers[i].azId))
        pa->setValueNotifyingHost(pa->convertTo0to1(az));
    if (auto* pe = apvts.getParameter(kLayers[i].elId))
        pe->setValueNotifyingHost(pe->convertTo0to1(el));
    repaint();
}

void SpatialDisplay::mouseDown(const juce::MouseEvent& e) {
    dragLayer = hitTest(e.position);
    if (dragLayer >= 0) applyPos(dragLayer, e.position);
}
void SpatialDisplay::mouseDrag(const juce::MouseEvent& e) {
    if (dragLayer >= 0) applyPos(dragLayer, e.position);
}
void SpatialDisplay::mouseUp(const juce::MouseEvent&) { dragLayer = -1; }

void SpatialDisplay::paint(juce::Graphics& g) {
    const float w = (float)getWidth(), h = (float)getHeight();
    const auto accent = ColourScheme::kpToAccent(currentKp);

    // Background
    g.setColour(ColourScheme::background(currentKp).withBrightness(0.10f));
    g.fillRoundedRectangle(0, 0, w, h, 4.f);
    g.setColour(accent.withAlpha(0.15f));
    g.drawRoundedRectangle(0.5f, 0.5f, w-1.f, h-1.f, 4.f, 0.8f);

    // Head oval
    const float ow = w * 0.78f, oh = h * 0.72f;
    const float ox = (w-ow)*0.5f, oy = (h-oh)*0.5f;
    g.setColour(accent.withAlpha(0.10f));
    g.fillEllipse(ox, oy, ow, oh);
    g.setColour(accent.withAlpha(0.25f));
    g.drawEllipse(ox, oy, ow, oh, 1.0f);

    // Center crosshair
    g.setColour(accent.withAlpha(0.08f));
    g.drawHorizontalLine((int)(h*0.5f), ox, ox+ow);
    g.drawVerticalLine(  (int)(w*0.5f), oy, oy+oh);

    // L/R labels
    g.setFont(8.0f);
    g.setColour(juce::Colours::lightgrey.withAlpha(0.4f));
    g.drawText("L", (int)(ox-12), (int)(h*0.5f-6), 12, 12, juce::Justification::right, false);
    g.drawText("R", (int)(ox+ow+2), (int)(h*0.5f-6), 12, 12, juce::Justification::left, false);
    g.drawText("^", (int)(w*0.5f-4), (int)(oy-12), 8, 12, juce::Justification::centred, false);

    // Layer dots
    for (int i = 0; i < 3; ++i) {
        const auto pos = dotPos(i);
        const float r = (i == 2 && flareLevel > 0.1f)
                        ? 6.f + flareLevel * 4.f   // L3 pulsates with flare
                        : 6.f;
        g.setColour(kLayers[i].col.withAlpha(0.85f));
        g.fillEllipse(pos.x - r, pos.y - r, r*2.f, r*2.f);
        g.setColour(kLayers[i].col.withAlpha(0.4f));
        g.drawEllipse(pos.x - r - 2.f, pos.y - r - 2.f, (r+2.f)*2.f, (r+2.f)*2.f, 1.0f);
        // Label
        g.setFont(7.5f);
        g.setColour(juce::Colours::white.withAlpha(0.75f));
        g.drawText(kLayers[i].label, (int)(pos.x-8), (int)(pos.y-r-11), 16, 10,
                   juce::Justification::centred, false);
    }
}
