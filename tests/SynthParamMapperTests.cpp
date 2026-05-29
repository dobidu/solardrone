#include <juce_core/juce_core.h>
#include "SynthParamMapper.h"

static SpaceWeatherState quietState() {
    SpaceWeatherState s;
    s.velocity = 400.0f;
    s.density  = 5.0f;
    s.bz_gsm   = 2.0f;
    s.kp       = 1.0f;
    return s;
}

static SpaceWeatherState stormState() {
    SpaceWeatherState s;
    s.velocity = 700.0f;
    s.density  = 20.0f;
    s.bz_gsm   = -15.0f;
    s.kp       = 8.0f;
    return s;
}

class SynthParamMapperTests : public juce::UnitTest {
public:
    SynthParamMapperTests() : juce::UnitTest("SynthParamMapper") {}

    void runTest() override {
        testQuietDayLowActivation();
        testStormHighActivation();
        testKp9Maximum();
        testDeterministic();
        testHarmonyModeDiffers();
        testVelocityLogScale();
        testDensityHarmonicCount();
    }

private:
    void testQuietDayLowActivation() {
        beginTest("Quiet day: low activation (vel=400, Bz=+2, Kp=1)");
        auto out = SynthParamMapper::map(quietState());
        expect(out.l1_timbre           < 0.3f, "l1_timbre < 0.3");
        expect(out.l2_harmonic_density < 0.3f, "l2_density < 0.3");
        expect(out.l2_amplitude        < 0.4f, "l2_amplitude < 0.4");
        expect(out.l2_brightness       < 0.3f, "l2_brightness < 0.3");
    }

    void testStormHighActivation() {
        beginTest("Storm: high activation (vel=700, Bz=-15, Kp=8)");
        auto out = SynthParamMapper::map(stormState());
        expect(out.l1_timbre           > 0.7f, "l1_timbre > 0.7");
        expect(out.l2_harmonic_density > 0.7f, "l2_density > 0.7");
        expect(out.l2_amplitude        > 0.7f, "l2_amplitude > 0.7");
        expect(out.l2_brightness       > 0.7f, "l2_brightness > 0.7");
    }

    void testKp9Maximum() {
        beginTest("Kp=9: maximum activation clamped to 1.0");
        SpaceWeatherState s; s.kp = 9.0f;
        auto out = SynthParamMapper::map(s);
        expectWithinAbsoluteError(out.l2_harmonic_density, 1.0f, 1e-5f, "density == 1.0");
        expectWithinAbsoluteError(out.l2_brightness,       1.0f, 1e-5f, "brightness == 1.0");
        expectWithinAbsoluteError(out.l2_amplitude,        1.0f, 1e-5f, "amplitude == 1.0");
    }

    void testDeterministic() {
        beginTest("Pure function: identical inputs produce identical outputs");
        auto s = stormState();
        auto a = SynthParamMapper::map(s);
        auto b = SynthParamMapper::map(s);
        expectEquals(a.l1_fundamental_hz,   b.l1_fundamental_hz,   "l1_hz equal");
        expectEquals(a.l2_amplitude,        b.l2_amplitude,        "l2_amp equal");
        expectEquals(a.l1_timbre,           b.l1_timbre,           "timbre equal");
        expectEquals(a.l2_harmonic_density, b.l2_harmonic_density, "density equal");
    }

    void testHarmonyModeDiffers() {
        beginTest("Harmony mode: just vs equal -> different l2_fundamental_hz, same l1");
        SpaceWeatherState s; s.velocity = 400.0f;
        UserParams just_p;  just_p.harmony_mode  = UserParams::HarmonyMode::just;
        UserParams equal_p; equal_p.harmony_mode = UserParams::HarmonyMode::equalTemperament;
        auto just_out  = SynthParamMapper::map(s, just_p);
        auto equal_out = SynthParamMapper::map(s, equal_p);
        expectEquals(just_out.l1_fundamental_hz, equal_out.l1_fundamental_hz, "l1 unchanged");
        expect(just_out.l2_fundamental_hz != equal_out.l2_fundamental_hz, "l2 differs");
    }

    void testVelocityLogScale() {
        beginTest("Velocity log scale: 300 km/s -> 55 Hz, 800 km/s -> 220 Hz");
        SpaceWeatherState s;
        s.velocity = 300.0f;
        expectWithinAbsoluteError(SynthParamMapper::map(s).l1_fundamental_hz, 55.0f,  0.1f, "300->55");
        s.velocity = 800.0f;
        expectWithinAbsoluteError(SynthParamMapper::map(s).l1_fundamental_hz, 220.0f, 0.1f, "800->220");
    }

    void testDensityHarmonicCount() {
        beginTest("Density harmonic count: 1 p/cm3 -> 2 partials, 50 p/cm3 -> 24 partials");
        SpaceWeatherState s;
        s.density = 1.0f;
        expectEquals(SynthParamMapper::map(s).l1_harmonic_count, 2,  "density=1 -> 2 partials");
        s.density = 50.0f;
        expectEquals(SynthParamMapper::map(s).l1_harmonic_count, 24, "density=50 -> 24 partials");
    }
};

static SynthParamMapperTests synthParamMapperTests;
