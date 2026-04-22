cmake_minimum_required(VERSION 3.22)

project(robin-control-redesign VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# JUCE Path Configuration
#
# Default matches the path used in the original Xcode export:
#   NewProject module path ../../juce  =>  <repo-root>/../juce
#
# Override from the command line if needed:
#   cmake -DJUCE_PATH=/path/to/JUCE -B build
# ============================================================
set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../juce" CACHE PATH
    "Path to JUCE root directory")

if(NOT EXISTS "${JUCE_PATH}/CMakeLists.txt")
    message(FATAL_ERROR
        "\nJUCE not found at '${JUCE_PATH}'.\n"
        "Set JUCE_PATH to the root of your JUCE installation, e.g.:\n"
        "  cmake -DJUCE_PATH=/path/to/JUCE -B build\n")
endif()

add_subdirectory("${JUCE_PATH}" "${CMAKE_BINARY_DIR}/juce")

add_subdirectory(NewProject)
