#include "OscillatorBank.h"
#include <juce_core/juce_core.h>
#include <cmath>
#include <algorithm>

void OscillatorBank::prepare(double sr, int maxPartials) {
    sampleRate = sr;
    partials.resize((size_t)maxPartials);
    activeCount = 0;
}

void OscillatorBank::applyParams(const SynthParams& p, int layer, float layerGain) {
    const double twoPi = juce::MathConstants<double>::twoPi;

    if (layer == 0) {
        // L1 — solar wind: harmonics of l1_fundamental_hz
        activeCount = std::max(2, std::min(p.l1_harmonic_count, (int)partials.size()));
        const double fundamental = (double)p.l1_fundamental_hz;
        const float timbre = p.l1_timbre;

        // Stereo: L1 slightly left
        const float sw = p.stereo_width;
        const float panL = 0.5f + sw * 0.3f;
        const float panR = 1.0f - panL;

        for (int i = 0; i < activeCount; ++i) {
            const float harmonic = (float)(i + 1);
            // Amplitude taper: 1/(i+1), attenuated by overall l1_amplitude
            float amp = (p.l1_amplitude / harmonic) * layerGain;
            // Timbre adds inharmonic detuning to upper partials
            double detune = 1.0 + (double)timbre * 0.003 * i;
            partials[i].phaseInc = twoPi * fundamental * harmonic * detune / sampleRate;
            partials[i].ampLeft  = amp * panL;
            partials[i].ampRight = amp * panR;
        }
    } else {
        // L2 — Kp: harmonics of l2_fundamental_hz, density-controlled count
        activeCount = 2 + (int)(22.0f * p.l2_harmonic_density);
        activeCount = std::min(activeCount, (int)partials.size());
        const double fundamental = (double)p.l2_fundamental_hz;

        // Stereo: L2 slightly right
        const float sw = p.stereo_width;
        const float panR = 0.5f + sw * 0.3f;
        const float panL = 1.0f - panR;

        for (int i = 0; i < activeCount; ++i) {
            const float harmonic = (float)(i + 1);
            // Brightness boosts upper partials
            float brightBoost = 1.0f + (float)i * p.l2_brightness * 0.3f;
            float amp = (p.l2_amplitude / harmonic) * brightBoost * layerGain;
            amp = std::min(amp, 1.0f);  // safety clamp
            partials[i].phaseInc = twoPi * fundamental * (double)harmonic / sampleRate;
            partials[i].ampLeft  = amp * panL;
            partials[i].ampRight = amp * panR;
        }
    }
}

void OscillatorBank::render(float* left, float* right, int numSamples) {
    const double twoPi = juce::MathConstants<double>::twoPi;
    for (int i = 0; i < activeCount; ++i) {
        auto& par = partials[i];
        for (int n = 0; n < numSamples; ++n) {
            const float s = (float)std::sin(par.phase);
            left[n]  += s * par.ampLeft;
            right[n] += s * par.ampRight;
            par.phase += par.phaseInc;
        }
        // Wrap phase to avoid precision loss over time
        while (par.phase > twoPi) par.phase -= twoPi;
    }
}
