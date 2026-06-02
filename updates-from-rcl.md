# Updates from RCL → Premium (port checklist)

**Purpose:** a running, checkbox-tracked list of changes made in **Robin Control
Lite (RCL)** that need to be ported into **Robin Control Premium (Pro)**. Check
each box off as you land it in the Pro repo (`round-robin-premium`, `NewProject/`).

**Relationship to `update-premium-spec.md`:** that file is the big narrative sync
spec (last snapshot through commit `26fb902`, 2026-05-24). **This file is the
incremental checklist for everything AFTER that** — newer work that hasn't been
folded into the spec yet. When an item here is fully ported and stable, consider
moving its summary into `update-premium-spec.md` and ticking it here.

**Source of truth = the Lite repo.** For exact diffs prefer:
```bash
git -C /Users/alex/Documents/Github/robin-control-redesign show <sha> -- <path>
```
Commit SHAs are referenced inline.

**Identity reminder — DO NOT copy these strings to Pro** (keep Pro's own): product
name, plugin code (`rcll`), manufacturer code (`Cdsp`), bundle ID, CMake target
(`RobinControlLite`). See `update-premium-spec.md §1`.

**Last updated:** 2026-06-01. Covers Lite commits `5d2f4eb`, `c488013`, `4c51b0f`.

---

## 1. Feature: Drag-and-drop sample loading  — commit `5d2f4eb`

**What it is:** the editor now accepts audio files dropped from the OS file
manager (Windows Explorer / macOS Finder), loading them into the sample pool. It
reuses the same additive "append to next empty slot" path as the Load button.

**Why it matters for Pro:** Pro should have at least feature parity. A customer
reported the absence of drag-and-drop as a bug; it had never been implemented in
any version. Pro almost certainly has the same gap.

### Implementation (files: `NewProject/Source/PluginEditor.h` / `.cpp`)

- [ ] **Make the editor a drop target.** Add `public juce::FileDragAndDropTarget`
      to the editor class's base list (Pro's editor class name differs — keep Pro's).
- [ ] **Declare the four overrides** in the header:
      `isInterestedInFileDrag`, `fileDragEnter`, `fileDragExit`, `filesDropped`.
- [ ] **Add the shared loader helper** `void addSamplesFromFiles(const juce::Array<juce::File>&)`.
      It: finds the first empty slot → for each existing file, loads it into that
      slot and advances to the next empty slot → calls `rebuildLoadedIndices()` →
      repaints the sample panel. **Additive — never clears the pool** (matches the
      Load Samples decision from 2026-04-27).
- [ ] **Refactor the existing file-chooser callback** (`addMoreSamples`) to call
      `addSamplesFromFiles(fc.getResults())` so the chooser and drag-drop share
      one code path and behave identically.
- [ ] **Add a format filter helper** `static bool isSupportedAudioFile(const juce::String&)`
      checking `.wav/.aif/.aiff/.flac/.ogg/.mp3` (lowercased extension).
      ⚠️ **Pro adaptation:** keep this list in sync with Pro's
      `SampleLoader::supportedFormats` and the file-chooser filter — Pro may support
      additional formats.
- [ ] **`isInterestedInFileDrag`** → returns true if any dragged file passes
      `isSupportedAudioFile` (so the cursor shows "droppable" only for audio).
- [ ] **`fileDragEnter` / `fileDragExit`** → toggle a `bool isFileDragHovering`
      member and `repaint()` (drives the drop-zone highlight).
- [ ] **`filesDropped`** → collect the supported files into a `juce::Array<juce::File>`,
      call `addSamplesFromFiles`, clear `isFileDragHovering`, repaint.
- [ ] **Drop-zone highlight** in `paintOverChildren` (drawn last): when
      `isFileDragHovering`, draw a rounded tint + cream border over the whole
      editor. ⚠️ **Pro adaptation:** uses Lite's `RRColors` palette — substitute
      Pro's colors.

### Slot-count / behaviour parity
- [ ] `addSamplesFromFiles` uses `NewProjectAudioProcessor::NUM_SAMPLE_SLOTS`.
      In Pro, use **Pro's** slot-count constant (Pro may have more than 20 slots).

### Known limitation to document for Pro too
- [ ] **DAW-internal-browser drops are host-dependent.** OS file-manager drops
      work everywhere; dragging from a DAW's own browser (e.g. FL Studio's) only
      works if the host forwards it as an OS file drop, which FL often does NOT.
      `FileDragAndDropTarget` is the only plugin-side hook — there's nothing more
      Pro can do about it. Set the same expectation in Pro's docs/README.

### Optional Pro enhancement (NOT in Lite — consider for Pro)
- [ ] `filesDropped` receives drop coordinates `(x, y)` that Lite ignores. Pro
      could use them to drop a file onto a **specific** slot under the cursor,
      rather than always appending to the next empty slot. Nice-to-have, not parity.

---

## 2. Build: macOS-only auto-copy (cross-platform CMake fix) — commit `c488013`

**What it is:** the plugin's `COPY_PLUGIN_AFTER_BUILD` + `VST3_COPY_DIR` /
`AU_COPY_DIR` previously hardcoded macOS paths (`$HOME/Library/Audio/Plug-Ins/...`).
Those don't exist on Windows/Linux and break Windows/CI builds. Now wrapped so the
auto-copy only applies on `if(APPLE)`; other platforms skip it and pick the
artefact from the build tree.

- [ ] In Pro's `NewProject/CMakeLists.txt`, gate `COPY_PLUGIN_AFTER_BUILD` and the
      two `*_COPY_DIR` values behind `if(APPLE)` (see the `_COPY_AFTER_BUILD` /
      `_VST3_COPY_DIR` / `_AU_COPY_DIR` variable pattern in Lite). Prerequisite for
      Pro's Windows/CI builds to configure cleanly.

---

## 3. Infrastructure: formal testing platform — commits `c488013`, `4c51b0f`

**What it is:** a layered, plugin-agnostic automated testing system. Full details
in Lite's **`TESTING.md`**; reuse instructions in its **§7**. This is the single
biggest reusable win for Pro.

Port these files (adapt names/paths per `TESTING.md §7`):
- [ ] **`scripts/test-plugin.sh`** — local pluginval (strictness 10) + auval.
      Edit the CONFIG block (Pro `PRODUCT_NAME`, AU `aumu`/subtype/manufacturer).
- [ ] **`tests/`** (`test_main.cpp` + `CMakeLists.txt`) — JUCE `UnitTest` console
      app. Repoint source paths at Pro's `Source/`; reuse the `RandomizationEngine`
      / `MidiMapper` tests (those units exist in Pro too) and add Pro-specific ones.
- [ ] **`option(RCL_BUILD_TESTS ...)` + `add_subdirectory(tests)`** block in Pro's
      CMake (rename the option, e.g. `RCP_BUILD_TESTS`).
- [ ] **`scripts/test-sanitizers.sh`** — ASan/UBSan + TSan over the test target.
      Change the `TARGET` var and source dir.
- [ ] **`.github/workflows/validate.yml`** — macOS+Windows pluginval, unit-test,
      and sanitizer jobs. Change artefact target name + `-S` source dir.
- [ ] **`.gitignore`** entries for `build-tests/ build-asan/ build-tsan/ .tools/`.
- [ ] **`TESTING.md`** — copy and update its §6 (Pro's own diagnostics) and
      identity references.
- [ ] Update Pro's **`CLAUDE.md`** testing section to point at the ported platform
      (Lite's `4c51b0f` shows the wording).

### Highest-value follow-up (applies to both Lite and Pro)
- [ ] Extend `tests/` to instantiate the audio processor and fuzz
      `processBlock` / `setStateInformation` with adversarial inputs (oversized
      buffers, sample-rate flips, zero channels, malformed state). This turns the
      sanitizer pass into a real crash-class hunter. Not yet done in Lite — do it
      once and port, or port the harness pattern when Lite has it.

---

## 4. Bug fixes — none to port yet

- The reported **FL Studio (Windows) "crashes on any button"** issue is **NOT
  fixed** in Lite v1.0.1 — root cause is still pending the customer's Windows
  crash dump. When a fix lands in Lite, add it here with its commit SHA and port
  it to Pro (Pro is equally likely affected, being the same codebase + Windows).

---

## How to use this file

1. Work top-down; tick boxes as each item lands and is verified in Pro.
2. For exact code, `git show <sha> -- <path>` against the Lite repo beats
   re-deriving from prose.
3. When a whole section is done, summarize it into `update-premium-spec.md` and
   note "(ported, see update-premium-spec §X)" here.
4. Add new RCL changes to the top of the relevant section with their commit SHA as
   they happen, so this stays the live RCL→Pro delta.
