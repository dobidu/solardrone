#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

class Chopper {
public:
    enum class Shape { sine, square, saw };

    void prepare(double sampleRate);
    void setEnabled(bool e);
    void setDepth(float d);
    void setShape(Shape s);
    void setRate(float hz);
    void setBPMSync(bool sync, float bpm, float divisionBeats);
    void setAttack(float a)      { attack  = std::max(0.f, std::min(1.f, a)); }
    void setRelease(float r)     { release = std::max(0.f, std::min(1.f, r)); }
    void setPhaseOffset(float deg) { phaseOffset = deg / 360.0; }

    void process(juce::AudioBuffer<float>& buffer);

private:
    void  updatePhaseInc();
    float lfoValue() const;

    double sampleRate = 44100.0;
    bool   enabled    = false;
    float  depth      = 1.0f;
    Shape  shape      = Shape::square;
    bool   bpmSync    = true;
    float  rate       = 4.0f;
    double phase          = 0.0;
    double phaseInc       = 0.0;
    double targetPhaseInc = 0.0;
    double phaseOffset    = 0.0;   // 0..1 (0=0°, 0.5=180°, 1=360°)
    float  attack         = 0.05f; // 0..1 fraction of half-period
    float  release        = 0.05f;
    float  smoothedGate   = 1.0f;  // current smoothed gate value
};
