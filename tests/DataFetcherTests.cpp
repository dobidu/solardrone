#include <juce_core/juce_core.h>
#include "DataFetcher.h"

static const char* kQuietDayWind = R"json(
[{
  "time_tag": "2024-01-15 12:00:00.000",
  "propagated_time_tag": "2024-01-15 12:06:00.000",
  "speed": 412.5,
  "density": 4.8,
  "bx_gsm": 1.2,
  "by_gsm": -0.8,
  "bz_gsm": 2.1,
  "temperature": 75000.0
}]
)json";

static const char* kStormDayWind = R"json(
[{
  "time_tag": "2024-03-24 06:00:00.000",
  "propagated_time_tag": "2024-03-24 06:04:00.000",
  "speed": 712.3,
  "density": 18.6,
  "bx_gsm": -3.1,
  "by_gsm": 5.4,
  "bz_gsm": -18.4,
  "temperature": 320000.0
}]
)json";

static const char* kAllNullWind = R"json(
[{
  "time_tag": "2024-01-15 12:01:00.000",
  "propagated_time_tag": null,
  "speed": null,
  "density": null,
  "bx_gsm": null,
  "by_gsm": null,
  "bz_gsm": null,
  "temperature": null
}]
)json";

static const char* kQuietDayKp = R"json(
[{"time_tag": "2024-01-15 12:00:00.000", "estimated_kp": 1.3}]
)json";

static const char* kStormDayKp = R"json(
[{"time_tag": "2024-03-24 06:00:00.000", "estimated_kp": 8.0}]
)json";

static const char* kNullKp = R"json(
[{"time_tag": "2024-01-15 12:01:00.000", "estimated_kp": null}]
)json";

static const char* kQuietXray = R"json(
[{"time_tag":"2024-01-15 12:00:00.000","satellite":17,"energy":"0.1-0.8nm","flux":2.0e-7},
 {"time_tag":"2024-01-15 12:00:00.000","satellite":17,"energy":"0.05-0.4nm","flux":1.2e-8}]
)json";

static const char* kMFlareXray = R"json(
[{"time_tag":"2024-03-24 06:00:00.000","satellite":17,"energy":"0.1-0.8nm","flux":2.0e-5}]
)json";

static const char* kWindWithTemp = R"json(
[{"time_tag":"2024-01-15 12:00:00.000","speed":450.0,"density":5.0,"bz_gsm":1.5,"temperature":85000.0}]
)json";
static const char* kDstQuiet = R"json(
[{"time_tag":"2024-01-15 12:00:00.000","dst":-12},{"time_tag":"2024-01-15 11:00:00.000","dst":-8}]
)json";
static const char* kProtonQuiet = R"json(
[{"time_tag":"2024-01-15 12:00:00.000","satellite":18,"energy":">=10 MeV","flux":0.5},
 {"time_tag":"2024-01-15 12:00:00.000","satellite":18,"energy":">=50 MeV","flux":0.02}]
)json";

// ─────────────────────────────────────────────────────────────────────────────

class DataFetcherTests : public juce::UnitTest {
public:
    DataFetcherTests() : juce::UnitTest("DataFetcher") {}

    void runTest() override {
        testDefaultState();
        testParseTemperature();
        testParseDst();
        testParseProtonFlux();
        testParseXrayQuiet();
        testParseXrayMFlare();
        testParseQuietDayWind();
        testParseStormDayWind();
        testNullWindRetainsLastValid();
        testParseQuietDayKp();
        testParseStormDayKp();
        testNullKpRetainsLastValid();
        testTimestampExtracted();
    }

private:
    void testParseTemperature() {
        beginTest("Parse solar wind temperature (85000 K)");
        DataFetcher df; df.setPollIntervalMs(999'000'000);
        SpaceWeatherState out;
        bool ok = df.parseSolarWindJson(kWindWithTemp, out);
        expect(ok, "parse succeeds");
        expectWithinAbsoluteError(out.temperature, 85000.f, 1.f, "temperature");
        df.stopThread(1000);
    }

    void testParseDst() {
        beginTest("Parse Dst index (-12 nT quiet)");
        DataFetcher df; df.setPollIntervalMs(999'000'000);
        SpaceWeatherState out;
        bool ok = df.parseDstJson(kDstQuiet, out);
        expect(ok, "parse succeeds");
        expectWithinAbsoluteError(out.dst_index, -12.f, 0.5f, "dst_index");
        df.stopThread(1000);
    }

    void testParseProtonFlux() {
        beginTest("Parse proton flux >=10 MeV (0.5 pfu)");
        DataFetcher df; df.setPollIntervalMs(999'000'000);
        SpaceWeatherState out;
        bool ok = df.parseProtonFluxJson(kProtonQuiet, out);
        expect(ok, "parse succeeds");
        expectWithinAbsoluteError(out.proton_flux_10mev, 0.5f, 0.01f, "proton_flux_10mev");
        df.stopThread(1000);
    }

    void testParseXrayQuiet() {
        beginTest("Parse quiet X-ray (B class, flux=5e-8)");
        DataFetcher df;
        df.setPollIntervalMs(999'000'000);
        SpaceWeatherState out;
        bool ok = df.parseXrayJson(kQuietXray, out);
        expect(ok, "parse should succeed");
        expectWithinAbsoluteError(out.x_ray_flux, 2.0e-7f, 1e-8f, "flux");
        expectEquals((int)out.flare_class, (int)SpaceWeatherState::FlareClass::B, "class B");
        df.stopThread(1000);
    }

    void testParseXrayMFlare() {
        beginTest("Parse M flare X-ray (M2, flux=2e-5)");
        DataFetcher df;
        df.setPollIntervalMs(999'000'000);
        SpaceWeatherState out;
        bool ok = df.parseXrayJson(kMFlareXray, out);
        expect(ok, "parse should succeed");
        expectWithinAbsoluteError(out.x_ray_flux, 2.0e-5f, 1e-6f, "flux");
        expectEquals((int)out.flare_class, (int)SpaceWeatherState::FlareClass::M, "class M");
        df.stopThread(1000);
    }

    void testDefaultState() {
        beginTest("Default state: velocity/density/Bz/Kp at spec defaults");
        // With JUCE_USE_CURL=0, HTTP always returns empty → source=cached, values stay default.
        DataFetcher df;
        df.setPollIntervalMs(999'000'000);
        juce::Thread::sleep(300); // let first poll attempt complete
        auto s = df.getState();
        expectEquals(s.velocity, 450.0f, "default velocity 450 km/s");
        expectEquals(s.density,    5.0f, "default density 5 p/cm3");
        expectEquals(s.bz_gsm,     0.0f, "default Bz 0 nT");
        expectEquals(s.kp,         0.0f, "default Kp 0");
        df.stopThread(1000);
    }

    void testParseQuietDayWind() {
        beginTest("Parse quiet day wind (speed=412.5, density=4.8, Bz=2.1)");
        DataFetcher df;
        df.setPollIntervalMs(999'000'000);
        SpaceWeatherState out;
        bool ok = df.parseSolarWindJson(kQuietDayWind, out);
        expect(ok, "parse returns true");
        expectWithinAbsoluteError(out.velocity, 412.5f, 0.01f, "velocity");
        expectWithinAbsoluteError(out.density,    4.8f, 0.01f, "density");
        expectWithinAbsoluteError(out.bz_gsm,     2.1f, 0.01f, "Bz");
        df.stopThread(1000);
    }

    void testParseStormDayWind() {
        beginTest("Parse storm day wind (speed=712.3, Bz=-18.4)");
        DataFetcher df;
        df.setPollIntervalMs(999'000'000);
        SpaceWeatherState out;
        bool ok = df.parseSolarWindJson(kStormDayWind, out);
        expect(ok, "parse returns true");
        expectWithinAbsoluteError(out.velocity, 712.3f, 0.1f,  "velocity storm");
        expectWithinAbsoluteError(out.bz_gsm,  -18.4f, 0.01f, "Bz negative storm");
        df.stopThread(1000);
    }

    void testNullWindRetainsLastValid() {
        beginTest("Null wind fields retain last valid (quiet day values)");
        DataFetcher df;
        df.setPollIntervalMs(999'000'000);
        SpaceWeatherState state;
        df.parseSolarWindJson(kQuietDayWind, state); // populate last-valid: vel=412.5
        bool ok = df.parseSolarWindJson(kAllNullWind, state);
        expect(ok, "parse of all-null fixture returns true (array non-empty)");
        expectWithinAbsoluteError(state.velocity, 412.5f, 0.01f, "velocity retained");
        expectWithinAbsoluteError(state.density,    4.8f, 0.01f, "density retained");
        expectWithinAbsoluteError(state.bz_gsm,     2.1f, 0.01f, "Bz retained");
        df.stopThread(1000);
    }

    void testParseQuietDayKp() {
        beginTest("Parse quiet day Kp (estimated_kp=1.3)");
        DataFetcher df;
        df.setPollIntervalMs(999'000'000);
        SpaceWeatherState out;
        bool ok = df.parseKpJson(kQuietDayKp, out);
        expect(ok, "parse returns true");
        expectWithinAbsoluteError(out.kp, 1.3f, 0.01f, "Kp quiet");
        df.stopThread(1000);
    }

    void testParseStormDayKp() {
        beginTest("Parse storm day Kp (estimated_kp=8.0)");
        DataFetcher df;
        df.setPollIntervalMs(999'000'000);
        SpaceWeatherState out;
        bool ok = df.parseKpJson(kStormDayKp, out);
        expect(ok, "parse returns true");
        expectWithinAbsoluteError(out.kp, 8.0f, 0.01f, "Kp storm");
        df.stopThread(1000);
    }

    void testNullKpRetainsLastValid() {
        beginTest("Null Kp retains last valid (1.3 from quiet day)");
        DataFetcher df;
        df.setPollIntervalMs(999'000'000);
        SpaceWeatherState state;
        df.parseKpJson(kQuietDayKp, state); // lastKp = 1.3
        df.parseKpJson(kNullKp, state);     // null → retain 1.3
        expectWithinAbsoluteError(state.kp, 1.3f, 0.01f, "Kp retained after null");
        df.stopThread(1000);
    }

    void testTimestampExtracted() {
        beginTest("Timestamp extracted from wind JSON");
        DataFetcher df;
        df.setPollIntervalMs(999'000'000);
        SpaceWeatherState out;
        df.parseSolarWindJson(kQuietDayWind, out);
        expect(out.timestamp.isNotEmpty(), "timestamp non-empty");
        df.stopThread(1000);
    }
};

static DataFetcherTests dataFetcherTests;
