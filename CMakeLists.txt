cmake_minimum_required(VERSION 3.22)
project(SolarDrone VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(SolarDrone
    PLUGIN_MANUFACTURER_CODE Sldr
    PLUGIN_CODE Sldr
    BUNDLE_ID "com.solardrone.SolarDrone"
    PLUGIN_AU_EXPORT_PREFIX SolarDroneAU
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "SolarDrone"
    PLUGIN_MANUFACTURER_NAME "SolarDrone"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(SolarDrone PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/DataFetcher.cpp
    src/SynthParamMapper.cpp
    src/Interpolator.cpp
    src/OscillatorBank.cpp
    src/AdditiveEngine.cpp
    src/VisualRenderer.cpp
    src/TempoTracker.cpp
    src/BeatRepeater.cpp
    src/Chopper.cpp
    src/SunDisc.cpp
)

target_compile_definitions(SolarDrone PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Linux needs libcurl for JUCE::URL; macOS/Windows use native stacks
if(UNIX AND NOT APPLE)
    find_package(CURL REQUIRED)
    target_compile_definitions(SolarDrone PUBLIC JUCE_USE_CURL=1)
    target_link_libraries(SolarDrone PRIVATE CURL::libcurl)
else()
    target_compile_definitions(SolarDrone PUBLIC JUCE_USE_CURL=0)
endif()

target_link_libraries(SolarDrone
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ── Unit test binary ──────────────────────────────────────────────────────────
juce_add_console_app(SolarDrone_Tests
    PRODUCT_NAME "SolarDroneTests"
)

target_sources(SolarDrone_Tests PRIVATE
    tests/Main.cpp
    tests/DataFetcherTests.cpp
    tests/SynthParamMapperTests.cpp
    src/DataFetcher.cpp
    src/SynthParamMapper.cpp
)

target_include_directories(SolarDrone_Tests PRIVATE src)

target_compile_definitions(SolarDrone_Tests PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(SolarDrone_Tests PRIVATE
    juce::juce_core
    juce::juce_data_structures
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
