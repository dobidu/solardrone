#include "DataFetcher.h"

DataFetcher::DataFetcher() : juce::Thread("DataFetcher") {
    startThread();
}

DataFetcher::~DataFetcher() {
    stopThread(2000);
}

SpaceWeatherState DataFetcher::getState() const {
    juce::ScopedLock sl(lock);
    return state;
}

void DataFetcher::setPollIntervalMs(int ms) {
    pollIntervalMs = ms;
}

void DataFetcher::run() {
    while (!threadShouldExit()) {
        fetchAndUpdateState();
        wait(pollIntervalMs);
    }
}

void DataFetcher::fetchAndUpdateState() {
    SpaceWeatherState next;
    {
        juce::ScopedLock sl(lock);
        next = state;
    }

    auto windJson  = juce::URL(kWindUrl).readEntireTextStream();
    auto kpJson    = juce::URL(kKpUrl).readEntireTextStream();
    auto xrayJson  = juce::URL(kXRayUrl).readEntireTextStream();

    const bool windOk = !windJson.isEmpty() && parseSolarWindJson(windJson, next);
    const bool kpOk   = !kpJson.isEmpty()   && parseKpJson(kpJson, next);
    if (!xrayJson.isEmpty() && parseXrayJson(xrayJson, next))
        lastXRayFlux = next.x_ray_flux;
    else {
        next.x_ray_flux  = lastXRayFlux;
        next.flare_class = SpaceWeatherState::classifyFlux(lastXRayFlux);
    }

    if (windOk || kpOk) {
        next.source    = SpaceWeatherState::Source::live;
        next.data_age_s = 0;
    } else {
        next.source      = SpaceWeatherState::Source::cached;
        next.data_age_s += pollIntervalMs / 1000;
    }

    {
        juce::ScopedLock sl(lock);
        state = next;
    }
}

bool DataFetcher::parseSolarWindJson(const juce::String& json, SpaceWeatherState& out) {
    auto parsed = juce::JSON::parse(json);
    if (!parsed.isArray() || parsed.getArray()->isEmpty())
        return false;

    auto entry = (*parsed.getArray())[0];
    if (!entry.isObject())
        return false;

    auto extract = [&](const juce::Identifier& key, float& field, float& lastValid) {
        auto val = entry[key];
        if (val.isDouble() || val.isInt()) {
            field = static_cast<float>(static_cast<double>(val));
            lastValid = field;
        } else {
            field = lastValid;
        }
    };

    extract("speed",   out.velocity, lastVelocity);
    extract("density", out.density,  lastDensity);
    extract("bz_gsm",  out.bz_gsm,   lastBz);

    auto ts = entry["time_tag"];
    if (ts.isString())
        out.timestamp = ts.toString();

    return true;
}

bool DataFetcher::parseKpJson(const juce::String& json, SpaceWeatherState& out) {
    auto parsed = juce::JSON::parse(json);
    if (!parsed.isArray() || parsed.getArray()->isEmpty())
        return false;

    auto entry = (*parsed.getArray())[0];
    if (!entry.isObject())
        return false;

    auto val = entry["estimated_kp"];
    if (val.isDouble() || val.isInt()) {
        out.kp = static_cast<float>(static_cast<double>(val));
        lastKp = out.kp;
    } else {
        out.kp = lastKp;
    }

    return true;
}

bool DataFetcher::parseXrayJson(const juce::String& json, SpaceWeatherState& out) {
    auto parsed = juce::JSON::parse(json);
    if (!parsed.isArray() || parsed.getArray()->isEmpty())
        return false;

    for (const auto& item : *parsed.getArray()) {
        if (!item.isObject()) continue;
        auto energy = item["energy"];
        if (!energy.isString() || energy.toString() != "0.1-0.8nm") continue;
        auto flux = item["flux"];
        if (flux.isDouble() || flux.isInt()) {
            const float f = static_cast<float>(static_cast<double>(flux));
            if (f > 0.0f) {
                out.x_ray_flux  = f;
                out.flare_class = SpaceWeatherState::classifyFlux(f);
                lastXRayFlux    = f;
                return true;
            }
        }
    }
    return false;
}
