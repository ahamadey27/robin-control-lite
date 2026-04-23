#include "SampleManagerPanel.h"
#include "../PluginProcessor.h"

SampleManagerPanel::SampleManagerPanel(NewProjectAudioProcessor& p)
    : processor(p)
{
    loadSamplesButton.setButtonText("Load Samples");
    loadSamplesButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3e5a40));
    loadSamplesButton.setColour(juce::TextButton::textColourOnId, RRColors::screenPrint);
    loadSamplesButton.setLookAndFeel(&buttonLAF);
    loadSamplesButton.onClick = [this]() { if (onLoadSamplesClicked) onLoadSamplesClicked(); };
    addAndMakeVisible(loadSamplesButton);

    clearSamplesButton.setButtonText("Clear");
    clearSamplesButton.setColour(juce::TextButton::buttonColourId, RRColors::s612RedDim);
    clearSamplesButton.setColour(juce::TextButton::textColourOnId, RRColors::screenPrint);
    clearSamplesButton.setLookAndFeel(&buttonLAF);
    clearSamplesButton.onClick = [this]() { if (onClearSamplesClicked) onClearSamplesClicked(); };
    addAndMakeVisible(clearSamplesButton);

    resetPoolButton.setButtonText("Reset");
    resetPoolButton.setColour(juce::TextButton::buttonColourId, RRColors::s612Red);
    resetPoolButton.setColour(juce::TextButton::textColourOnId, RRColors::screenPrint);
    resetPoolButton.setLookAndFeel(&buttonLAF);
    resetPoolButton.onClick = [this]() { processor.resetPlaybackPosition(); };
    addAndMakeVisible(resetPoolButton);

    playbackModeButton.setButtonText("Series");
    playbackModeButton.setClickingTogglesState(true);
    playbackModeButton.onClick = [this]()
        {
            bool isRandom = playbackModeButton.getToggleState();
            playbackModeButton.setButtonText(isRandom ? "Random" : "Series");
        };
    playbackModeButton.setLookAndFeel(&toggleLAF);
    addAndMakeVisible(playbackModeButton);

    playbackModeAttachment = std::make_unique<ButtonAttachment>(
        p.apvts, ParameterIDs::playbackMode, playbackModeButton);
}

SampleManagerPanel::~SampleManagerPanel()
{
    playbackModeButton.setLookAndFeel(nullptr);
    loadSamplesButton.setLookAndFeel(nullptr);
    clearSamplesButton.setLookAndFeel(nullptr);
    resetPoolButton.setLookAndFeel(nullptr);
}

juce::String SampleManagerPanel::truncateName(const juce::String& name, int maxChars)
{
    if (name.length() <= maxChars)
        return name;

    auto dot = name.lastIndexOfChar('.');
    juce::String ext = (dot >= 0) ? name.substring(dot) : "";
    int available = maxChars - ext.length() - 3;
    if (available < 1) available = 1;
    return name.substring(0, available) + "..." + ext;
}

void SampleManagerPanel::paint(juce::Graphics& g)
{
    // ── Section title ────────────────────────────────────────────────────────
    {
        juce::Font sectionFont(juce::FontOptions(16.0f));
        sectionFont = sectionFont.boldened().withExtraKerningFactor(0.04f);

        juce::GlyphArrangement ga;
        ga.addFittedText(sectionFont, "SAMPLE POOL",
                         8.0f, 6.0f, (float)(getWidth() - 16), 18.0f,
                         juce::Justification::left, 1);
        juce::Path path;
        ga.createPath(path);

        g.setColour(juce::Colour(0xff0a0806));
        g.strokePath(path, juce::PathStrokeType(1.0f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour(RRColors::ampCol);
        g.fillPath(path);
    }

    // ── Playback type label above toggle ─────────────────────────────────────
    g.setColour(juce::Colour(0xff1f160d));
    g.setFont(juce::Font(juce::FontOptions(9.0f)).boldened());
    g.drawText("PLAYBACK TYPE", getWidth() - 168, 16, 160, 10, juce::Justification::centred);

    // ── LED Screen area ─────────────────────────────────────────────────────
    constexpr int screenY    = 55;
    constexpr int screenPad  = 6;
    const int screenH = getHeight() - screenY - 4;
    const auto screenRect = juce::Rectangle<int>(screenPad, screenY,
                                                  getWidth() - screenPad * 2, screenH);

    // Outer bezel (recessed look)
    g.setColour(juce::Colour(0xff080810));
    g.fillRoundedRectangle(screenRect.toFloat().expanded(1.0f), 3.0f);

    // Screen background — deep blue LCD (matches LED Scren.png reference)
    g.setColour(RRColors::lcdBg);
    g.fillRoundedRectangle(screenRect.toFloat(), 2.0f);

    // Subtle distress — pseudo-random specks (fixed seed = deterministic)
    {
        juce::Random rng(12345);
        const int spots = (screenRect.getWidth() * screenRect.getHeight()) / 380;
        for (int s = 0; s < spots; ++s)
        {
            int px = screenRect.getX() + rng.nextInt(screenRect.getWidth());
            int py = screenRect.getY() + rng.nextInt(screenRect.getHeight());
            g.setColour(rng.nextBool()
                ? RRColors::lcdBgDark.withAlpha(0.28f)
                : juce::Colour(0xff4a80d0).withAlpha(0.18f));
            g.fillRect(px, py, 1, 1);
        }
    }

    // Subtle inner shadow (top edge darker)
    {
        juce::ColourGradient shadow(
            RRColors::lcdBgDark.withAlpha(0.6f), 0.0f, (float)screenRect.getY(),
            juce::Colours::transparentBlack,     0.0f, (float)screenRect.getY() + 8.0f,
            false);
        g.setGradientFill(shadow);
        g.fillRect(screenRect.getX() + 1, screenRect.getY() + 1,
                   screenRect.getWidth() - 2, 8);
    }

    // Scanline overlay (darker blue lines for LCD texture)
    g.setColour(RRColors::lcdBgDark.withAlpha(0.18f));
    for (int sy = screenRect.getY(); sy < screenRect.getBottom(); sy += 2)
        g.fillRect(screenRect.getX(), sy, screenRect.getWidth(), 1);

    // Screen border highlight (subtle lighter blue rim)
    g.setColour(juce::Colour(0xff5a80c8));
    g.drawRoundedRectangle(screenRect.toFloat(), 2.0f, 1.0f);

    // ── LCD colors (white-on-blue scheme) ───────────────────────────────────
    const juce::Colour ledGreen    = RRColors::lcdText;        // sample names (white)
    const juce::Colour ledDim      = RRColors::lcdTextDim;     // dim state
    const juce::Colour ledAmber    = RRColors::lcdHighlight;   // icons (pale blue highlight)
    const juce::Colour ledRed      = RRColors::lcdRed;         // delete warning

    // ── Sample list (two columns, 10 rows each) ────────────────────────────
    constexpr int listY      = 62;     // inside the screen
    constexpr int btnSize    = 16;
    constexpr int btnGap     = 2;
    constexpr int btnsW      = btnSize * 4 + btnGap * 3;
    constexpr int padX       = 14;     // inside screen padding
    constexpr int colGap     = 10;
    constexpr int rowsPerCol = 10;

    // Dynamic row height: fill available space
    const int rowH = (getHeight() - listY - 4) / rowsPerCol;

    const int halfW = (getWidth() - padX * 2 - colGap) / 2;
    const int nameW = halfW - btnsW - 4;
    const int maxChars = juce::jmax(12, nameW / 7);

    // LED-style monospace font (slightly larger)
    juce::Font ledFont(juce::FontOptions("Courier New", 12.5f, juce::Font::plain));

    rowHitAreas.clear();

    for (int i = 0; i < NewProjectAudioProcessor::NUM_SAMPLE_SLOTS; ++i)
    {
        int col = i / rowsPerCol;
        int row = i % rowsPerCol;

        int colX = padX + col * (halfW + colGap);
        int y = listY + row * rowH;

        if (processor.sampleSlots[i].isLoaded)
        {
            // Highlight drop target (swap)
            if (isDragging && !dragIsInsert && dragTargetSlot == i)
            {
                g.setColour(RRColors::lcdBgDark.brighter(0.15f));
                g.fillRect(colX, y, halfW, rowH);
            }

            // Sample name — LED green
            juce::String slotNum = juce::String(i + 1).paddedLeft('0', 2);
            juce::String fileName = processor.sampleSlots[i].sourceFile.getFileName();
            juce::String display = slotNum + ". " + truncateName(fileName, maxChars);

            g.setColour(isDragging && dragSourceSlot == i ? ledDim : ledGreen);
            g.setFont(ledFont);
            g.drawText(display, colX, y, nameW, rowH, juce::Justification::centredLeft);

            // Action buttons — LED-colored icons
            int bx = colX + nameW + 4;
            RowHitAreas hit;
            hit.slotIndex = i;
            hit.nameArea = { colX, y, nameW, rowH };
            hit.rowArea  = { colX, y, halfW, rowH };

            hit.reorderBtn = { bx, y, btnSize, btnSize };
            g.setColour(ledDim.brighter(0.3f));
            g.setFont(juce::Font(juce::FontOptions(11.0f)));
            g.drawText(juce::String::charToString(0x2725), hit.reorderBtn, juce::Justification::centred);
            bx += btnSize + btnGap;

            hit.playBtn = { bx, y, btnSize, btnSize };
            g.setColour(ledGreen.withAlpha(0.7f));
            g.drawText(juce::String::charToString(0x25B6), hit.playBtn, juce::Justification::centred);
            bx += btnSize + btnGap;

            hit.replaceBtn = { bx, y, btnSize, btnSize };
            g.setColour(ledAmber.withAlpha(0.7f));
            g.drawText(juce::String::charToString(0x21C4), hit.replaceBtn, juce::Justification::centred);
            bx += btnSize + btnGap;

            hit.deleteBtn = { bx, y, btnSize, btnSize };
            g.setColour(ledRed.withAlpha(0.7f));
            g.drawText(juce::String::charToString(0x2715), hit.deleteBtn, juce::Justification::centred);

            rowHitAreas.push_back(hit);

            // Draw insertion line if dragging between rows
            if (isDragging && dragIsInsert && dragTargetSlot == i)
            {
                g.setColour(ledGreen);
                g.fillRect(colX, y - 1, halfW, 2);
            }
        }
    }

    // ── "Add more" text ─────────────────────────────────────────────────────
    int loadedCount = 0;
    for (int i = 0; i < NewProjectAudioProcessor::NUM_SAMPLE_SLOTS; ++i)
        if (processor.sampleSlots[i].isLoaded) ++loadedCount;

    int emptySlots = NewProjectAudioProcessor::NUM_SAMPLE_SLOTS - loadedCount;
    if (emptySlots > 0)
    {
        int nextSlot = loadedCount;
        int col = nextSlot / rowsPerCol;
        int row = nextSlot % rowsPerCol;
        int colX = padX + col * (halfW + colGap);
        int addY = listY + row * rowH + 4;

        juce::String addText = "--- click to add " + juce::String(emptySlots).paddedLeft('0', 2) + " samples ---";
        g.setColour(ledDim);
        g.setFont(ledFont);
        g.drawText(addText, colX, addY, halfW, rowH, juce::Justification::centredLeft);

        addMoreArea = { colX, addY, halfW, rowH };
    }
    else
    {
        addMoreArea = {};
    }
}

void SampleManagerPanel::resized()
{
    constexpr int headerY = 24;
    loadSamplesButton .setBounds(8, headerY, 76, 26);
    clearSamplesButton.setBounds(88, headerY, 46, 26);
    resetPoolButton   .setBounds(138, headerY, 46, 26);
    // Horizontal toggle: needs ~160px wide for labels + pill, 20px tall
    playbackModeButton.setBounds(getWidth() - 168, headerY + 3, 160, 20);
}

int SampleManagerPanel::getSlotAtPosition(juce::Point<int> pos, bool& isInsertGap) const
{
    isInsertGap = false;

    for (auto& hit : rowHitAreas)
    {
        if (hit.rowArea.contains(pos))
        {
            int relY = pos.y - hit.rowArea.getY();
            if (relY < 4)
                isInsertGap = true;
            return hit.slotIndex;
        }
    }
    return -1;
}

void SampleManagerPanel::mouseDown(const juce::MouseEvent& e)
{
    auto pos = e.getPosition();

    for (auto& hit : rowHitAreas)
    {
        if (hit.reorderBtn.contains(pos))
        {
            isDragging = true;
            dragSourceSlot = hit.slotIndex;
            dragTargetSlot = -1;
            dragIsInsert = false;
            dragPos = pos;
            return;
        }
        if (hit.playBtn.contains(pos))
        {
            if (onAuditionSample) onAuditionSample(hit.slotIndex);
            return;
        }
        if (hit.replaceBtn.contains(pos))
        {
            if (onReplaceSample) onReplaceSample(hit.slotIndex);
            return;
        }
        if (hit.deleteBtn.contains(pos))
        {
            deleteSample(hit.slotIndex);
            return;
        }
    }

    if (addMoreArea.contains(pos) && onAddMoreClicked)
        onAddMoreClicked();
}

void SampleManagerPanel::mouseDrag(const juce::MouseEvent& e)
{
    if (!isDragging) return;

    dragPos = e.getPosition();
    bool isInsert = false;
    int target = getSlotAtPosition(dragPos, isInsert);

    if (target == dragSourceSlot)
        target = -1;

    dragTargetSlot = target;
    dragIsInsert = isInsert;
    repaint();
}

void SampleManagerPanel::mouseUp(const juce::MouseEvent& e)
{
    if (!isDragging) return;

    if (dragTargetSlot >= 0 && dragSourceSlot >= 0 && dragTargetSlot != dragSourceSlot)
    {
        if (dragIsInsert)
            processor.insertSample(dragSourceSlot, dragTargetSlot);
        else
            processor.swapSamples(dragSourceSlot, dragTargetSlot);
    }

    isDragging = false;
    dragSourceSlot = -1;
    dragTargetSlot = -1;
    repaint();
}

void SampleManagerPanel::mouseMove(const juce::MouseEvent& e)
{
    auto pos = e.getPosition();
    juce::String newTooltip;

    for (auto& hit : rowHitAreas)
    {
        if (hit.nameArea.contains(pos))
        {
            // Show full filename as tooltip if it was truncated
            juce::String fullName = processor.sampleSlots[hit.slotIndex].sourceFile.getFileName();
            int maxChars = juce::jmax(14, hit.nameArea.getWidth() / 7);
            if (fullName.length() > maxChars)
                newTooltip = fullName;
            break;
        }
    }

    if (newTooltip != currentTooltip)
    {
        currentTooltip = newTooltip;
        // TooltipWindow picks up changes via getTooltip() automatically
    }
}

void SampleManagerPanel::mouseExit(const juce::MouseEvent&)
{
    currentTooltip = {};
}

void SampleManagerPanel::deleteSample(int slotIndex)
{
    processor.sampleLoader.clearSlot(slotIndex);

    for (int i = slotIndex; i < NewProjectAudioProcessor::NUM_SAMPLE_SLOTS - 1; ++i)
    {
        if (processor.sampleSlots[i + 1].isLoaded)
        {
            processor.sampleSlots[i] = processor.sampleSlots[i + 1];
            processor.sampleSlots[i + 1].clear();
        }
        else
        {
            break;
        }
    }

    processor.rebuildLoadedIndices();
    processor.reshuffleIndices();
    processor.sampleLoader.updateSynthesiserSounds();
    repaint();
}
