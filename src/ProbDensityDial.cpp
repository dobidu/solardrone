#include "ProbDensityDial.h"
#include <cmath>
#include <algorithm>

ProbDensityDial::ProbDensityDial(juce::AudioProcessorValueTreeState& a)
    : apvts(a) {
    regenerateDots();
    startTimerHz(2);
}

ProbDensityDial::~ProbDensityDial() { stopTimer(); }

float ProbDensityDial::getDensity() const {
    return *apvts.getRawParameterValue("repeater_density");
}

void ProbDensityDial::regenerateDots() {
    const float d = getDensity();
    auto& rng = juce::Random::getSystemRandom();
    for (int i = 0; i < 16; ++i)
        dots[i] = (rng.nextFloat() < d);
}

void ProbDensityDial::timerCallback() { regenerateDots(); repaint(); }
void ProbDensityDial::setKp(float kp) { currentKp = kp; repaint(); }

void ProbDensityDial::mouseDown(const juce::MouseEvent& e) {
    dragStartY       = e.position.y;
    dragStartDensity = getDensity();
}

void ProbDensityDial::mouseDrag(const juce::MouseEvent& e) {
    const float delta = (dragStartY - e.position.y) / (float)getHeight() * 1.5f;
    const float newD  = std::max(0.f, std::min(1.f, dragStartDensity + delta));
    if (auto* p = apvts.getParameter("repeater_density"))
        p->setValueNotifyingHost(p->convertTo0to1(newD));
    regenerateDots();
    repaint();
}

void ProbDensityDial::paint(juce::Graphics& g) {
    const float w = (float)getWidth(), h = (float)getHeight();
    const float cx = w * 0.5f, cy = h * 0.5f;
    const auto  accent = ColourScheme::kpToAccent(currentKp);

    // Background
    g.setColour(ColourScheme::background(currentKp).withBrightness(0.10f));
    g.fillRoundedRectangle(0, 0, w, h, 4.f);
    g.setColour(accent.withAlpha(0.18f));
    g.drawRoundedRectangle(0.5f, 0.5f, w-1.f, h-1.f, 4.f, 1.0f);

    // 16 dots in ring
    const float ringR = std::min(cx, cy) * 0.72f;
    const float dotR  = 4.0f;
    const float twoPi = juce::MathConstants<float>::twoPi;
    for (int i = 0; i < 16; ++i) {
        const float angle = twoPi * i / 16.f - juce::MathConstants<float>::halfPi;
        const float dx = cx + ringR * std::cos(angle);
        const float dy = cy + ringR * std::sin(angle);
        g.setColour(dots[i]
            ? accent.withAlpha(0.90f)
            : accent.withAlpha(0.10f));
        g.fillEllipse(dx - dotR, dy - dotR, dotR * 2.f, dotR * 2.f);
    }

    // Density text in center
    const float d = getDensity();
    g.setFont(9.0f);
    g.setColour(accent.withAlpha(0.7f));
    g.drawText(juce::String((int)(d * 100)) + "%",
               getLocalBounds(), juce::Justification::centred, false);

    // Label
    g.setFont(8.0f);
    g.setColour(juce::Colours::lightgrey.withAlpha(0.5f));
    g.drawText("DENS", getLocalBounds().removeFromBottom(12),
               juce::Justification::centred, false);
}
