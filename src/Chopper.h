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
    double phase      = 0.0;
    double phaseInc   = 0.0;
};
