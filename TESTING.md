# TESTING.md — Formal plugin testing for Robin Control Lite

This is the testing playbook for Robin Control Lite (and, by design, any future
`conduit.dsp` JUCE plugin — see [§7 Reusing this for another plugin](#7-reusing-this-for-another-plugin)).

It exists because **v1.0.0 shipped an FL-Studio-on-Windows crash that never
appeared on the developer's Mac** — the Windows build was never validated on
Windows. The whole point of what follows is to make that class of bug
*impossible to ship unnoticed*.

> **Mental model:** validators (pluginval, auval, VST3 Validator) are
> **necessary but not sufficient**. They catch ~80% of host-compatibility bugs
> mechanically. The last ~20% (real DAW quirks, GUI/threading edge cases) still
> needs real-host smoke testing and, post-ship, crash telemetry. A plugin can
> pass pluginval at strictness 10 and still crash in Logic or FL.
> ([source](https://melatonin.dev/blog/pluginval-is-a-plugin-devs-best-friend/))

---

## 0. TL;DR — the layered pipeline

| Layer | Tool | Status | Catches |
|---|---|---|---|
| 1. Local host validation | `scripts/test-plugin.sh` (pluginval + auval) | ✅ wired & passing | Most VST3/AU host-compat bugs, lifecycle, state, automation |
| 2. Unit / correctness tests | `tests/` (JUCE UnitTest) via `-DRCL_BUILD_TESTS=ON` | ✅ wired & passing | Logic regressions (randomization ranges, MIDI mapping, …) |
| 3. Sanitizers (bug-class) | `scripts/test-sanitizers.sh` (ASan/UBSan + TSan) | ✅ wired & clean | OOB reads/writes, use-after-free, UB, data races |
| 4. Cross-platform CI | `.github/workflows/validate.yml` | ✅ written, ⏳ push to activate | **Windows** build + validation without owning Windows |
| 5. Real-host smoke matrix | Manual checklist (§5) | ⏳ per-release | GUI/drag-drop, the FL-class quirks validators miss |
| 6. Field crash telemetry | Crashpad → Sentry (§6) | ⏳ deferred (privacy decision) | Crashes on machines you'll never see |

Layers 1–4 are **already set up and verified in this repo**. Run them locally with:

```bash
cd NewProject && cmake -B build && cmake --build build && cd ..   # build plugin
scripts/test-plugin.sh        # layer 1: pluginval + auval
scripts/test-sanitizers.sh    # layer 3: ASan/UBSan + TSan over the unit tests
```
(Layer 2 builds + runs inside `scripts/test-sanitizers.sh`; to run the plain
unit tests on their own, see §1b.)

---

## 1. Layer 1 — Local validation (set up, run now)

### What's installed
- `scripts/test-plugin.sh` — downloads pluginval once into `.tools/`, then runs:
  - **pluginval** (strictness 10) against the installed VST3 and AU
  - **auval** (Apple's AU conformance checker, built into macOS)

### Run it
```bash
# Build first (installs VST3+AU to ~/Library/Audio/Plug-Ins on macOS)
cd NewProject && cmake -B build && cmake --build build && cd ..

# Validate
scripts/test-plugin.sh           # strictness 10 (default)
scripts/test-plugin.sh 5         # quicker, host-compat minimum
```
Exit code is non-zero on any failure, so it doubles as a pre-commit gate.

### What pluginval actually does ([source](https://github.com/Tracktion/pluginval))
- Runs **out-of-process** — a plugin crash is reported, not fatal to the run.
- **Strictness 1–10.** Level **5** is the minimum meaningful host-compat bar;
  **8–10** adds parameter fuzzing, automation hammering, and **repeated
  state save/restore** — the exact stress a host like FL applies.
- Catches classic JUCE bugs, e.g. a parameter listener referenced after
  destruction (a missing `removeListener` in a destructor).

### auval
```bash
auval -v aumu rcll Cdsp     # type / subtype(PLUGIN_CODE) / manufacturer(MANUF_CODE)
```
Apple AU validation. Logic/GarageBand will refuse a plugin that fails auval, so
this is a hard gate for the AU build.

### (Optional) Steinberg VST3 Validator
Ships inside the VST3 SDK (`bin/validator`). CLI host that runs a VST3
conformance check; build-server friendly.
([source](https://steinbergmedia.github.io/vst3_dev_portal/pages/What+is+the+VST+3+SDK/Validator.html))
pluginval already covers most of what it checks, so treat it as a belt-and-braces
extra rather than a requirement.

---

## 1b. Unit / correctness tests (set up, passing)

`tests/test_main.cpp` is a console app built on **JUCE's built-in `UnitTest`
framework** (no extra dependencies). It tests pure-logic units that host
validators can't reason about — currently `RandomizationEngine` (asymmetric
neg/pos ranges, determinism per seed, bounds) and `MidiMapper` (note mapping,
range guards). Add a new `juce::UnitTest` subclass per unit as the engine grows.

### Build & run
```bash
cmake -S NewProject -B build-tests -DRCL_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -DJUCE_AAX_SDK_PATH=/nonexistent
cmake --build build-tests --target RobinControlLiteTests -j3
./build-tests/tests/RobinControlLiteTests_artefacts/Debug/RobinControlLiteTests
```
Exit code is non-zero on any failed test → CI gate.

The target is **off by default** (`RCL_BUILD_TESTS=ON` to enable) so the plugin
build is untouched. It compiles the units-under-test directly — no plugin-client
wrapper, no GUI — which keeps it fast and, crucially, **sanitizer-friendly** (the
whole process is instrumented; see §3).

> **Next increment:** extend this target to instantiate `NewProjectAudioProcessor`
> and hammer `processBlock` / `setStateInformation` with adversarial inputs
> (oversized buffers, sample-rate flips, zero channels, malformed state). That
> turns it into an in-process fuzz harness for the *crash* class — and §3's
> sanitizers then cover those paths automatically.

---

## 2. Layer 2 — CI validation (set up, needs a push)

`.github/workflows/validate.yml` builds on **macOS and Windows** and runs
pluginval (strictness 10) on each. This is the single most important addition for
this project: **it validates the Windows build on Windows, every commit, with no
Windows machine.**

- JUCE is pulled via FetchContent automatically (no local JUCE on the runner).
- AAX is skipped automatically (SDK not on the runner).
- Built plugins are uploaded as downloadable artifacts on every run.

### To activate
```bash
git add .github/workflows/validate.yml
git commit -m "Add cross-platform plugin validation CI"
git push
```
Then watch the **Actions** tab on GitHub. A red X = a host-compat regression
caught before a user sees it.

> The structure mirrors the community-standard [Pamplejuce](https://github.com/sudara/pamplejuce)
> template, which builds + pluginval's JUCE plugins across macOS/Windows/Linux on
> GitHub Actions. If this project ever outgrows the hand-rolled workflow,
> Pamplejuce is the upgrade path.

---

## 3. Layer 3 — Sanitizers (set up, clean)

Sanitizers are compiler instrumentation that catch the bug *class* behind most
crashes — defects validators can't see. **`scripts/test-sanitizers.sh`** builds
the unit-test target (§1b) fully instrumented and runs it in two passes:

```bash
scripts/test-sanitizers.sh
#  Pass 1: AddressSanitizer + UndefinedBehaviorSanitizer
#  Pass 2: ThreadSanitizer   (separate — can't combine with ASan)
```
Any sanitizer hit aborts with a non-zero exit → CI gate. These are
**Clang/GCC** tools (not MSVC), so run on macOS/Linux; most defects they find are
cross-platform, so catching them on Mac still fixes Windows.

| Sanitizer | Finds |
|---|---|
| **ASan** (Address) | Buffer overflows, use-after-free, the OOB sample reads flagged in the audit |
| **TSan** (Thread) | Data races — e.g. `setStateInformation` ↔ `processBlock` |
| **UBSan** (Undefined Behavior) | Signed overflow, bad casts, null deref |
| **RTSan** (Realtime, Clang 20+) | `[[clang::nonblocking]]` on `processBlock` + `-fsanitize=realtime` → **allocations, locks, syscalls inside the audio callback** ([source](https://clang.llvm.org/docs/RealtimeSanitizer.html)) |

> The script's value scales with the test target's coverage. Right now it
> instruments the pure-logic units; once the target instantiates the processor
> (§1b "Next increment"), the same script covers `processBlock` and state I/O —
> the actual crash-class paths — for free.

RTSan also has a **static** companion, Clang's Function Effect Analysis
(`[[clang::nonblocking]]`), which flags RT-unsafe calls at compile time.
([source](https://clang.llvm.org/docs/FunctionEffectAnalysis.html)) Adopt it when
you move to a Clang-20+ toolchain.

---

## 4. Layer 4 — Real-host smoke matrix (the FL-class bugs)

Validators don't open the GUI, drag files, or reproduce host-specific wrappers.
Keep a short manual checklist per release. You don't need to *own* every DAW:

| DAW | How to test without buying | Priority |
|---|---|---|
| **FL Studio** | Free **trial** (fully functional except saving) — **highest priority, it's where the bug was reported** | ★★★ |
| Reaper | Free evaluation (unlimited) | ★★★ |
| Ableton Live | Free trial | ★★ |
| Logic / GarageBand | GarageBand is free on macOS; covers the AU path | ★★ |
| Cubase | Free Cubase LE/AI if bundled, or trial | ★ |
| Pro Tools | Free Pro Tools Intro (AAX) | ★ |

### Per-host smoke checklist
- [ ] Plugin scans/loads without crashing
- [ ] GUI opens; **click every button** (Load, Save, Trigger, Panic, About)
- [ ] Load a sample via the file chooser
- [ ] **Drag a file in from Explorer/Finder** (works), and from the DAW browser (host-dependent)
- [ ] Play notes; automate a parameter
- [ ] Save the project, reload it — sample + parameters restore
- [ ] Change sample rate (44.1k → 48k → 96k) and buffer size mid-session

### FL-Studio-specific knobs to test ([Image-Line KB](https://support.image-line.com/action/knowledgebase/?ans=145))
FL exposes wrapper toggles that change how it drives plugins. Test with each:
- **"Use fixed size buffers"** — on/off. Off means variable block sizes; your
  `processBlock` must be block-size-agnostic (it reads `buffer.getNumSamples()`,
  so it is — keep it that way).
- **"Allow threaded processing"** — on/off. Off serializes; on stresses
  thread-safety.
- **"Make bridged"** — runs the plugin in a separate process. A frequent crash
  source *and* sometimes the only way FL browser drag-drop reaches a plugin.

> **Known FL/JUCE precedent:** [JUCE #1193](https://github.com/juce-framework/JUCE/issues/1193)
> — FL's *Patcher* triggered a null-pointer crash in JUCE's VST3 buffer mapping
> (`ClientBufferMapperData`). It was **JUCE-7-only and is fixed in JUCE 8** (this
> project's version), but it confirms FL's VST3 buffer handling is a real crash
> surface. If a crash recurs, test the **AU** and test **outside Patcher** to
> isolate it.

---

## 5. Layer 5 — Crash telemetry for shipped builds

Once shipped, you can't attach a debugger to a user's machine. Bake in automatic
crash reporting so the *next* crash arrives with a symbolicated stack instead of
a vague email.

- **Crashpad** (Google, open-source) runs an **out-of-process** handler beside
  the plugin; on crash it writes a **minidump** and uploads it.
  ([source](https://docs.sentry.io/platforms/native/configuration/backends/crashpad/))
- **Sentry** ingests those minidumps and symbolicates them (upload your PDB/dSYM
  symbols). ([source](https://docs.sentry.io/platforms/native/guides/minidumps/))
- ⚠️ The out-of-process handler can fail under **sandboxed / App-Store** hosts;
  use Sentry's in-process backend there.
- ⚠️ Respect the privacy policy — `Privacy.md` states the plugin collects
  nothing. Crash telemetry changes that; either disclose it (opt-in) or keep it
  out of the shipped Lite build. **Decide before enabling.**

---

## 6. Diagnosing the current FL Studio (Windows) crash report

The customer reports: **crashes when hitting any button; won't accept dropped
files; on Windows.** Status:

- **Drag-and-drop** was never implemented (no `FileDragAndDropTarget`). Added in
  v1.0.1 — accepts Explorer/Finder drops; DAW-browser drops remain host-dependent.
- **"Any button" crash** is not reproducible from static analysis (handlers are
  trivial, LookAndFeel lifetime is correct). It is almost certainly
  Windows-build-specific.

**To root-cause, get from the customer:**
1. The Windows minidump: `%LOCALAPPDATA%\Image-Line\FL Studio\Crashlogs\`
   (or `Documents\Image-Line\FL Studio\Crashlogs\`).
2. FL Studio version + Windows version.
3. Whether the plugin runs **bridged** (VST wrapper settings → Processing →
   "Make bridged"), and whether toggling it changes the crash.
4. VST3 vs (if applicable) other format, and whether it's inside **Patcher**.

Then: reproduce with the **FL Studio trial on Windows** + the CI Windows build,
and run **pluginval on Windows** (the new CI job) against the same binary.

---

## 7. Reusing this for another plugin (e.g. the Pro version)

Everything here is plugin-agnostic by design:

- **`scripts/test-plugin.sh`** — edit only the `CONFIG` block at the top
  (`PRODUCT_NAME`, `AU_TYPE`, `AU_SUBTYPE`, `AU_MANUF`). Copy the file as-is.
- **`scripts/test-sanitizers.sh`** — change only `TARGET` and the source dir.
- **`tests/`** — the `juce_add_console_app` + `juce::UnitTest` pattern is
  identical for any JUCE plugin; point the source paths at the new repo and write
  new `UnitTest` subclasses. Gate it from the main CMake with the same
  `RCL_BUILD_TESTS` option block.
- **`.github/workflows/validate.yml`** — change the artefact target name and the
  `-S` source dir. The pluginval/auval/unit-test/sanitizer jobs are identical for
  every JUCE plugin.
- **`TESTING.md`** — copy and update §6.

For a shared setup across multiple plugins, lift `scripts/` + `.github/` into a
template repo (or a git submodule), or adopt [Pamplejuce](https://github.com/sudara/pamplejuce)
as the common foundation. The validators, sanitizers, and crash-telemetry choices
above are the same regardless of which plugin you point them at.

---

## Sources

- pluginval — https://github.com/Tracktion/pluginval
- pluginval limitations (Melatonin) — https://melatonin.dev/blog/pluginval-is-a-plugin-devs-best-friend/
- Steinberg VST3 Validator — https://steinbergmedia.github.io/vst3_dev_portal/pages/What+is+the+VST+3+SDK/Validator.html
- Pamplejuce CI template — https://github.com/sudara/pamplejuce
- FL Studio plugin wrapper settings — https://support.image-line.com/action/knowledgebase/?ans=145
- JUCE #1193 (FL Patcher / VST3 buffer crash) — https://github.com/juce-framework/JUCE/issues/1193
- RealtimeSanitizer — https://clang.llvm.org/docs/RealtimeSanitizer.html
- Clang Function Effect Analysis — https://clang.llvm.org/docs/FunctionEffectAnalysis.html
- Sentry Crashpad backend — https://docs.sentry.io/platforms/native/configuration/backends/crashpad/
- Sentry minidumps — https://docs.sentry.io/platforms/native/guides/minidumps/
