#include "VisualRenderer.h"
#include "PluginProcessor.h"
#include "SpaceWeatherState.h"
#include "ColourScheme.h"
#include <cmath>
#include <algorithm>

static constexpr int kMaxParticles = 80;

VisualRenderer::VisualRenderer(SolarDroneAudioProcessor* proc)
    : processor(proc) {
    particles.reserve(kMaxParticles);
    startTimerHz(30);
}

VisualRenderer::~VisualRenderer() {
    stopTimer();
}

void VisualRenderer::setSynthParams(const SynthParams& p, const UserParams& u) {
    juce::ScopedLock sl(paramsLock);
    displayParams     = p;
    displayUserParams = u;
}

void VisualRenderer::timerCallback() {
    // Poll processor for latest smoothed params + weather state
    if (processor != nullptr) {
        auto p = processor->getCurrentSynthParams();
        UserParams u;
        setSynthParams(p, u);
        {
            juce::ScopedLock sl(paramsLock);
            cachedWeatherState = processor->getLatestSpaceWeatherState();
            bzHistory[bzHistoryIdx % 120] = cachedWeatherState.bz_gsm;
            ++bzHistoryIdx;
        }

        // Freeze edge detection
        const bool nowFrozen =
            *processor->apvts.getRawParameterValue("freeze_on") > 0.5f;
        if (nowFrozen && !wasFreeze) {
            frozenSnapshot = createComponentSnapshot(getLocalBounds(), 1.0f);
        }
        wasFreeze = nowFrozen;
        frozen    = nowFrozen;
    }

    // Compute delta time
    auto nowMs = juce::Time::currentTimeMillis();
    float dt = (lastFrameMs > 0)
        ? (float)(nowMs - lastFrameMs) / 1000.0f
        : 0.033f;
    lastFrameMs = nowMs;
    dt = std::min(dt, 0.1f);

    // Update animation state
    SynthParams p;
    { juce::ScopedLock sl(paramsLock); p = displayParams; }

    lissPhase += (double)(p.l1_timbre * 0.4f + 0.06f) * dt;
    updateParticles(p, dt);

    repaint();
}

// ── Lissajous ────────────────────────────────────────────────────────────────

void VisualRenderer::drawLissajous(juce::Graphics& g,
                                    const SynthParams& p, float blend) {
    if (blend < 0.01f) return;
    const float w = (float)getWidth();
    const float h = (float)getHeight();
    const float cx = w * 0.5f, cy = h * 0.5f;
    const float rx = w * 0.44f, ry = h * 0.44f;

    // Use normalized ratio from L1/L2 fundamentals (clamped for aesthetics)
    double ratio = (double)(p.l2_fundamental_hz / p.l1_fundamental_hz);
    ratio = std::max(0.5, std::min(ratio, 4.0));

    // Color from timbre: 0=blue, 0.5=teal, 1=red
    const float hue = 0.6f - p.l1_timbre * 0.6f;
    const float alpha = blend * 0.85f;
    g.setColour(juce::Colour::fromHSV(hue, 0.8f, 1.0f, alpha));

    const int N = 512;
    const double step = juce::MathConstants<double>::twoPi * 2.0 / N;
    juce::Path path;

    for (int i = 0; i < N; ++i) {
        const double t = i * step;
        const float x = cx + (float)std::sin(t)           * rx;
        const float y = cy + (float)std::sin(ratio * t + lissPhase) * ry;
        if (i == 0) path.startNewSubPath(x, y);
        else        path.lineTo(x, y);
    }
    path.closeSubPath();

    const float thickness = 0.8f + p.l1_amplitude * 1.5f;
    g.strokePath(path, juce::PathStrokeType(thickness));
}

// ── Particles ────────────────────────────────────────────────────────────────

void VisualRenderer::updateParticles(const SynthParams& p, float dt) {
    auto& rng = juce::Random::getSystemRandom();

    // Target count from l2_harmonic_density
    const int targetCount = 5 + (int)(45.0f * p.l2_harmonic_density);
    const float speed     = 0.05f + p.l1_amplitude * 0.25f;
    const float turb      = p.l1_timbre * 0.8f;
    const float hue       = 0.6f - p.l2_brightness * 0.5f;  // violet → orange
    const float lifetime  = 3.0f + p.l2_amplitude * 4.0f;

    // Update existing particles
    for (auto& par : particles) {
        par.vx += (rng.nextFloat() - 0.5f) * turb * dt * 2.0f;
        par.vy += (rng.nextFloat() - 0.5f) * turb * dt * 2.0f;
        // Dampen velocity
        par.vx *= (1.0f - dt * 0.5f);
        par.vy *= (1.0f - dt * 0.5f);
        par.x  += par.vx * dt;
        par.y  += par.vy * dt;
        par.life -= dt / par.maxLife;
    }
    // Remove dead particles
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& par) { return par.life <= 0.0f; }),
        particles.end());

    // Spawn new particles
    spawnAccum += (float)targetCount * dt;
    while (spawnAccum >= 1.0f && (int)particles.size() < kMaxParticles) {
        Particle par;
        par.x       = rng.nextFloat();
        par.y       = rng.nextFloat();
        par.vx      = (rng.nextFloat() - 0.5f) * speed;
        par.vy      = (rng.nextFloat() - 0.5f) * speed;
        par.life    = 1.0f;
        par.maxLife = lifetime * (0.5f + rng.nextFloat() * 0.5f);
        par.hue     = hue + (rng.nextFloat() - 0.5f) * 0.1f;
        par.size    = 1.5f + rng.nextFloat() * 3.5f;
        particles.push_back(par);
        spawnAccum -= 1.0f;
    }
    if (spawnAccum > (float)targetCount) spawnAccum = 0.0f;
}

void VisualRenderer::drawParticles(juce::Graphics& g,
                                    const SynthParams& /*p*/, float blend) {
    if (blend < 0.01f || particles.empty()) return;
    const float w = (float)getWidth();
    const float h = (float)getHeight();

    for (const auto& par : particles) {
        if (par.life <= 0.0f) continue;
        const float alpha = par.life * blend * 0.8f;
        g.setColour(juce::Colour::fromHSV(par.hue, 0.7f, 1.0f, alpha));
        const float px = par.x * w;
        const float py = par.y * h;
        const float sz = par.size * blend;
        g.fillEllipse(px - sz * 0.5f, py - sz * 0.5f, sz, sz);
    }
}

// ── Spectral ─────────────────────────────────────────────────────────────────

void VisualRenderer::drawSpectral(juce::Graphics& g,
                                   const SynthParams& p, float blend) {
    if (blend < 0.01f) return;
    const float w  = (float)getWidth();
    const float h  = (float)getHeight();
    const float barH = h * 0.28f;  // max bar height = 28% of window height
    const float baseY = h;          // bars grow upward from bottom

    // L1 harmonic bars
    const int n1 = std::max(2, std::min(p.l1_harmonic_count, 24));
    const float spacing = w / (float)(n1 + 1);
    const float hue1    = 0.6f - p.l1_timbre * 0.6f;

    for (int i = 0; i < n1; ++i) {
        const float harmonic = (float)(i + 1);
        const float amp    = (p.l1_amplitude / harmonic) * blend;
        const float barHi  = amp * barH;
        // Timbre warps bar position slightly
        const float warp   = p.l1_timbre * 4.0f * (float)i;
        const float bx     = spacing * (float)(i + 1) + warp;
        const float bw     = std::max(1.5f, spacing * 0.5f);
        const float alpha  = blend * 0.7f;
        g.setColour(juce::Colour::fromHSV(hue1, 0.6f + p.l1_timbre * 0.3f, 1.0f, alpha));
        g.fillRect(bx - bw * 0.5f, baseY - barHi, bw, barHi);
    }

    // L2 overlay — thin lines from density
    const int n2 = 2 + (int)(22.0f * p.l2_harmonic_density);
    const float hue2 = 0.1f + (1.0f - p.l2_brightness) * 0.3f;  // orange→yellow
    const float spacing2 = w / (float)(n2 + 1);

    for (int i = 0; i < n2; ++i) {
        const float amp  = (p.l2_amplitude / (float)(i + 1))
                           * (1.0f + (float)i * p.l2_brightness * 0.3f);
        const float barHi = std::min(amp * barH * 0.6f, barH);
        const float bx    = spacing2 * (float)(i + 1);
        const float alpha = blend * p.l2_harmonic_density * 0.5f;
        g.setColour(juce::Colour::fromHSV(hue2, 0.8f, 1.0f, alpha));
        g.fillRect(bx - 1.0f, baseY - barHi, 2.0f, barHi);
    }
}

// ── paint ────────────────────────────────────────────────────────────────────

void VisualRenderer::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);

    SynthParams p; UserParams u;
    { juce::ScopedLock sl(paramsLock); p = displayParams; u = displayUserParams; }

    // Draw layers: spectral (background) → particles (mid) → lissajous (top)
    if (u.visual_spectral   > 0.01f) drawSpectral(g,  p, u.visual_spectral);
    if (u.visual_particles  > 0.01f) drawParticles(g, p, u.visual_particles);
    if (u.visual_lissajous  > 0.01f) drawLissajous(g, p, u.visual_lissajous);

    // Read cached weather state once for data-layer elements
    SpaceWeatherState ws;
    { juce::ScopedLock sl(paramsLock); ws = cachedWeatherState; }

    // ── Degraded-signal texture ───────────────────────────────────────────
    {
        const int age = ws.data_age_s;
        if (age > 60) {
            const float deg = juce::jlimit(0.f, 1.f, (age - 60) / 180.f);
            g.setColour(juce::Colours::black.withAlpha(deg * 0.32f));
            for (int sy = 0; sy < getHeight(); sy += 3)
                g.drawHorizontalLine(sy, 0.f, (float)getWidth());
        }
    }

    // ── Bz spark-line (bottom 8px of visual) ─────────────────────────────
    {
        const float sparkH = 8.f, sparkY = (float)getHeight() - sparkH - 1.f;
        const float sparkW = (float)getWidth();
        g.setColour(juce::Colours::black.withAlpha(0.55f));
        g.fillRect(0.f, sparkY, sparkW, sparkH);
        juce::Path sparkPath;
        const int nS = 120;
        for (int i = 0; i < nS; ++i) {
            const int idx  = (bzHistoryIdx - nS + i + 120) % 120;
            const float bz = bzHistory[idx];
            const float x  = sparkW * i / (float)nS;
            const float n  = juce::jlimit(-1.f, 1.f, bz / 30.f);
            const float y  = sparkY + sparkH * 0.5f - n * sparkH * 0.42f;
            if (i == 0) sparkPath.startNewSubPath(x, y);
            else        sparkPath.lineTo(x, y);
        }
        const float latestBz = bzHistory[(bzHistoryIdx - 1 + 120) % 120];
        g.setColour(latestBz < 0
            ? juce::Colours::orangered.withAlpha(0.8f)
            : ColourScheme::kpToAccent(ws.kp).withAlpha(0.65f));
        g.strokePath(sparkPath, juce::PathStrokeType(1.2f));
    }

    // ── Kp severity badge (9 squares, above spark-line) ──────────────────
    {
        const float sqSz = 10.f, sqGap = 2.f;
        const float badgeW = 9 * (sqSz + sqGap);
        float bx = (float)getWidth() - badgeW - 4.f;
        const float by = (float)getHeight() - 23.f;
        for (int i = 0; i < 9; ++i) {
            const bool lit = (float)(i + 1) <= ws.kp;
            juce::Colour sqCol;
            if      (i < 3) sqCol = juce::Colours::limegreen;
            else if (i < 5) sqCol = juce::Colours::yellow;
            else if (i < 7) sqCol = juce::Colours::orange;
            else            sqCol = juce::Colours::red;
            g.setColour(lit ? sqCol.withAlpha(0.85f) : sqCol.withAlpha(0.12f));
            g.fillRect(bx, by, sqSz, sqSz);
            bx += sqSz + sqGap;
        }
    }

    // Freeze ghost overlay — drawn before status text
    if (frozen && frozenSnapshot.isValid()) {
        g.setOpacity(0.25f);
        g.drawImage(frozenSnapshot, getLocalBounds().toFloat());
        g.setOpacity(1.0f);
        g.setFont(10.0f);
        g.setColour(juce::Colours::cyan.withAlpha(0.75f));
        g.drawText("FROZEN",
                   getLocalBounds().reduced(8).removeFromTop(18),
                   juce::Justification::topLeft, false);
    }

    // Status overlay — drawn last so it appears above all visual layers
    SpaceWeatherState state;
    { juce::ScopedLock sl(paramsLock); state = cachedWeatherState; }

    juce::String sourceStr;
    juce::Colour statusCol;
    switch (state.source) {
        case SpaceWeatherState::Source::live:
            sourceStr = "live";    statusCol = juce::Colours::limegreen; break;
        case SpaceWeatherState::Source::cached:
            sourceStr = "cached";  statusCol = juce::Colours::yellow;    break;
        default:
            sourceStr = "default"; statusCol = juce::Colours::grey;      break;
    }
    juce::String bzSign = (state.bz_gsm >= 0.0f) ? "+" : "";
    juce::String statusText = sourceStr
        + "  v=" + juce::String((int)state.velocity) + "km/s"
        + "  Bz=" + bzSign + juce::String(state.bz_gsm, 1)
        + "  Kp=" + juce::String(state.kp, 1)
        + "  age=" + juce::String(state.data_age_s) + "s";

    g.setFont(10.0f);
    g.setColour(statusCol.withAlpha(0.75f));
    g.drawText(statusText,
               getLocalBounds().reduced(8).removeFromTop(18),
               juce::Justification::topRight, false);

    g.setColour(juce::Colours::white.withAlpha(0.35f));
    g.setFont(11.0f);
    g.drawText("SolarDrone", getLocalBounds().reduced(8),
               juce::Justification::topLeft, false);
}
