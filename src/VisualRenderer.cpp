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

            // Flare flash on new M+ event
            const auto fc = cachedWeatherState.flare_class;
            if (fc >= SpaceWeatherState::FlareClass::M && fc > lastFlareClass)
                flashAlpha = 0.4f;
            lastFlareClass = fc;
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
    // Store trail phase
    trailPhases[trailWriteIdx % kTrailLen] = lissPhase;
    ++trailWriteIdx;
    updateParticles(p, dt);

    repaint();
}

// ── Lissajous ────────────────────────────────────────────────────────────────

void VisualRenderer::drawLissajous(juce::Graphics& g,
                                    const SynthParams& p, float blend) {
    if (blend < 0.01f) return;
    const float w = (float)getWidth(), h = (float)getHeight();
    const float cx = w*0.5f, cy = h*0.5f;
    const float rx = w*0.43f, ry = h*0.43f;
    const double twoPi2 = juce::MathConstants<double>::twoPi * 2.0;

    double ratio = (double)(p.l2_fundamental_hz / p.l1_fundamental_hz);
    ratio = std::max(0.5, std::min(ratio, 4.0));

    // Depth of 3D effect (increases with timbre/storm)
    const float depth   = 0.20f + p.l1_timbre * 0.40f;
    const float thickness = 0.8f + p.l1_amplitude * 1.5f;

    const auto accent    = juce::Colour::fromHSV(0.6f - p.l1_timbre*0.6f, 0.8f, 1.0f, 1.f);
    const auto calmColor = juce::Colour::fromHSV(0.62f, 0.75f, 0.85f, 1.f);

    // ── Phosphor trail (drawn first, behind main curve) ───────────────────
    for (int tr = kTrailLen - 1; tr >= 1; --tr) {
        const int    idx   = (trailWriteIdx - tr + kTrailLen * 64) % kTrailLen;
        const double tPhase = trailPhases[idx];
        const float  alpha  = (float)(kTrailLen - tr) / kTrailLen * 0.15f * blend;
        juce::Path   tp;
        const int    tN = 128;
        for (int i = 0; i <= tN; ++i) {
            const double t  = twoPi2 * i / tN;
            const float  tz = (float)std::sin(t*(ratio+0.7)+tPhase*1.4) * 0.35f;
            const float  sc = 1.f / (1.f + tz * depth);
            const float  tx = cx + (float)std::sin(t) * rx * sc;
            const float  ty = cy + (float)std::sin(ratio*t+tPhase) * ry * sc;
            if (i==0) tp.startNewSubPath(tx,ty); else tp.lineTo(tx,ty);
        }
        g.setColour(accent.withAlpha(alpha));
        g.strokePath(tp, juce::PathStrokeType(0.7f));
    }

    // ── Main curve: 3D projection + gradient color in 16 segments ────────
    const int N   = 512;
    const int seg = 16;
    const int spp = N / seg;

    // Pre-compute projected points
    std::vector<juce::Point<float>> pts(N + 1);
    for (int i = 0; i <= N; ++i) {
        const double t  = twoPi2 * i / N;
        const float  z  = (float)std::sin(t*(ratio+0.7)+lissPhase*1.4) * 0.35f;
        const float  sc = 1.f / (1.f + z * depth);
        pts[i] = { cx + (float)std::sin(t)*rx*sc,
                   cy + (float)std::sin(ratio*t+lissPhase)*ry*sc };
    }

    // Glow layer: thick + very transparent, drawn before main
    {
        juce::Path glowPath;
        glowPath.startNewSubPath(pts[0].x, pts[0].y);
        for (int i = 1; i <= N; ++i) glowPath.lineTo(pts[i].x, pts[i].y);
        g.setColour(accent.withAlpha(blend * 0.12f));
        g.strokePath(glowPath, juce::PathStrokeType(thickness * 5.f));
        g.setColour(accent.withAlpha(blend * 0.22f));
        g.strokePath(glowPath, juce::PathStrokeType(thickness * 2.5f));
    }

    for (int s = 0; s < seg; ++s) {
        const float frac = (float)s / seg;
        const auto  col  = calmColor.interpolatedWith(accent, frac);
        juce::Path  sp;
        const int   start = s * spp;
        const int   end   = (s == seg-1) ? N : start + spp;
        sp.startNewSubPath(pts[start].x, pts[start].y);
        for (int i = start+1; i <= end; ++i) sp.lineTo(pts[i].x, pts[i].y);
        g.setColour(col.withAlpha(blend * 0.85f));
        g.strokePath(sp, juce::PathStrokeType(thickness));
    }
}

// ── Particles ────────────────────────────────────────────────────────────────

void VisualRenderer::updateParticles(const SynthParams& p, float dt) {
    auto& rng = juce::Random::getSystemRandom();

    // Target count: up to 200 with density
    const int targetCount = 10 + (int)(190.0f * p.l2_harmonic_density);
    const float speed     = 0.05f + p.l1_amplitude * 0.25f;
    const float turb      = p.l1_timbre * 0.8f;
    const float hue       = 0.6f - p.l2_brightness * 0.5f;  // violet → orange
    const float lifetime  = 3.0f + p.l2_amplitude * 4.0f;

    // Update existing particles
    for (auto& par : particles) {
        par.vx += (rng.nextFloat() - 0.5f) * turb * dt * 2.0f;
        par.vy += (rng.nextFloat() - 0.5f) * turb * dt * 2.0f;
        par.vx *= (1.0f - dt * 0.5f);
        par.vy *= (1.0f - dt * 0.5f);
        par.x  += par.vx * dt;
        par.y  += par.vy * dt;
        // Z drift (slow depth oscillation)
        par.z  += (rng.nextFloat() - 0.5f) * 0.02f * dt;
        par.z   = std::max(-0.5f, std::min(0.5f, par.z));
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
        par.z       = (rng.nextFloat() - 0.5f);  // random depth
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
    const float w = (float)getWidth(), h = (float)getHeight();

    // Painter's sort: far (negative Z) first
    std::vector<int> idx(particles.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(),
              [&](int a, int b){ return particles[a].z < particles[b].z; });

    for (int i : idx) {
        const auto& par = particles[i];
        if (par.life <= 0.f) continue;
        const float depthScale = 0.5f + (par.z + 0.5f) * 1.0f;  // 0.5 far → 1.5 near
        const float alpha = par.life * blend * 0.8f * depthScale;
        const float sz    = par.size * blend * depthScale;
        // Slight parallax horizontal offset by Z
        const float px    = (par.x + par.z * 0.06f) * w;
        const float py    = (par.y + par.z * 0.04f) * h;
        // Bloom: 3-pass (outer glow → mid → core)
        const float h2 = par.hue;
        g.setColour(juce::Colour::fromHSV(h2, 0.5f, 1.0f, std::min(1.f, alpha*0.15f)));
        g.fillEllipse(px - sz*2.f, py - sz*2.f, sz*4.f, sz*4.f);
        g.setColour(juce::Colour::fromHSV(h2, 0.65f, 1.0f, std::min(1.f, alpha*0.35f)));
        g.fillEllipse(px - sz,     py - sz,     sz*2.f, sz*2.f);
        g.setColour(juce::Colour::fromHSV(h2, 0.9f, 1.0f, std::min(1.f, alpha)));
        g.fillEllipse(px - sz*0.5f, py - sz*0.5f, sz, sz);
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
        const auto barCol = juce::Colour::fromHSV(hue1, 0.6f + p.l1_timbre*0.3f, 1.0f, alpha);
        g.setColour(barCol);
        g.fillRect(bx - bw*0.5f, baseY - barHi, bw, barHi);
        // 3D right face
        const float ex = 4.f, ey = -3.f;
        g.setColour(barCol.withBrightness(barCol.getBrightness()*0.55f).withAlpha(alpha*0.8f));
        juce::Path side;
        side.startNewSubPath(bx+bw*0.5f, baseY);
        side.lineTo(bx+bw*0.5f+ex, baseY+ey);
        side.lineTo(bx+bw*0.5f+ex, baseY-barHi+ey);
        side.lineTo(bx+bw*0.5f,    baseY-barHi);
        side.closeSubPath();
        g.fillPath(side);
        // 3D top face
        g.setColour(barCol.brighter(0.3f).withAlpha(alpha*0.6f));
        juce::Path top;
        top.startNewSubPath(bx-bw*0.5f,    baseY-barHi);
        top.lineTo(bx-bw*0.5f+ex,  baseY-barHi+ey);
        top.lineTo(bx+bw*0.5f+ex,  baseY-barHi+ey);
        top.lineTo(bx+bw*0.5f,     baseY-barHi);
        top.closeSubPath();
        g.fillPath(top);
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

    // Storm flash (Kp crosses 7) + volumetric glow
    if (ws.kp >= 7.f && lastKpForStorm < 7.f) stormFlashAlpha = 0.5f;
    lastKpForStorm = ws.kp;
    stormFlashAlpha *= 0.87f;
    if (stormFlashAlpha > 0.01f) {
        g.setColour(ColourScheme::kpToAccent(ws.kp).withAlpha(stormFlashAlpha * 0.35f));
        g.fillAll();
    }
    // Volumetric center glow (increases with Kp or amplitude)
    if (ws.kp > 3.f || p.l1_amplitude > 0.5f) {
        const float glowR = std::min((float)getWidth(),(float)getHeight()) * 0.33f;
        const float gAmp  = (ws.kp / 9.f * 0.5f + p.l1_amplitude * 0.25f)
                            * std::min(u.visual_lissajous, 1.f);
        juce::ColourGradient grad(
            ColourScheme::kpToAccent(ws.kp).withAlpha(gAmp * 0.18f),
            (float)getWidth()*0.5f, (float)getHeight()*0.5f,
            juce::Colours::transparentBlack,
            (float)getWidth()*0.5f + glowR, (float)getHeight()*0.5f,
            true);
        g.setGradientFill(grad);
        g.fillEllipse((float)getWidth()*0.5f-glowR,(float)getHeight()*0.5f-glowR,
                      glowR*2.f, glowR*2.f);
    }

    // Flare flash + badge
    flashAlpha *= 0.85f;
    if (flashAlpha > 0.01f) {
        g.setColour(juce::Colours::white.withAlpha(flashAlpha));
        g.fillAll();
    }
    if (ws.flare_class >= SpaceWeatherState::FlareClass::M) {
        const char* fc = (ws.flare_class == SpaceWeatherState::FlareClass::X) ? "X-FLARE"
                       : (ws.flare_class == SpaceWeatherState::FlareClass::M) ? "M-FLARE" : "";
        if (fc[0] != '\0') {
            g.setFont(13.0f);
            g.setColour(juce::Colours::orange.withAlpha(0.90f));
            g.drawText(fc, getLocalBounds().reduced(8).removeFromTop(22),
                       juce::Justification::topLeft, false);
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
