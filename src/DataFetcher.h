#pragma once
#include <juce_core/juce_core.h>
#include "SpaceWeatherState.h"

class DataFetcher : public juce::Thread {
public:
    DataFetcher();
    ~DataFetcher() override;

    SpaceWeatherState getState() const;
    void setPollIntervalMs(int ms);

    // Exposed for unit testing — parse fixture strings without network
    bool parseSolarWindJson(const juce::String& json, SpaceWeatherState& out);
    bool parseKpJson(const juce::String& json, SpaceWeatherState& out);

private:
    void run() override;
    void fetchAndUpdateState();

    mutable juce::CriticalSection lock;
    SpaceWeatherState state;

    float lastVelocity = 450.0f;
    float lastDensity  =   5.0f;
    float lastBz       =   0.0f;
    float lastKp       =   0.0f;

    int pollIntervalMs = 60000;

    static constexpr const char* kWindUrl =
        "https://services.swpc.noaa.gov/json/rtsw/rtsw_wind_1m.json";
    static constexpr const char* kKpUrl =
        "https://services.swpc.noaa.gov/json/planetary_k_index_1m.json";
};
