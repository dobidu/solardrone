#pragma once
#include "SpaceWeatherState.h"
#include "SynthParams.h"
#include "UserParams.h"

class SynthParamMapper {
public:
    static SynthParams map(const SpaceWeatherState& state,
                           const UserParams& params = UserParams{});

private:
    static float computeIntervalRatio(const UserParams& params);
};
