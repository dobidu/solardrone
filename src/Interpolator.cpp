#include "Interpolator.h"
#include <cmath>

void Interpolator::prepare(double /*sampleRate*/, int ctrlRateHz) {
    controlRateHz = ctrlRateHz;
    updateAlpha();
}

void Interpolator::setGlideTimeSecs(float seconds) {
    glideTimeSecs = seconds > 0.0f ? seconds : 0.1f;
    updateAlpha();
}

void Interpolator::updateAlpha() {
    // Exponential approach: reaches 99% of target in glideTimeSecs
    // ln(100) ≈ 4.6
    alpha = 1.0f - std::exp(-4.6f / (glideTimeSecs * (float)controlRateHz));
}

void Interpolator::setTarget(const SynthParams& t) {
    l1_hz.target    = t.l1_fundamental_hz;
    l1_timbre.target = t.l1_timbre;
    l1_amp.target   = t.l1_amplitude;
    l2_hz.target    = t.l2_fundamental_hz;
    l2_density.target = t.l2_harmonic_density;
    l2_bright.target  = t.l2_brightness;
    l2_amp.target     = t.l2_amplitude;
    stereo_w.target   = t.stereo_width;
    l1_harmonic_count_target = t.l1_harmonic_count;
}

void Interpolator::reset(const SynthParams& v) {
    l1_hz     = {v.l1_fundamental_hz,   v.l1_fundamental_hz};
    l1_timbre = {v.l1_timbre,           v.l1_timbre};
    l1_amp    = {v.l1_amplitude,        v.l1_amplitude};
    l2_hz     = {v.l2_fundamental_hz,   v.l2_fundamental_hz};
    l2_density = {v.l2_harmonic_density, v.l2_harmonic_density};
    l2_bright  = {v.l2_brightness,       v.l2_brightness};
    l2_amp     = {v.l2_amplitude,        v.l2_amplitude};
    stereo_w   = {v.stereo_width,        v.stereo_width};
    l1_harmonic_count_target = v.l1_harmonic_count;
}

float Interpolator::slew(SlewState& s, float a) {
    s.current += a * (s.target - s.current);
    return s.current;
}

SynthParams Interpolator::getSmoothed() {
    SynthParams out;
    out.l1_fundamental_hz   = slew(l1_hz,      alpha);
    out.l1_timbre           = slew(l1_timbre,   alpha);
    out.l1_amplitude        = slew(l1_amp,      alpha);
    out.l2_fundamental_hz   = slew(l2_hz,       alpha);
    out.l2_harmonic_density = slew(l2_density,  alpha);
    out.l2_brightness       = slew(l2_bright,   alpha);
    out.l2_amplitude        = slew(l2_amp,      alpha);
    out.stereo_width        = slew(stereo_w,    alpha);
    out.l1_harmonic_count   = l1_harmonic_count_target;
    return out;
}
