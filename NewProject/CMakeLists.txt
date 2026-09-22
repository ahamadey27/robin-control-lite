cmake_minimum_required(VERSION 3.22)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
project(RobinControlLite VERSION 2.0.0)

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
        URL https://github.com/juce-framework/JUCE/archive/91ad83ae34a81e0833b1a2b0866f54846370ae53.tar.gz
        URL_HASH SHA256=04f8d5055382582c757be9da069ea98338005f98248facd9c2804435ac853e70
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# ============================================================
# AAX SDK — opt in by setting JUCE_AAX_SDK_PATH (defaults to ~/SDKs/aax-sdk-2-9-0).
# Builds fall back to VST3/AU only when the SDK isn't present, so fresh clones
# and Windows boxes without the SDK still configure cleanly.
# Override per invocation: cmake -DJUCE_AAX_SDK_PATH=/path/to/aax-sdk -B build
# ============================================================
set(JUCE_AAX_SDK_PATH "$ENV{HOME}/SDKs/aax-sdk-2-9-0" CACHE PATH "Path to AAX SDK root")

if(EXISTS "${JUCE_AAX_SDK_PATH}/Interfaces/AAX.h")
    message(STATUS "AAX SDK found at ${JUCE_AAX_SDK_PATH} — enabling AAX format")
    juce_set_aax_sdk_path("${JUCE_AAX_SDK_PATH}")
    set(PLUGIN_FORMATS VST3 AU AAX)
else()
    message(STATUS "AAX SDK not found at ${JUCE_AAX_SDK_PATH} — building VST3/AU only")
    set(PLUGIN_FORMATS VST3 AU)
endif()

# ============================================================
# Plugin target
# ============================================================
option(RCL_BUILD_STANDALONE "Build the standalone app for local testing (not distributed)" OFF)
if(RCL_BUILD_STANDALONE)
    list(APPEND PLUGIN_FORMATS Standalone)
endif()

# Auto-copy install dirs are macOS-only — the paths below ($HOME/Library/...)
# don't exist on Windows/Linux. On those platforms we skip the copy and let
# tooling (CI validators, the installer) pick the artefact straight out of the
# build tree. Keeps the local macOS dev loop identical while making Windows/CI
# builds configure and link cleanly.
set(_COPY_AFTER_BUILD FALSE)
set(_VST3_COPY_DIR "")
set(_AU_COPY_DIR   "")
if(APPLE)
    set(_COPY_AFTER_BUILD TRUE)
    set(_VST3_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3")
    set(_AU_COPY_DIR   "$ENV{HOME}/Library/Audio/Plug-Ins/Components")
endif()
option(RCL_COPY_PLUGIN_AFTER_BUILD "Install local plugin builds into user plugin folders" ${_COPY_AFTER_BUILD})

juce_add_plugin(RobinControlLite
    VERSION                         ${PROJECT_VERSION}
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
    COPY_PLUGIN_AFTER_BUILD         ${RCL_COPY_PLUGIN_AFTER_BUILD}
    VST3_COPY_DIR                   "${_VST3_COPY_DIR}"
    AU_COPY_DIR                     "${_AU_COPY_DIR}"

    FORMATS                         ${PLUGIN_FORMATS}
)

# Generate JuceHeader.h (required because source files use #include <JuceHeader.h>)
juce_generate_juce_header(RobinControlLite)

# Unlicensed builds are for engine development / historical beta reproduction.
option(RCL_ENABLE_MOONBASE "Enable Moonbase customer licensing (required for final release)" OFF)
option(RCL_FINAL_RELEASE "Enforce final-release customer licensing requirements" OFF)
if(RCL_FINAL_RELEASE AND NOT RCL_ENABLE_MOONBASE)
    message(FATAL_ERROR "Final releases require RCL_ENABLE_MOONBASE=ON. Beta 1 uses RCL_FINAL_RELEASE=OFF.")
endif()
if(RCL_ENABLE_MOONBASE)
    include(FetchContent)
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    FetchContent_Declare(moonbase_cpp
        URL https://github.com/Moonbase-sh/moonbase-cpp/archive/refs/tags/v4.4.0.tar.gz
        URL_HASH SHA256=c3f2ea70dabb40a71cc0502fa07d82e18e2e6fdbd548165d2ec78272c327dede
        SOURCE_SUBDIR modules)
    FetchContent_MakeAvailable(moonbase_cpp)
    if(APPLE)
        enable_language(OBJC OBJCXX)
    endif()
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/MoonbaseHardening.cmake")
    juce_add_module("${RCL_MOONBASE_MODULE}")
    target_link_libraries(RobinControlLite PRIVATE moonbase_licensing)
    target_sources(RobinControlLite PRIVATE Source/Licensing/MoonbaseLicense.cpp)
    foreach(format IN LISTS PLUGIN_FORMATS)
        if(NOT TARGET RobinControlLite_${format})
            continue() # JUCE omits AU on Windows.
        endif()
        if(APPLE)
            target_sources(RobinControlLite_${format} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/MoonbaseNotices.txt")
            set_source_files_properties(Resources/MoonbaseNotices.txt PROPERTIES
                HEADER_FILE_ONLY TRUE MACOSX_PACKAGE_LOCATION Resources)
        elseif(WIN32 AND NOT format STREQUAL "Standalone")
            add_custom_command(TARGET RobinControlLite_${format} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:RobinControlLite_${format}>/../Resources"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/Resources/MoonbaseNotices.txt"
                    "$<TARGET_FILE_DIR:RobinControlLite_${format}>/../Resources/MoonbaseNotices.txt"
                VERBATIM)
        endif()
    endforeach()
    message(STATUS "RCL customer licensing: Moonbase ON")
else()
    message(STATUS "RCL customer licensing: OFF (Beta 1 / development only)")
endif()
target_compile_definitions(RobinControlLite PUBLIC RCL_ENABLE_MOONBASE=$<BOOL:${RCL_ENABLE_MOONBASE}>)

# Carry the exact licensing profile into signed bundles and installer checks.
set(RCL_BUILD_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/RCLBuildConfig.txt")
file(WRITE "${RCL_BUILD_CONFIG}"
    "version=${PROJECT_VERSION}\nmoonbase=${RCL_ENABLE_MOONBASE}\nfinal=${RCL_FINAL_RELEASE}\nmoonbase_sdk=4.4.0-rcl-hardening-1\n")
foreach(format IN LISTS PLUGIN_FORMATS)
    if(NOT TARGET RobinControlLite_${format})
        continue()
    endif()
    if(APPLE)
        target_sources(RobinControlLite_${format} PRIVATE "${RCL_BUILD_CONFIG}")
        set_source_files_properties("${RCL_BUILD_CONFIG}" PROPERTIES
            HEADER_FILE_ONLY TRUE MACOSX_PACKAGE_LOCATION Resources)
    elseif(WIN32 AND NOT format STREQUAL "Standalone")
        add_custom_command(TARGET RobinControlLite_${format} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:RobinControlLite_${format}>/../Resources"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${RCL_BUILD_CONFIG}"
                "$<TARGET_FILE_DIR:RobinControlLite_${format}>/../Resources/RCLBuildConfig.txt"
            VERBATIM)
    endif()
endforeach()

# AAX's automation page table is a bundle resource, not embedded BinaryData.
if(TARGET RobinControlLite_AAX)
    set(RCL_AAX_PAGE_TABLE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/AAX/RobinControlLitePages.xml")
    if(APPLE)
        target_sources(RobinControlLite_AAX PRIVATE "${RCL_AAX_PAGE_TABLE}")
        set_source_files_properties("${RCL_AAX_PAGE_TABLE}" PROPERTIES
            HEADER_FILE_ONLY TRUE MACOSX_PACKAGE_LOCATION Resources)
    elseif(WIN32)
        set_property(TARGET RobinControlLite_AAX APPEND PROPERTY LINK_DEPENDS "${RCL_AAX_PAGE_TABLE}")
        add_custom_command(TARGET RobinControlLite_AAX POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:RobinControlLite_AAX>/../Resources"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${RCL_AAX_PAGE_TABLE}"
                "$<TARGET_FILE_DIR:RobinControlLite_AAX>/../Resources/RobinControlLitePages.xml"
            VERBATIM)
    endif()
endif()

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
        JUCE_USE_MP3AUDIOFORMAT=1
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

# ============================================================
# Unit tests (correctness layer). Off by default so the plugin build is
# unaffected; enable with:  cmake -DRCL_BUILD_TESTS=ON ...
# ============================================================
option(RCL_BUILD_TESTS "Build the Robin Control Lite unit-test console app" OFF)
if(RCL_BUILD_TESTS)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../tests" "${CMAKE_BINARY_DIR}/tests")
endif()
