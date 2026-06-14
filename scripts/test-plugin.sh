#!/usr/bin/env bash
#
# test-plugin.sh — local automated validation for a JUCE plugin (macOS).
#
# Runs the free, industry-standard validators against the built plugin:
#   1. pluginval   (Tracktion)  — VST3 + AU, out-of-process, strictness 1-10
#   2. auval       (Apple)      — AU component conformance (built into macOS)
#
# It is intentionally plugin-agnostic: every plugin-specific value is a variable
# at the top, so this script can be copied verbatim into any JUCE plugin repo
# (e.g. the Pro version) and only the CONFIG block needs editing.
#
# Usage:
#   scripts/test-plugin.sh                 # default strictness (10)
#   scripts/test-plugin.sh 5               # custom strictness level
#   STRICTNESS=8 scripts/test-plugin.sh    # via env var
#
# Exit code is non-zero if any validator fails, so it doubles as a CI gate.

set -euo pipefail

# ============================================================================
# CONFIG — the only block to edit when reusing this for another plugin
# ============================================================================
PRODUCT_NAME="Robin Control Lite"          # PRODUCT_NAME from CMakeLists.txt
AU_TYPE="aumu"                             # aumu=instrument, aufx=effect, aumf=music effect
AU_SUBTYPE="rcll"                          # PLUGIN_CODE
AU_MANUF="Cdsp"                            # PLUGIN_MANUFACTURER_CODE
# ============================================================================

STRICTNESS="${1:-${STRICTNESS:-10}}"

VST3_PATH="$HOME/Library/Audio/Plug-Ins/VST3/${PRODUCT_NAME}.vst3"
AU_PATH="$HOME/Library/Audio/Plug-Ins/Components/${PRODUCT_NAME}.component"

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TOOLS_DIR="${REPO_ROOT}/.tools"
PLUGINVAL_BIN="${TOOLS_DIR}/pluginval.app/Contents/MacOS/pluginval"

bold() { printf "\033[1m%s\033[0m\n" "$1"; }
green() { printf "\033[32m%s\033[0m\n" "$1"; }
red() { printf "\033[31m%s\033[0m\n" "$1"; }

# ----------------------------------------------------------------------------
# 1. Ensure pluginval is available (download the prebuilt binary once)
# ----------------------------------------------------------------------------
ensure_pluginval() {
    if [[ -x "${PLUGINVAL_BIN}" ]]; then
        return
    fi
    bold "pluginval not found locally — downloading latest release…"
    mkdir -p "${TOOLS_DIR}"
    local url="https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_macOS.zip"
    curl -sL "${url}" -o "${TOOLS_DIR}/pluginval_macOS.zip"
    unzip -oq "${TOOLS_DIR}/pluginval_macOS.zip" -d "${TOOLS_DIR}"
    rm -f "${TOOLS_DIR}/pluginval_macOS.zip"
    # Strip quarantine so it runs without a Gatekeeper prompt
    xattr -dr com.apple.quarantine "${TOOLS_DIR}/pluginval.app" 2>/dev/null || true
    if [[ ! -x "${PLUGINVAL_BIN}" ]]; then
        red "ERROR: pluginval download/extract failed."
        exit 1
    fi
}

# ----------------------------------------------------------------------------
# 2. Run pluginval against one plugin file
# ----------------------------------------------------------------------------
run_pluginval() {
    local path="$1"
    local label="$2"
    if [[ ! -e "${path}" ]]; then
        red "  SKIP ${label}: not found at ${path} (did you build first?)"
        return 1
    fi
    bold "── pluginval (strictness ${STRICTNESS}) → ${label}"
    # --validate-in-process is NOT used: out-of-process keeps a plugin crash
    # from taking down the validator, which is the whole point.
    "${PLUGINVAL_BIN}" \
        --strictness-level "${STRICTNESS}" \
        --validate "${path}" \
        --skip-gui-tests \
        --output-dir "${TOOLS_DIR}/pluginval-logs"
}

# ----------------------------------------------------------------------------
# 3. Run Apple's auval against the AU
# ----------------------------------------------------------------------------
run_auval() {
    bold "── auval → ${PRODUCT_NAME} (${AU_TYPE} ${AU_SUBTYPE} ${AU_MANUF})"
    auval -v "${AU_TYPE}" "${AU_SUBTYPE}" "${AU_MANUF}"
}

# ----------------------------------------------------------------------------
main() {
    bold "=========================================================="
    bold " Validating: ${PRODUCT_NAME}   (strictness ${STRICTNESS})"
    bold "=========================================================="

    ensure_pluginval

    local failures=0

    run_pluginval "${VST3_PATH}" "VST3" || failures=$((failures+1))
    run_pluginval "${AU_PATH}"   "AU"   || failures=$((failures+1))
    run_auval                          || failures=$((failures+1))

    echo
    if [[ "${failures}" -eq 0 ]]; then
        green "ALL VALIDATORS PASSED ✅"
    else
        red "${failures} validator(s) FAILED ❌  (see logs in ${TOOLS_DIR}/pluginval-logs)"
        exit 1
    fi
}

main "$@"
