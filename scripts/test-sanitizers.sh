#!/usr/bin/env bash
#
# test-sanitizers.sh — build + run the unit-test target under sanitizers (macOS/Linux, Clang).
#
# Sanitizers catch the bug CLASS behind most plugin crashes — out-of-bounds
# reads/writes, use-after-free, undefined behaviour, and data races — which the
# host validators (pluginval/auval) cannot see. Because the unit-test target is a
# plain console app, the WHOLE process is instrumented, so reports are reliable
# (unlike instrumenting a plugin dylib loaded by a non-instrumented host).
#
#   Pass 1: AddressSanitizer + UndefinedBehaviorSanitizer (combinable)
#   Pass 2: ThreadSanitizer                                (separate — can't mix with ASan)
#
# Any sanitizer hit aborts the run with a non-zero exit (halt_on_error/abort_on_error),
# so this doubles as a CI gate.
#
# NOTE: these are Clang/GCC tools — NOT MSVC. Run on macOS or Linux. Most defects
# they find are cross-platform, so catching them here also fixes Windows.
#
# COVERAGE: engine/state/decode fuzz plus DRM-enabled processor, synthetic signed
# licenses, storage and activation-controller lifecycle. No real license issued.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="${REPO_ROOT}/NewProject"
TARGET="RobinControlLiteTests"

bold() { printf "\033[1m%s\033[0m\n" "$1"; }
green() { printf "\033[32m%s\033[0m\n" "$1"; }
red() { printf "\033[31m%s\033[0m\n" "$1"; }

run_pass() {
    local label="$1" build_dir="$2" san_flags="$3"
    bold "──────────────────────────────────────────────────"
    bold " ${label}"
    bold "──────────────────────────────────────────────────"

    cmake -S "${SRC}" -B "${build_dir}" \
        -DRCL_BUILD_TESTS=ON \
        -DRCL_ENABLE_MOONBASE=ON \
        -DRCL_COPY_PLUGIN_AFTER_BUILD=OFF \
        "-DFETCHCONTENT_SOURCE_DIR_MOONBASE_CPP=${RCL_MOONBASE_SOURCE:-}" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DJUCE_AAX_SDK_PATH=/nonexistent \
        -DCMAKE_C_FLAGS="${san_flags} -fno-omit-frame-pointer -g" \
        -DCMAKE_CXX_FLAGS="${san_flags} -fno-omit-frame-pointer -g" \
        -DCMAKE_OBJC_FLAGS="${san_flags} -fno-omit-frame-pointer -g" \
        -DCMAKE_OBJCXX_FLAGS="${san_flags} -fno-omit-frame-pointer -g" \
        -DCMAKE_EXE_LINKER_FLAGS="${san_flags}" \
        > "${build_dir}.cfg.log" 2>&1 || return 1

    cmake --build "${build_dir}" --target "${TARGET}" RobinControlLiteLicensingTests -j3 > "${build_dir}.build.log" 2>&1 || return 1

    local exe
    exe="$(find "${build_dir}/tests" -name "${TARGET}" -type f -perm +111 | head -1)"
    if [[ -z "${exe}" ]]; then
        red "  build produced no executable (see ${build_dir}.build.log)"
        return 1
    fi
    "${exe}" || return 1
    "${build_dir}/tests/RobinControlLiteLicensingTests_artefacts/Debug/RobinControlLiteLicensingTests" || return 1
}

failures=0

# Pass 1 — ASan + UBSan
ASAN_OPTIONS="halt_on_error=1:abort_on_error=1:detect_leaks=0" \
UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1" \
run_pass "AddressSanitizer + UndefinedBehaviorSanitizer" \
         "${REPO_ROOT}/build-asan" \
         "-fsanitize=address,undefined -fno-sanitize-recover=all" \
    || failures=$((failures+1))

# Pass 2 — TSan
TSAN_OPTIONS="halt_on_error=1" \
run_pass "ThreadSanitizer" \
         "${REPO_ROOT}/build-tsan" \
         "-fsanitize=thread" \
    || failures=$((failures+1))

echo
if [[ "${failures}" -eq 0 ]]; then
    green "ALL SANITIZER PASSES CLEAN ✅"
else
    red "${failures} sanitizer pass(es) FAILED ❌"
    exit 1
fi
