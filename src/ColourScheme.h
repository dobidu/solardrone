#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>
#include <algorithm>

struct ColourScheme {
    static juce::Colour kpToAccent(float kp) {
        kp = std::max(0.f, std::min(9.f, kp));
        if (kp < 2.f)
            return juce::Colour::fromHSV(0.62f, 0.75f, 0.85f, 1.f);   // deep blue
        if (kp < 5.f) {
            const float t = (kp - 2.f) / 3.f;
            return juce::Colour::fromHSV(0.62f - t * 0.17f, 0.75f, 0.9f, 1.f); // blue→teal
        }
        if (kp < 7.f) {
            const float t = (kp - 5.f) / 2.f;
            return juce::Colour::fromHSV(0.45f - t * 0.34f, 0.85f, 1.0f, 1.f); // teal→amber
        }
        const float t = (kp - 7.f) / 2.f;
        return juce::Colour::fromHSV(0.11f - t * 0.11f, 0.9f, 1.0f, 1.f);      // amber→red
    }

    static juce::Colour background(float kp) {
        return kpToAccent(kp).withBrightness(0.07f).withSaturation(0.45f);
    }

    static juce::Colour grid(float kp) {
        return kpToAccent(kp).withAlpha(0.06f);
    }
};
