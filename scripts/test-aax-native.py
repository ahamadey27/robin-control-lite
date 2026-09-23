#!/usr/bin/env python3
"""Run all installed DigiShell tests applicable to Robin Control Lite AAX Native.

Requires the macOS Avid validator toolkit and unrestricted local helper sockets.
Does not modify the toolkit, install the plugin, sign it, or add customer DRM.
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import sys


def result_passed(output, test_id):
    """DigiShell's exit code alone does not indicate validation success."""
    ids = re.findall(r"^\s+id: ([\w.]+)\s*$", output, re.MULTILINE)
    statuses = re.findall(r"result_status: (\w+)", output)
    errors = re.search(
        r"message_type: (?:error|cmd_error)|\bERROR:|\bABORTED\b|"
        r"Test stage completed and FAILED|[1-9]\d* failed(?: to complete)?|"
        r"failed:\s*[1-9]\d*|ServeTests error|Runsuite error", output)
    return ids == [test_id] and statuses == ["E_COMPLETED_PASS"] and not errors


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path)
    parser.add_argument("--dsh", type=Path, default=Path.home() /
                        "SDKs/aax-validator-dsh-2024-6-0/CommandLineTools/dsh")
    parser.add_argument("--output", type=Path, required=True,
                        help="New directory for raw logs and summary.json")
    args = parser.parse_args()
    bundle = args.bundle.resolve(strict=True)
    dsh = args.dsh.resolve(strict=True)
    if bundle.suffix != ".aaxplugin" or not bundle.is_dir():
        parser.error("Provide one .aaxplugin bundle, not a directory of plugins")
    # DigiShell has no character escaping; reject ambiguous command input.
    if any(c in str(bundle) for c in ('"', '\n', '\r', '\\')):
        parser.error("DigiShell cannot safely quote this bundle path")
    args.output.mkdir(parents=True, exist_ok=False)
    results = {"bundle": str(bundle), "dsh": str(dsh), "tests": {}, "excluded": {}}

    def run(name, commands):
        proc = subprocess.Popen([str(dsh)], stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                start_new_session=True)
        try:
            raw, _ = proc.communicate((commands + "\nexit\n").encode(), timeout=900)
        except subprocess.TimeoutExpired:
            os.killpg(proc.pid, signal.SIGKILL)
            raw, _ = proc.communicate()
            (args.output / (name + ".log")).write_bytes(raw)
            raise RuntimeError(f"{name}: timed out; helper processes terminated")
        (args.output / (name + ".log")).write_bytes(raw)
        output = raw.decode("utf-8", errors="replace").replace("\0", "")
        if proc.returncode:
            raise RuntimeError(f"{name}: DigiShell exited {proc.returncode}")
        return output

    try:
        # Fail closed before excluding the DSP-only check. This runner is scoped
        # to Lite's single effect with two Native types and no DSP/HDX types.
        description = run("native-description", 'load_dish aaxh\ninit debug_max\n'
                          f'loadpi "{bundle}"\nlisteffects 0\n'
                          'getdescription {plugin: 0, effect: 0, stringformat: yaml}')
        effects = re.findall(r"effectID: (\S+)", description)
        if (effects != ["dsp.conduit.RobinControlLite"]
                or description.count("key: AAX_eProperty_PlugInID_Native") != 2
                or "AAX_eProperty_PlugInID_TI" in description
                or "AAX_CProcessProc_TI" in description
                or "message_type: error" in description):
            raise RuntimeError("Could not confirm Lite's Native-only descriptor; no tests excluded")
        executable = bundle / "Contents/MacOS/Robin Control Lite"
        results["executable_sha256"] = hashlib.sha256(executable.read_bytes()).hexdigest()
        results["page_table_sha256"] = hashlib.sha256(
            (bundle / "Contents/Resources/RobinControlLitePages.xml").read_bytes()).hexdigest()
        listing = run("test-list", "load_dish aaxval\nlisttests")
        tests = re.findall(r"^\s+id: ((?:test|info)\.[\w.]+)\s*$", listing, re.MULTILINE)
        required = {"test.page_table.load", "test.page_table.automation_list",
                    "test.describe_validation", "test.data_model", "test.load_unload",
                    "test.parameters", "test.parameter_traversal.linear",
                    "test.parameter_traversal.random", "test.parameter_traversal.random.fast"}
        if not required.issubset(tests) or len(tests) != len(set(tests)):
            raise RuntimeError("Missing or duplicate validator tests; inspect test-list.log")
        for test in tests:
            if test == "test.cycle_counts":
                reason = "N/A: Avid's AAX DSP cycle-count test requires HDX; Lite has only Native types"
                results["excluded"][test] = reason
                print(f"{test}: {reason}", flush=True)
                continue
            print(f"Running {test}...", flush=True)
            output = run(test, f'load_dish aaxval\nruntest [{test}, "{bundle}"]')
            passed = result_passed(output, test)
            results["tests"][test] = "PASS" if passed else "FAIL"
            print(f"{test}: {results['tests'][test]}", flush=True)
        results["passed"] = bool(results["tests"]) and all(
            value == "PASS" for value in results["tests"].values())
    except (RuntimeError, OSError) as exc:
        results["passed"] = False
        results["error"] = str(exc)
        print(str(exc), file=sys.stderr)
    finally:
        (args.output / "summary.json").write_text(json.dumps(results, indent=2) + "\n")
    return 0 if results["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
