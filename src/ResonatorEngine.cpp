#include "ResonatorEngine.h"
#include <cmath>
#include <algorithm>

void ResonatorEngine::prepare(const juce::dsp::ProcessSpec& spec) {
    modalBank.prepare(spec);
    fdn.prepare(spec);
    strings.prepare(spec);
    const int ch = (int)spec.numChannels;
    const int bs = (int)spec.maximumBlockSize;
    modalBuf.setSize(ch, bs);
    fdnBuf.setSize(ch, bs);
    stringBuf.setSize(ch, bs);
    // Apply initial wet levels
    modalBank.setWet(e_w(modalEnabled, modalWet));
    fdn.setWet(e_w(fdnEnabled, fdnWet));
    strings.setWet(e_w(stringsEnabled, stringsWet));
}

void ResonatorEngine::reset() {
    modalBank.reset();
    fdn.reset();
    strings.reset();
}

void ResonatorEngine::updateFromSolarData(const SpaceWeatherState& sw, float l1Hz) {
    // ── Modal Bank ────────────────────────────────────────────────────────
    // Velocity → decay (fast wind = brighter, shorter modes)
    const float vNorm = std::max(0.f, std::min(1.f, (sw.velocity - 300.f) / 500.f));
    modalBank.setDecaySeconds(modalDecayUser * (2.f - vNorm * 1.5f));  // 0.5× – 2× user decay

    // Kp → mode count (storm = dense, inharmonic)
    const int   modes = 4 + (int)(28.f * (sw.kp / 9.f));
    modalBank.setModeCount(std::max(4, std::min(32, modes)));
    modalBank.setInharmonicity(sw.kp >= 6.f ? (sw.kp - 6.f) / 3.f * 0.45f : 0.f);
    modalBank.setFundamental(l1Hz);

    // ── FDN ───────────────────────────────────────────────────────────────
    // Dst → T60 (deep storm = very long cavernous tail)
    const float dstNorm = std::max(0.f, std::min(1.f, -sw.dst_index / 200.f));
    fdn.setT60(fdnT60User * (0.3f + dstNorm * 7.7f));  // 0.3× – 8×

    // Dynamic pressure → absorption cutoff (high pressure = dampened)
    const float pressure  = 1.67e-6f * sw.density * sw.velocity * sw.velocity; // nPa
    const float pressNorm = std::max(0.f, std::min(1.f, (pressure - 1.f) / 49.f));
    fdn.setAbsorptionCutoff(12000.f - pressNorm * 8000.f);  // 12kHz→4kHz

    // ── Sympathetic Strings ───────────────────────────────────────────────
    // Temperature → brightness (hot = brighter loop filter via damping proxy)
    const float tempNorm = std::max(0.f, std::min(1.f,
                               (std::log10(std::max(1.f, sw.temperature)) - 4.3f) / 1.5f));
    // Proton flux → damping + noise injection (SEP event = turbulent)
    const float fluxLog  = std::max(0.f, std::log10(std::max(0.3f, sw.proton_flux_10mev))
                                         - std::log10(0.3f));
    const float protonN  = std::min(1.f, fluxLog / 4.f);
    strings.setDamping(protonN * 0.8f);
    strings.setNoiseLevel(protonN > 0.3f ? (protonN - 0.3f) / 0.7f : 0.f);
    strings.setFundamental(l1Hz);
    juce::ignoreUnused(tempNorm);  // reserved for future brightness mapping
}

void ResonatorEngine::process(juce::AudioBuffer<float>& buffer) {
    juce::ScopedNoDenormals noDenormals;
    if (!isEnabled()) return;

    const int n  = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();

    // Copy dry to all three temp buffers
    for (int c = 0; c < ch; ++c) {
        modalBuf.copyFrom( c, 0, buffer, c, 0, n);
        fdnBuf.copyFrom(   c, 0, buffer, c, 0, n);
        stringBuf.copyFrom(c, 0, buffer, c, 0, n);
    }

    // Each resonator produces wet-only signal in its temp buffer
    if (modalEnabled)   modalBank.process(modalBuf);   else modalBuf.clear();
    if (fdnEnabled)     fdn.process(fdnBuf);            else fdnBuf.clear();
    if (stringsEnabled) strings.process(stringBuf);    else stringBuf.clear();

    // Sum: dry + modal_wet + fdn_wet + string_wet (NaN guard)
    for (int c = 0; c < ch; ++c) {
        auto* out = buffer.getWritePointer(c);
        const auto* m = modalBuf.getReadPointer(c);
        const auto* f = fdnBuf.getReadPointer(c);
        const auto* s = stringBuf.getReadPointer(c);
        for (int i = 0; i < n; ++i) {
            const float wet = m[i] + f[i] + s[i];
            out[i] = std::isfinite(wet) ? out[i] + wet : out[i];
        }
    }
}
