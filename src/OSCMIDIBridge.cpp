#include "OSCMIDIBridge.h"
#include <algorithm>

OSCMIDIBridge::OSCMIDIBridge() {}

OSCMIDIBridge::~OSCMIDIBridge() {
    oscSender.disconnect();
}

void OSCMIDIBridge::setOSCEnabled(bool e, int port) {
    if (e != oscEnabled || (e && port != lastPort)) {
        oscEnabled = e;
        oscPort    = port;
        lastPort   = port;
        oscSender.disconnect();
        if (e) oscSender.connect("127.0.0.1", port);
    }
}

void OSCMIDIBridge::setMIDIEnabled(bool e) {
    if (e == midiEnabled) return;
    midiEnabled = e;
    if (e && midiOut == nullptr) {
        const auto devices = juce::MidiOutput::getAvailableDevices();
        if (!devices.isEmpty())
            midiOut = juce::MidiOutput::openDevice(devices[0].identifier);
    }
}

void OSCMIDIBridge::send(const SynthParams& s, const SpaceWeatherState& w,
                          float fl, float vol, float balance) {
    const auto now = juce::Time::currentTimeMillis();
    if (now - lastSendMs < kIntervalMs) return;
    lastSendMs = now;
    if (oscEnabled)  sendOSC(s, w, fl);
    if (midiEnabled) sendMIDI(s, w, fl, vol, balance);
    lastSentMs = now;
}

bool OSCMIDIBridge::wasRecentlySent() const {
    return lastSentMs >= 0 && (juce::Time::currentTimeMillis() - lastSentMs) < 150;
}

void OSCMIDIBridge::sendOSC(const SynthParams& s, const SpaceWeatherState& w, float fl) {
    oscSender.send("/solardrone/synth/l1_fundamental", s.l1_fundamental_hz);
    oscSender.send("/solardrone/synth/l1_timbre",      s.l1_timbre);
    oscSender.send("/solardrone/synth/l1_amplitude",   s.l1_amplitude);
    oscSender.send("/solardrone/synth/l2_amplitude",   s.l2_amplitude);
    oscSender.send("/solardrone/synth/l2_density",     s.l2_harmonic_density);
    oscSender.send("/solardrone/synth/l2_brightness",  s.l2_brightness);
    oscSender.send("/solardrone/synth/l3_amplitude",   s.l3_amplitude);
    oscSender.send("/solardrone/weather/velocity",     w.velocity);
    oscSender.send("/solardrone/weather/bz",           w.bz_gsm);
    oscSender.send("/solardrone/weather/kp",           w.kp);
    oscSender.send("/solardrone/weather/flare_class",  (int)w.flare_class);
    oscSender.send("/solardrone/flare/level",          fl);
}

void OSCMIDIBridge::sendMIDI(const SynthParams& s, const SpaceWeatherState& w,
                              float fl, float vol, float balance) {
    if (!midiOut) return;
    auto cc = [&](int num, float v01) {
        midiOut->sendMessageNow(
            juce::MidiMessage::controllerEvent(1, num,
                std::max(0, std::min(127, (int)(v01 * 127.f)))));
    };
    cc(1,   w.kp / 9.0f);
    cc(7,   vol);
    cc(10,  balance);
    cc(11,  s.l1_amplitude);
    cc(71,  s.l2_harmonic_density);
    cc(74,  s.l2_brightness);
    cc(116, fl);
}
