# ============================================================
# Unit-test console app (correctness layer — see TESTING.md §1/§3).
#
# Enabled from the main build with -DRCL_BUILD_TESTS=ON. Kept out of the default
# build so the plugin build is unaffected. Compiles the units-under-test directly
# (no plugin-client wrapper, no GUI) so it stays fast and sanitizer-friendly.
# ============================================================

juce_add_console_app(RobinControlLiteTests
    PRODUCT_NAME "Robin Control Lite Tests")

juce_generate_juce_header(RobinControlLiteTests)

set(PLUGIN_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../NewProject/Source")

target_sources(RobinControlLiteTests
    PRIVATE
        test_main.cpp
        # Units under test (pure logic, depend only on juce_core):
        ${PLUGIN_SOURCE}/DSP/RandomizationEngine.cpp
)

target_include_directories(RobinControlLiteTests
    PRIVATE
        ${PLUGIN_SOURCE}
        ${PLUGIN_SOURCE}/DSP
        ${PLUGIN_SOURCE}/Audio        # MidiMapper.h is header-only
)

target_compile_definitions(RobinControlLiteTests
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0)

target_link_libraries(RobinControlLiteTests
    PRIVATE
        juce::juce_core
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
