#include "MappingCurveDisplay.h"
#include <cmath>
#include <algorithm>

void MappingCurveDisplay::setParams(float vLo, float vHi, float bzt, float kpd) {
    velLo = vLo; velHi = vHi; bzThresh = bzt; kpDens = kpd;
    repaint();
}
void MappingCurveDisplay::setLive(const LiveValues& v) { live = v; repaint(); }
void MappingCurveDisplay::setKp(float k) { currentKp = k; repaint(); }

void MappingCurveDisplay::drawCurve(juce::Graphics& g,
                                     juce::Rectangle<float> area,
                                     const juce::String& label,
                                     std::function<float(float)> fn,
                                     float liveInput) {
    const auto accent = ColourScheme::kpToAccent(currentKp);

    // Background
    g.setColour(ColourScheme::background(currentKp).withBrightness(0.12f));
    g.fillRoundedRectangle(area, 3.f);
    g.setColour(accent.withAlpha(0.15f));
    g.drawRoundedRectangle(area.reduced(0.5f), 3.f, 0.8f);

    const float margin = 4.f;
    auto inner = area.reduced(margin, margin + 6.f);
    inner.setY(area.getY() + margin);
    inner.setHeight(area.getHeight() - margin * 2 - 12.f);

    // Curve
    juce::Path p;
    const int N = 48;
    for (int i = 0; i <= N; ++i) {
        const float x = inner.getX() + inner.getWidth() * i / (float)N;
        const float y = inner.getBottom() - fn((float)i / N) * inner.getHeight();
        if (i == 0) p.startNewSubPath(x, y); else p.lineTo(x, y);
    }
    g.setColour(accent.withAlpha(0.75f));
    g.strokePath(p, juce::PathStrokeType(1.5f));

    // Live input indicator
    const float lx = inner.getX() + std::max(0.f, std::min(1.f, liveInput)) * inner.getWidth();
    g.setColour(accent.withAlpha(0.55f));
    g.drawVerticalLine((int)lx, inner.getY(), inner.getBottom());

    // Live dot on curve
    const float lyVal = fn(std::max(0.f, std::min(1.f, liveInput)));
    const float ly = inner.getBottom() - lyVal * inner.getHeight();
    g.setColour(accent);
    g.fillEllipse(lx - 2.5f, ly - 2.5f, 5.f, 5.f);

    // Label
    g.setFont(8.5f);
    g.setColour(juce::Colours::lightgrey.withAlpha(0.6f));
    g.drawText(label, (int)area.getX(), (int)(area.getBottom() - 12.f),
               (int)area.getWidth(), 12, juce::Justification::centred, false);
}

void MappingCurveDisplay::paint(juce::Graphics& g) {
    const float w = (float)getWidth(), h = (float)getHeight();
    const float cw = w / 2.f - 2.f, ch = h / 2.f - 2.f;

    // 2×2 grid of curves
    auto makeRect = [&](int col, int row) {
        return juce::Rectangle<float>(col * (w/2.f) + 1.f, row * (h/2.f) + 1.f, cw, ch);
    };

    // vel → pitch
    drawCurve(g, makeRect(0,0), "vel→pitch",
        [&](float x) {
            const float vLo = std::max(velLo, 1.f), vHi = std::max(velHi, vLo+1.f);
            return (std::pow(vHi/vLo, x) * vLo - vLo) / (vHi - vLo);
        },
        (live.velocity - 300.f) / 500.f);

    // Bz → timbre
    drawCurve(g, makeRect(1,0), "Bz→timbre",
        [&](float x) {
            // x=0 = Bz=0, x=1 = Bz=-30
            const float bz = -x * 30.f;
            const float t  = std::min(bzThresh, -0.1f);
            if (bz >= 0.f) return 0.f;
            if (bz > t) return (bz / t) * 0.3f;
            const float xr = std::max(0.f, std::min(1.f, (t - bz) / 7.f));
            return 0.3f + 0.7f * xr;
        },
        (-live.bz) / 30.f);

    // Kp → amplitude
    drawCurve(g, makeRect(0,1), "Kp→amp",
        [](float x) { return 0.1f + 0.9f * x; },
        live.kp / 9.f);

    // Kp → density
    drawCurve(g, makeRect(1,1), "Kp→dens",
        [&](float x) {
            const float kpVal = x * 9.f;
            const float kds   = std::max(0.f, std::min(8.f, kpDens));
            const float range = std::max(9.f - kds, 0.1f);
            if (kpVal < kds) return 0.f;
            const float xr = std::max(0.f, std::min(1.f, (kpVal - kds) / range));
            return xr * xr;
        },
        live.kp / 9.f);
}
