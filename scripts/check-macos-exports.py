#!/usr/bin/env python3
"""Check every slice of macOS plug-in bundles for namespace leaks/entry points.

Read-only; never loads the plug-in. JSON records can be retained before and after
signing. Any explicitly required accessibility export can be allowed by exact
symbol name; no namespace-wide exemption is supported.
"""

import argparse
import hashlib
import json
from pathlib import Path
import plistlib
import subprocess
import sys


def run(*args):
    return subprocess.run(args, capture_output=True, text=True, check=True).stdout


def inspect(bundle, expected_architectures, allowed_symbols):
    result = {"bundle": str(bundle.resolve()), "passed": False, "slices": []}
    try:
        with (bundle / "Contents/Info.plist").open("rb") as stream:
            info = plistlib.load(stream)
        executable = bundle / "Contents/MacOS" / info["CFBundleExecutable"]
        result["version"] = info.get("CFBundleShortVersionString")
        result["executable_sha256"] = hashlib.sha256(executable.read_bytes()).hexdigest()
        architectures = run("lipo", "-archs", str(executable)).split()
        if not architectures:
            raise ValueError("No Mach-O architectures found")
        missing_architectures = sorted(set(expected_architectures) - set(architectures))
        if missing_architectures:
            raise ValueError(f"Missing architectures: {missing_architectures}")
        if bundle.suffix == ".vst3":
            required = {"_GetPluginFactory", "_bundleEntry", "_bundleExit"}
        elif bundle.suffix == ".aaxplugin":
            required = {"_ACFCanUnloadNow", "_ACFGetClassFactory", "_ACFGetSDKVersion",
                        "_ACFRegisterComponent", "_ACFRegisterPlugin", "_ACFShutdown", "_ACFStartup"}
        elif bundle.suffix == ".component":
            required = {"_" + component["factoryFunction"] for component in info["AudioComponents"]}
            if not required:
                raise ValueError("No AU factory entry point recorded in Info.plist")
        else:
            raise ValueError("Expected a .vst3, .component or .aaxplugin bundle")
        for arch in architectures:
            symbols = {line.split()[-1] for line in run("nm", "-arch", arch, "-gU", str(executable)).splitlines()
                       if line.split()}
            leaked = sorted(symbol for symbol in symbols
                            if ("juce" in symbol or "moonbase" in symbol) and symbol not in allowed_symbols)
            missing = sorted(required - symbols)
            result["slices"].append({"architecture": arch, "external_symbol_count": len(symbols),
                                     "internal_export_count": len(leaked), "internal_export_examples": leaked[:12],
                                     "required_entry_points": sorted(required), "missing_entry_points": missing,
                                     "passed": not leaked and not missing})
        result["passed"] = all(item["passed"] for item in result["slices"])
    except (OSError, ValueError, KeyError, plistlib.InvalidFileException, subprocess.CalledProcessError) as ex:
        result["error"] = str(ex)
        if isinstance(ex, subprocess.CalledProcessError):
            result["tool_stderr"] = ex.stderr.strip()
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundles", nargs="+", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--expect-architecture", action="append", default=[], choices=("arm64", "x86_64"))
    parser.add_argument("--allow-symbol", action="append", default=[], help="Exact documented export exception")
    args = parser.parse_args()
    results = [inspect(bundle, args.expect_architecture, set(args.allow_symbol)) for bundle in args.bundles]
    summary = {"passed": all(item["passed"] for item in results), "bundles": results}
    rendered = json.dumps(summary, indent=2) + "\n"
    if args.output:
        args.output.write_text(rendered)
    print(rendered, end="")
    return 0 if summary["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
