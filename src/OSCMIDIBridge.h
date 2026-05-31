#pragma once
#include <juce_osc/juce_osc.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "SynthParams.h"
#include "SpaceWeatherState.h"

class OSCMIDIBridge {
public:
    OSCMIDIBridge();
    ~OSCMIDIBridge();

    void setOSCEnabled(bool e, int port);
    void setMIDIEnabled(bool e);

    void send(const SynthParams& synth,
              const SpaceWeatherState& weather,
              float flareLevel,
              float volume,
              float layerBalance);

    bool wasRecentlySent() const;

private:
    void sendOSC(const SynthParams& s, const SpaceWeatherState& w, float fl);
    void sendMIDI(const SynthParams& s, const SpaceWeatherState& w,
                  float fl, float vol, float balance);

    juce::OSCSender   oscSender;
    std::unique_ptr<juce::MidiOutput> midiOut;

    bool oscEnabled  = false;
    bool midiEnabled = false;
    int  oscPort     = 9000;
    int  lastPort    = -1;

    juce::int64 lastSendMs = 0;
    juce::int64 lastSentMs = -1;
    static constexpr int kIntervalMs = 100;
};
