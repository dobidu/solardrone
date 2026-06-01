#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "SpaceWeatherState.h"

class SolarTerminal : public juce::Component, private juce::Timer {
public:
    SolarTerminal();
    ~SolarTerminal() override = default;

    void updateFromSolarData(const SpaceWeatherState& sw,
                             bool modalOn, bool fdnOn, bool stringsOn);
    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;

    struct LogLine { juce::String text; bool alert = false; };
    static constexpr int kLogMax = 10;
    LogLine log[kLogMax];
    int logCount = 0, logHead = 0;

    void addLog(const juce::String& text, bool alert = false);
    juce::String timeTag() const;

    SpaceWeatherState cached;
    bool modalOn = false, fdnOn = false, stringsOn = false;
    bool hasData = false;
    bool cursorOn = true;

    // Change detection — solar
    float prevKp      = -99.f;
    float prevDst     =  999.f;
    float prevProton  =   -1.f;
    SpaceWeatherState::Source prevSource = SpaceWeatherState::Source::defaultValue;
    // Change detection — resonators
    int prevResState  = -1;  // bitmask: bit0=modal, bit1=fdn, bit2=strings

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SolarTerminal)
};
