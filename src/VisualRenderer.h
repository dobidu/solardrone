#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include "SynthParams.h"
#include "UserParams.h"
#include "SpaceWeatherState.h"

class SolarDroneAudioProcessor;

class VisualRenderer : public juce::Component, public juce::Timer {
public:
    explicit VisualRenderer(SolarDroneAudioProcessor* proc = nullptr);
    ~VisualRenderer() override;

    void setSynthParams(const SynthParams& p, const UserParams& u);

    void paint(juce::Graphics&) override;
    void resized() override {}
    void timerCallback() override;

private:
    // ── Lissajous layer ───────────────────────────────────────────────────
    void drawLissajous(juce::Graphics&, const SynthParams&, float blend);
    double lissPhase = 0.0;

    // Phosphor trail
    static constexpr int kTrailLen = 6;
    double trailPhases[kTrailLen] = {};
    int    trailWriteIdx = 0;

    // ── Particle layer ────────────────────────────────────────────────────
    struct Particle {
        float x=0.5f, y=0.5f, z=0.0f;
        float vx=0.0f, vy=0.0f;
        float life=1.0f, maxLife=4.0f, hue=0.6f, size=3.0f;
    };
    static constexpr int kMaxParticles = 200;
    void drawParticles(juce::Graphics&, const SynthParams&, float blend);
    void updateParticles(const SynthParams&, float dt);
    std::vector<Particle> particles;
    float spawnAccum = 0.0f;

    // ── Spectral layer ────────────────────────────────────────────────────
    void drawSpectral(juce::Graphics&, const SynthParams&, float blend);

    // ── Thread-safe SynthParams handoff ───────────────────────────────────
    juce::CriticalSection paramsLock;
    SynthParams displayParams;
    UserParams  displayUserParams;

    // ── Processor reference ───────────────────────────────────────────────
    SolarDroneAudioProcessor* processor = nullptr;
    juce::int64 lastFrameMs = 0;

    SpaceWeatherState cachedWeatherState;

    // Freeze ghost overlay
    bool        frozen       = false;
    bool        wasFreeze    = false;
    juce::Image frozenSnapshot;

    // Bz history for spark-line
    float bzHistory[120] = {};
    int   bzHistoryIdx   = 0;

    // Flare flash + storm flash
    float flashAlpha      = 0.0f;
    float stormFlashAlpha = 0.0f;
    float lastKpForStorm  = 0.0f;
    SpaceWeatherState::FlareClass lastFlareClass = SpaceWeatherState::FlareClass::none;
};
