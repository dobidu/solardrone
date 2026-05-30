#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include "SynthParams.h"
#include "UserParams.h"
#include "SpaceWeatherState.h"

class SolarDroneAudioProcessor;  // forward declaration — avoid circular include

class VisualRenderer : public juce::Component, public juce::Timer {
public:
    explicit VisualRenderer(SolarDroneAudioProcessor* proc = nullptr);
    ~VisualRenderer() override;

    // Thread-safe: may be called from audio thread or any thread
    void setSynthParams(const SynthParams& p, const UserParams& u);

    void paint(juce::Graphics&) override;
    void resized() override {}
    void timerCallback() override;

private:
    // ── Lissajous layer ───────────────────────────────────────────────────
    void drawLissajous(juce::Graphics&, const SynthParams&, float blend);
    double lissPhase = 0.0;

    // ── Particle layer ────────────────────────────────────────────────────
    struct Particle {
        float x = 0.5f, y = 0.5f;
        float vx = 0.0f, vy = 0.0f;
        float life     = 1.0f;    // 1=just born, 0=dead
        float maxLife  = 4.0f;    // lifetime in seconds
        float hue      = 0.6f;
        float size     = 3.0f;
    };
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

    // ── Processor reference for polling ──────────────────────────────────
    SolarDroneAudioProcessor* processor = nullptr;
    juce::int64 lastFrameMs = 0;

    // Cached for status overlay — updated in timerCallback()
    SpaceWeatherState cachedWeatherState;

    // Freeze ghost overlay
    bool        frozen       = false;
    bool        wasFreeze    = false;
    juce::Image frozenSnapshot;

};
