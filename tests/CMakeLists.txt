# ============================================================
# Unit-test + fuzz console app (correctness + crash-class layer — see TESTING.md §1/§3).
#
# Enabled from the main build with -DRCL_BUILD_TESTS=ON. Kept out of the default
# build so the plugin build is unaffected.
#
# Two layers live in this one console target so the sanitizer runner
# (scripts/test-sanitizers.sh) and CI instrument BOTH with no script changes:
#   1. Pure-logic units (RandomizationEngine, MidiMapper) — fast, dependency-light.
#   2. A headless NewProjectAudioProcessor harness that fuzzes processBlock /
#      setStateInformation (the crash-class paths). Because it's a plain console
#      app, the WHOLE process is instrumented by ASan/UBSan/TSan.
#
# Instantiating the processor headlessly means pulling in the full plugin source
# and JUCE module set, but NOT juce_audio_plugin_client — so we hand-define the
# handful of JucePlugin_* macros our source references (normally emitted by
# juce_add_plugin). Keep these in sync with NewProject/CMakeLists.txt's
# juce_add_plugin(...) identity block.
# ============================================================

juce_add_console_app(RobinControlLiteTests
    PRODUCT_NAME "Robin Control Lite Tests")

juce_generate_juce_header(RobinControlLiteTests)

set(PLUGIN_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../NewProject/Source")

target_sources(RobinControlLiteTests
    PRIVATE
        test_main.cpp
        # Full plugin source — the processor harness instantiates the real engine.
        ${PLUGIN_SOURCE}/PluginProcessor.cpp
        ${PLUGIN_SOURCE}/PluginEditor.cpp
        ${PLUGIN_SOURCE}/Audio/RRvoice.cpp
        ${PLUGIN_SOURCE}/Audio/RRSound.cpp
        ${PLUGIN_SOURCE}/Data/SampleLoader.cpp
        ${PLUGIN_SOURCE}/DSP/RandomizationEngine.cpp
        ${PLUGIN_SOURCE}/DSP/ThreeBandEQ.cpp
        ${PLUGIN_SOURCE}/DSP/TransientShaper.cpp
        ${PLUGIN_SOURCE}/DSP/ToneControl.cpp
        ${PLUGIN_SOURCE}/UI/RRLookAndFeel.cpp
        ${PLUGIN_SOURCE}/UI/SampleManagerPanel.cpp
)

target_include_directories(RobinControlLiteTests
    PRIVATE
        ${PLUGIN_SOURCE}
        ${PLUGIN_SOURCE}/Audio
        ${PLUGIN_SOURCE}/Data
        ${PLUGIN_SOURCE}/DSP
        ${PLUGIN_SOURCE}/Parameters
        ${PLUGIN_SOURCE}/UI
)

target_compile_definitions(RobinControlLiteTests
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_USE_MP3AUDIOFORMAT=1
        RCL_ENABLE_MOONBASE=0 # Engine/fuzz tests deliberately exercise unrestricted DSP.
        # JucePlugin_* macros normally emitted by juce_add_plugin(). The processor
        # and editor source reference these directly; mirror the plugin identity.
        JucePlugin_Name="Robin Control Lite"
        JucePlugin_IsSynth=1
        JucePlugin_WantsMidiInput=1
        JucePlugin_ProducesMidiOutput=0
        JucePlugin_IsMidiEffect=0
        JucePlugin_VersionString="${PROJECT_VERSION}")

target_link_libraries(RobinControlLiteTests
    PRIVATE
        RobinControlLite_BinaryData
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

if(RCL_ENABLE_MOONBASE)
    # Reuse the actual DRM-enabled plugin implementation, including its gate.
    juce_add_console_app(RobinControlLiteLicensingTests PRODUCT_NAME "RCL Licensing Tests")
    target_sources(RobinControlLiteLicensingTests PRIVATE licensing_tests.cpp)
    if(APPLE)
        target_sources(RobinControlLiteLicensingTests PRIVATE licensing_test_app.mm)
    endif()
    target_include_directories(RobinControlLiteLicensingTests PRIVATE
        $<TARGET_PROPERTY:RobinControlLite,INCLUDE_DIRECTORIES>)
    target_compile_definitions(RobinControlLiteLicensingTests PRIVATE
        $<TARGET_PROPERTY:RobinControlLite,COMPILE_DEFINITIONS>
        RCL_LICENSE_FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/licensing-fixtures")
    target_link_libraries(RobinControlLiteLicensingTests PRIVATE RobinControlLite)
endif()
