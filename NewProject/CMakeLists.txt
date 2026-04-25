cmake_minimum_required(VERSION 3.22)
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS version")
project(RobinControlLite VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# JUCE — local checkout if available, FetchContent otherwise
#
# Override the local path with: cmake -DJUCE_PATH=/path/to/JUCE -B build
# CI / fresh clones with no JUCE installed will fall through to FetchContent.
# ============================================================
set(JUCE_PATH "$ENV{HOME}/Documents/JUCE" CACHE PATH "Path to JUCE root")

if(EXISTS "${JUCE_PATH}/CMakeLists.txt")
    message(STATUS "Using local JUCE at ${JUCE_PATH}")
    add_subdirectory("${JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE")
else()
    message(STATUS "Local JUCE not found at ${JUCE_PATH} — fetching via FetchContent")
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# ============================================================
# Plugin target
# ============================================================
juce_add_plugin(RobinControlLite
    COMPANY_NAME                    "conduit.dsp"
    BUNDLE_ID                       "dsp.conduit.RobinControlLite"
    PLUGIN_CODE                     "rcll"
    PLUGIN_MANUFACTURER_CODE        "Cdsp"
    PRODUCT_NAME                    "Robin Control Lite"

    IS_SYNTH                        TRUE
    NEEDS_MIDI_INPUT                TRUE
    NEEDS_MIDI_OUTPUT               FALSE
    IS_MIDI_EFFECT                  FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS     FALSE
    COPY_PLUGIN_AFTER_BUILD         TRUE
    VST3_COPY_DIR                   "$ENV{HOME}/Library/Audio/Plug-Ins/VST3"
    AU_COPY_DIR                     "$ENV{HOME}/Library/Audio/Plug-Ins/Components"

    FORMATS                         VST3 AU Standalone
)

# Generate JuceHeader.h (required because source files use #include <JuceHeader.h>)
juce_generate_juce_header(RobinControlLite)

# ============================================================
# Binary resources (logos, images)
# ============================================================
juce_add_binary_data(RobinControlLite_BinaryData
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../LOGOS/FULLLOGO-standard-clear.png
        ${CMAKE_CURRENT_SOURCE_DIR}/../LOGOS/FULLLOGO-email-white.png
)

# ============================================================
# Source files — grouped to mirror the on-disk folder structure
# NOTE: RRvoice uses lowercase 'v' to match the actual filename
# ============================================================

set(SOURCE_ROOT
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

set(SOURCE_AUDIO
    Source/Audio/RRvoice.cpp
    Source/Audio/RRvoice.h
    Source/Audio/RRSound.cpp
    Source/Audio/RRSound.h
    Source/Audio/MidiMapper.h
)

set(SOURCE_DATA
    Source/Data/SampleLoader.cpp
    Source/Data/SampleLoader.h
    Source/Data/SampleSlot.h
)

set(SOURCE_DSP
    Source/DSP/RandomizationEngine.cpp
    Source/DSP/RandomizationEngine.h
    Source/DSP/ThreeBandEQ.cpp
    Source/DSP/ThreeBandEQ.h
    Source/DSP/TransientShaper.cpp
    Source/DSP/TransientShaper.h
    Source/DSP/ToneControl.cpp
    Source/DSP/ToneControl.h
    Source/DSP/RandomAlgorithmTable.h
)

set(SOURCE_PARAMETERS
    Source/Parameters/ParametersIDs.h
    Source/Parameters/RandomizationParams.h
)

set(SOURCE_UI
    Source/UI/RRLookAndFeel.cpp
    Source/UI/RRLookAndFeel.h
    Source/UI/SampleManagerPanel.cpp
    Source/UI/SampleManagerPanel.h
    Source/UI/DualThumbRndSlider.h
)

target_sources(RobinControlLite
    PRIVATE
        ${SOURCE_ROOT}
        ${SOURCE_AUDIO}
        ${SOURCE_DATA}
        ${SOURCE_DSP}
        ${SOURCE_PARAMETERS}
        ${SOURCE_UI}
)

# Mirrors the folder structure in Xcode's project navigator
source_group("Source"            FILES ${SOURCE_ROOT})
source_group("Source\\Audio"     FILES ${SOURCE_AUDIO})
source_group("Source\\Data"      FILES ${SOURCE_DATA})
source_group("Source\\DSP"       FILES ${SOURCE_DSP})
source_group("Source\\Parameters" FILES ${SOURCE_PARAMETERS})
source_group("Source\\UI"        FILES ${SOURCE_UI})

# ============================================================
# Include directories
# ============================================================
target_include_directories(RobinControlLite
    PRIVATE
        Source
        Source/Audio
        Source/Data
        Source/DSP
        Source/Parameters
        Source/UI
)

# ============================================================
# Compile definitions
# ============================================================
target_compile_definitions(RobinControlLite
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
    PRIVATE
        JUCE_STRICT_REFCOUNTEDPOINTER=1
)

# ============================================================
# Linked libraries — all modules from RobinControlLite.jucer
# ============================================================
target_link_libraries(RobinControlLite
    PRIVATE
        RobinControlLite_BinaryData
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
