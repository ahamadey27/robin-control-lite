#include "PluginProcessor.h"
#include "PluginEditor.h"

NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor(NewProjectAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    semitoneAttachment(p.apvts, ParameterIDs::semitone, semitoneSlider),
    fineTuneAttachment(p.apvts, ParameterIDs::fineTune, fineTuneSlider),
    volumeAttachment(p.apvts, ParameterIDs::volume, volumeSlider),
    panAttachment(p.apvts, ParameterIDs::pan, panSlider),
    toneLowAttachment(p.apvts, ParameterIDs::toneLow, toneLowSlider),
    toneHighAttachment(p.apvts, ParameterIDs::toneHigh, toneHighSlider),
    sampleStartAttachment(p.apvts, ParameterIDs::sampleStart, sampleStartSlider),
    sampleEndAttachment(p.apvts, ParameterIDs::sampleEnd, sampleEndSlider),
    randomAlgorithmAttachment(p.apvts, ParameterIDs::randomAlgorithm, randomAlgorithmSlider),
    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //lowGainAttachment(p.apvts, ParameterIDs::lowGain, lowGainSlider),
    //lowFreqAttachment(p.apvts, ParameterIDs::lowFreq, lowFreqSlider),
    //midGainAttachment(p.apvts, ParameterIDs::midGain, midGainSlider),
    //midFreqAttachment(p.apvts, ParameterIDs::midFreq, midFreqSlider),
    //highGainAttachment(p.apvts, ParameterIDs::highGain, highGainSlider),
    //highFreqAttachment(p.apvts, ParameterIDs::highFreq, highFreqSlider),
    //transientAttackAttachment(p.apvts, ParameterIDs::transientAttack, transientAttackSlider),
    //transientDecayAttachment(p.apvts, ParameterIDs::transientDecay, transientDecaySlider),
    //envAttackAttachment(p.apvts, ParameterIDs::envAttack, envAttackSlider),
    //envDecayAttachment(p.apvts, ParameterIDs::envDecay, envDecaySlider),
    semitoneRndNegAttachment(p.apvts, ParameterIDs::semitoneRndNeg, semitoneRndNegSlider),
    semitoneRndPosAttachment(p.apvts, ParameterIDs::semitoneRndPos, semitoneRndPosSlider),
    fineTuneRndNegAttachment(p.apvts, ParameterIDs::fineTuneRndNeg, fineTuneRndNegSlider),
    fineTuneRndPosAttachment(p.apvts, ParameterIDs::fineTuneRndPos, fineTuneRndPosSlider),
    volumeRndNegAttachment(p.apvts, ParameterIDs::volumeRndNeg, volumeRndNegSlider),
    volumeRndPosAttachment(p.apvts, ParameterIDs::volumeRndPos, volumeRndPosSlider),
    panRndNegAttachment(p.apvts, ParameterIDs::panRndNeg, panRndNegSlider),
    panRndPosAttachment(p.apvts, ParameterIDs::panRndPos, panRndPosSlider),
    toneLowRndNegAttachment(p.apvts, ParameterIDs::toneLowRndNeg, toneLowRndNegSlider),
    toneLowRndPosAttachment(p.apvts, ParameterIDs::toneLowRndPos, toneLowRndPosSlider),
    toneHighRndNegAttachment(p.apvts, ParameterIDs::toneHighRndNeg, toneHighRndNegSlider),
    toneHighRndPosAttachment(p.apvts, ParameterIDs::toneHighRndPos, toneHighRndPosSlider),
    sampleStartRndNegAttachment(p.apvts, ParameterIDs::sampleStartRndNeg, sampleStartRndNegSlider),
    sampleStartRndPosAttachment(p.apvts, ParameterIDs::sampleStartRndPos, sampleStartRndPosSlider),
    sampleEndRndNegAttachment(p.apvts, ParameterIDs::sampleEndRndNeg, sampleEndRndNegSlider),
    sampleEndRndPosAttachment(p.apvts, ParameterIDs::sampleEndRndPos, sampleEndRndPosSlider),
    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //lowGainRndNegAttachment(p.apvts, ParameterIDs::lowGainRndNeg, lowGainRndNegSlider),
    //lowGainRndPosAttachment(p.apvts, ParameterIDs::lowGainRndPos, lowGainRndPosSlider),
    //lowFreqRndNegAttachment(p.apvts, ParameterIDs::lowFreqRndNeg, lowFreqRndNegSlider),
    //lowFreqRndPosAttachment(p.apvts, ParameterIDs::lowFreqRndPos, lowFreqRndPosSlider),
    //midGainRndNegAttachment(p.apvts, ParameterIDs::midGainRndNeg, midGainRndNegSlider),
    //midGainRndPosAttachment(p.apvts, ParameterIDs::midGainRndPos, midGainRndPosSlider),
    //midFreqRndNegAttachment(p.apvts, ParameterIDs::midFreqRndNeg, midFreqRndNegSlider),
    //midFreqRndPosAttachment(p.apvts, ParameterIDs::midFreqRndPos, midFreqRndPosSlider),
    //highGainRndNegAttachment(p.apvts, ParameterIDs::highGainRndNeg, highGainRndNegSlider),
    //highGainRndPosAttachment(p.apvts, ParameterIDs::highGainRndPos, highGainRndPosSlider),
    //highFreqRndNegAttachment(p.apvts, ParameterIDs::highFreqRndNeg, highFreqRndNegSlider),
    //highFreqRndPosAttachment(p.apvts, ParameterIDs::highFreqRndPos, highFreqRndPosSlider),
    //transAtkRndNegAttachment(p.apvts, ParameterIDs::transientAttackRndNeg, transAtkRndNegSlider),
    //transAtkRndPosAttachment(p.apvts, ParameterIDs::transientAttackRndPos, transAtkRndPosSlider),
    //transDecRndNegAttachment(p.apvts, ParameterIDs::transientDecayRndNeg, transDecRndNegSlider),
    //transDecRndPosAttachment(p.apvts, ParameterIDs::transientDecayRndPos, transDecRndPosSlider),
    //envAtkRndNegAttachment(p.apvts, ParameterIDs::envAttackRndNeg, envAtkRndNegSlider),
    //envAtkRndPosAttachment(p.apvts, ParameterIDs::envAttackRndPos, envAtkRndPosSlider),
    //envDecRndNegAttachment(p.apvts, ParameterIDs::envDecayRndNeg, envDecRndNegSlider),
    //envDecRndPosAttachment(p.apvts, ParameterIDs::envDecayRndPos, envDecRndPosSlider),
    sampleManagerPanel(p)
{
    // Wire sample manager panel callbacks
    sampleManagerPanel.onLoadSamplesClicked = [this]() { loadSamplesFromFiles(); };
    sampleManagerPanel.onClearSamplesClicked = [this]()
        {
            for (int i = 0; i < NewProjectAudioProcessor::NUM_SAMPLE_SLOTS; ++i)
                audioProcessor.sampleLoader.clearSlot(i);

            audioProcessor.rebuildLoadedIndices();
            audioProcessor.reshuffleIndices();
            audioProcessor.sampleLoader.updateSynthesiserSounds();
            sampleManagerPanel.repaint();
        };
    sampleManagerPanel.onAddMoreClicked = [this]() { addMoreSamples(); };
    sampleManagerPanel.onReplaceSample = [this](int slotIndex)
        {
            fileChooser = std::make_unique<juce::FileChooser>(
                "Replace Sample", juce::File{}, "*.wav;*.aif;*.aiff;*.flac;*.ogg");
            fileChooser->launchAsync(
                juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                [this, slotIndex](const juce::FileChooser& fc)
                {
                    auto file = fc.getResult();
                    if (file.existsAsFile())
                    {
                        audioProcessor.sampleLoader.loadSample(slotIndex, file);
                        audioProcessor.rebuildLoadedIndices();
                        sampleManagerPanel.repaint();
                    }
                });
        };
    sampleManagerPanel.onAuditionSample = [this](int slotIndex)
        {
            audioProcessor.auditionSample(slotIndex);
        };

    // Trigger button — original muted red
    triggerButton.setButtonText("Trigger");
    triggerButton.setColour(juce::TextButton::buttonColourId, RRColors::s612RedDim);
    triggerButton.setColour(juce::TextButton::textColourOnId, RRColors::screenPrint);
    triggerButton.setLookAndFeel(&buttonLAF);
    triggerButton.onClick = [this]() { audioProcessor.requestTrigger(); };
    addAndMakeVisible(triggerButton);

    // Panic — original bright red
    panicButton.setButtonText("!");
    panicButton.setColour(juce::TextButton::buttonColourId, RRColors::s612Red);
    panicButton.setColour(juce::TextButton::textColourOnId, RRColors::screenPrint);
    panicButton.setLookAndFeel(&buttonLAF);
    panicButton.onClick = [this]() { audioProcessor.requestPanic(); };
    addAndMakeVisible(panicButton);

    // About button — muted warm gray accent strip
    aboutButton.setButtonText("?");
    aboutButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff5a5448));
    aboutButton.setColour(juce::TextButton::textColourOnId, RRColors::screenPrint);
    aboutButton.setLookAndFeel(&buttonLAF);
    aboutButton.onClick = [this]
        {
            aboutWindow.setTopLeftPosition(getWidth() / 2 - 170, getHeight() / 2 - 100);
            addAndMakeVisible(aboutWindow);
            aboutWindow.toFront(true);
            repaint();
        };

    addAndMakeVisible(aboutButton);
    aboutWindow.addComponentListener(this);

    // User Presets — muted dark green accent (distinct from red actions)
    savePresetButton.setButtonText("Save");
    savePresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3e5a40));
    savePresetButton.setColour(juce::TextButton::textColourOnId, RRColors::screenPrint);
    savePresetButton.setLookAndFeel(&buttonLAF);
    savePresetButton.onClick = [this]() { savePreset(); };
    addAndMakeVisible(savePresetButton);

    loadPresetButton.setButtonText("Load");
    loadPresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3e5a40));
    loadPresetButton.setColour(juce::TextButton::textColourOnId, RRColors::screenPrint);
    loadPresetButton.setLookAndFeel(&buttonLAF);
    loadPresetButton.onClick = [this]() { loadPreset(); };
    addAndMakeVisible(loadPresetButton);

    // REMOVED FOR LITE: samplesInfoLabel
    //samplesInfoLabel.setText("No samples loaded", juce::dontSendNotification);
    //samplesInfoLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    //addAndMakeVisible(samplesInfoLabel);

    addAndMakeVisible(sampleManagerPanel);

    // Label setup:
    //playbackTypeLabel.setText("Playback Type", juce::dontSendNotification);
    //playbackTypeLabel.setFont(juce::Font(juce::FontOptions(10.0f)));
    //playbackTypeLabel.setColour(juce::Label::textColourId, juce::Colour(180, 180, 195));
    //playbackTypeLabel.setJustificationType(juce::Justification::centred);
    //addAndMakeVisible(playbackTypeLabel);

    // Main parameter knobs — visible, on the editor
    for (auto* s : { &semitoneSlider, &fineTuneSlider, &volumeSlider, &panSlider,
                     &toneLowSlider, &toneHighSlider,
                     &sampleStartSlider, &sampleEndSlider
                     // COMMENTED FOR LITE — ACTIVE IN PREMIUM
                     //,&lowGainSlider,  &lowFreqSlider,
                     //,&midGainSlider,  &midFreqSlider,
                     //,&highGainSlider, &highFreqSlider,
                     //,&transientAttackSlider, &transientDecaySlider,
                     //,&envAttackSlider, &envDecaySlider
                     })
        setupKnob(*s);

    // Random Algorithm knob — larger, integer snapping, no randomization arc
    randomAlgorithmSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    randomAlgorithmSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 16);
    randomAlgorithmSlider.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(randomAlgorithmSlider);

    // Rnd sliders — hidden, APVTS-bound, values read by paintOverChildren
    for (auto* s : { &semitoneRndNegSlider,  &semitoneRndPosSlider,
                     &fineTuneRndNegSlider,  &fineTuneRndPosSlider,
                     &volumeRndNegSlider,    &volumeRndPosSlider,
                     &panRndNegSlider,       &panRndPosSlider,
                     &toneLowRndNegSlider,   &toneLowRndPosSlider,
                     &toneHighRndNegSlider,  &toneHighRndPosSlider,
                     &sampleStartRndNegSlider, &sampleStartRndPosSlider,
                     &sampleEndRndNegSlider,   &sampleEndRndPosSlider
                     // COMMENTED FOR LITE — ACTIVE IN PREMIUM
                     //,&lowGainRndNegSlider,   &lowGainRndPosSlider,
                     //,&lowFreqRndNegSlider,   &lowFreqRndPosSlider,
                     //,&midGainRndNegSlider,   &midGainRndPosSlider,
                     //,&midFreqRndNegSlider,   &midFreqRndPosSlider,
                     //,&highGainRndNegSlider,  &highGainRndPosSlider,
                     //,&highFreqRndNegSlider,  &highFreqRndPosSlider,
                     //,&transAtkRndNegSlider,  &transAtkRndPosSlider,
                     //,&transDecRndNegSlider,  &transDecRndPosSlider,
                     //,&envAtkRndNegSlider,    &envAtkRndPosSlider,
                     //,&envDecRndNegSlider,    &envDecRndPosSlider
                     })
        setupSlider(*s);

    // Apply knob LAF to main parameter sliders
    for (auto* s : { &semitoneSlider, &fineTuneSlider, &volumeSlider, &panSlider,
                     &toneLowSlider, &toneHighSlider,
                     &sampleStartSlider, &sampleEndSlider
                     // COMMENTED FOR LITE — ACTIVE IN PREMIUM
                     //,&lowGainSlider, &lowFreqSlider, &midGainSlider, &midFreqSlider,
                     //,&highGainSlider, &highFreqSlider,
                     //,&transientAttackSlider, &transientDecaySlider,
                     //,&envAttackSlider, &envDecaySlider
                     })
        s->setLookAndFeel(&knobLAF);
    
    // NEW: set indicator line color per section (rotarySliderFillColourId)
        for (auto* s : { &semitoneSlider, &fineTuneSlider })
            s->setColour(juce::Slider::rotarySliderFillColourId, RRColors::pitchCol);

        for (auto* s : { &volumeSlider, &panSlider })
            s->setColour(juce::Slider::rotarySliderFillColourId, RRColors::ampCol);

        for (auto* s : { &toneLowSlider, &toneHighSlider })
            s->setColour(juce::Slider::rotarySliderFillColourId, RRColors::toneCol);

        for (auto* s : { &sampleStartSlider, &sampleEndSlider })
            s->setColour(juce::Slider::rotarySliderFillColourId, RRColors::trimCol);

        randomAlgorithmSlider.setLookAndFeel(&knobLAF);
        randomAlgorithmSlider.setColour(juce::Slider::rotarySliderFillColourId, RRColors::algoCol);

        // COMMENTED FOR LITE — ACTIVE IN PREMIUM
        //for (auto* s : { &envAttackSlider, &envDecaySlider })
        //    s->setColour(juce::Slider::rotarySliderFillColourId, RRColors::envCol);

        //for (auto* s : { &transientAttackSlider, &transientDecaySlider })
        //    s->setColour(juce::Slider::rotarySliderFillColourId, RRColors::transCol);

        //for (auto* s : { &lowGainSlider, &lowFreqSlider })
        //    s->setColour(juce::Slider::rotarySliderFillColourId, RRColors::eqLowCol);

        //for (auto* s : { &midGainSlider, &midFreqSlider })
        //    s->setColour(juce::Slider::rotarySliderFillColourId, RRColors::eqMidCol);

        //for (auto* s : { &highGainSlider, &highFreqSlider })
        //    s->setColour(juce::Slider::rotarySliderFillColourId, RRColors::eqHighCol);
    
    // NEW: value box text + border color matches section label color
        auto setValueBoxColors = [](juce::Slider& s, juce::Colour col)
        {
            s.setColour(juce::Slider::textBoxTextColourId,       col);
            s.setColour(juce::Slider::textBoxOutlineColourId,    col.withAlpha(0.55f));
            s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff111111));
        };

        setValueBoxColors(semitoneSlider,        RRColors::pitchCol);
        setValueBoxColors(fineTuneSlider,        RRColors::pitchCol);
        setValueBoxColors(volumeSlider,          RRColors::ampCol);
        setValueBoxColors(panSlider,             RRColors::ampCol);
        setValueBoxColors(toneLowSlider,         RRColors::toneCol);
        setValueBoxColors(toneHighSlider,        RRColors::toneCol);
        setValueBoxColors(sampleStartSlider,     RRColors::trimCol);
        setValueBoxColors(sampleEndSlider,       RRColors::trimCol);
        setValueBoxColors(randomAlgorithmSlider, RRColors::algoCol);
        // COMMENTED FOR LITE — ACTIVE IN PREMIUM
        //setValueBoxColors(envAttackSlider,       RRColors::envCol);
        //setValueBoxColors(envDecaySlider,        RRColors::envCol);
        //setValueBoxColors(transientAttackSlider, RRColors::transCol);
        //setValueBoxColors(transientDecaySlider,  RRColors::transCol);
        //setValueBoxColors(lowGainSlider,         RRColors::eqLowCol);
        //setValueBoxColors(lowFreqSlider,         RRColors::eqLowCol);
        //setValueBoxColors(midGainSlider,         RRColors::eqMidCol);
        //setValueBoxColors(midFreqSlider,         RRColors::eqMidCol);
        //setValueBoxColors(highGainSlider,        RRColors::eqHighCol);
        //setValueBoxColors(highFreqSlider,        RRColors::eqHighCol);


    // Dual-thumb randomization sliders
    for (auto* bar : { &semitoneRndBar, &fineTuneRndBar, &volumeRndBar, &panRndBar,
                       &toneLowRndBar, &toneHighRndBar, &sampleStartRndBar, &sampleEndRndBar })
        addAndMakeVisible(*bar);

    // Repaint arc outlines when main knobs move (arcs track knob position)
    for (auto* s : { &semitoneSlider, &fineTuneSlider, &volumeSlider, &panSlider,
                     &toneLowSlider, &toneHighSlider, &sampleStartSlider, &sampleEndSlider })
        s->onValueChange = [this] { repaint(); };

    setSize(1400, 400);
    resized();
}

//==============================================================================
NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    for (auto* s : { &semitoneSlider, &fineTuneSlider, &volumeSlider, &panSlider,
                     &toneLowSlider, &toneHighSlider,
                     &sampleStartSlider, &sampleEndSlider,
                     &randomAlgorithmSlider })
        s->setLookAndFeel(nullptr);

    for (auto* b : { &triggerButton, &panicButton, &savePresetButton, &loadPresetButton, &aboutButton })
        b->setLookAndFeel(nullptr);
}

//==============================================================================
void NewProjectAudioProcessorEditor::setupSlider(juce::Slider& s)
{
    // Hidden — APVTS-bound but not rendered directly.
    // Values are read by paintOverChildren() to draw rnd arcs over knobs.
    s.setSliderStyle(juce::Slider::LinearHorizontal);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addChildComponent(s);   // addChildComponent = part of tree but invisible
}

void NewProjectAudioProcessorEditor::setupKnob(juce::Slider& s)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 65, 16);
    addAndMakeVisible(s);
}

//==============================================================================
void NewProjectAudioProcessorEditor::loadSamplesFromFiles()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Load Samples (up to 20)",
        juce::File{},
        "*.wav;*.aif;*.aiff;*.flac;*.ogg"
    );

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles |
        juce::FileBrowserComponent::canSelectMultipleItems,
        [this](const juce::FileChooser& fc)
        {
            auto results = fc.getResults();

            for (int i = 0; i < NewProjectAudioProcessor::NUM_SAMPLE_SLOTS; ++i)
                audioProcessor.sampleLoader.clearSlot(i);

            int slot = 0;
            for (const auto& file : results)
            {
                if (slot >= NewProjectAudioProcessor::NUM_SAMPLE_SLOTS) break;
                if (file.existsAsFile())
                {
                    audioProcessor.sampleLoader.loadSample(slot, file);
                    ++slot;
                }
            }

            audioProcessor.rebuildLoadedIndices();
            sampleManagerPanel.repaint();
        }
    );
}

void NewProjectAudioProcessorEditor::addMoreSamples()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Add Samples",
        juce::File{},
        "*.wav;*.aif;*.aiff;*.flac;*.ogg"
    );

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles |
        juce::FileBrowserComponent::canSelectMultipleItems,
        [this](const juce::FileChooser& fc)
        {
            auto results = fc.getResults();

            // Find first empty slot
            int slot = 0;
            for (; slot < NewProjectAudioProcessor::NUM_SAMPLE_SLOTS; ++slot)
                if (!audioProcessor.sampleSlots[slot].isLoaded)
                    break;

            for (const auto& file : results)
            {
                if (slot >= NewProjectAudioProcessor::NUM_SAMPLE_SLOTS) break;
                if (file.existsAsFile())
                {
                    audioProcessor.sampleLoader.loadSample(slot, file);
                    ++slot;
                    // Skip to next empty slot
                    while (slot < NewProjectAudioProcessor::NUM_SAMPLE_SLOTS &&
                           audioProcessor.sampleSlots[slot].isLoaded)
                        ++slot;
                }
            }

            audioProcessor.rebuildLoadedIndices();
            sampleManagerPanel.repaint();
        }
    );
}

void NewProjectAudioProcessorEditor::updateSamplesInfo()
{
    // REMOVED FOR LITE: samplesInfoLabel removed — no-op
    //int count = (int)audioProcessor.loadedSlotIndices.size();
    //if (count == 0)
    //    samplesInfoLabel.setText("No Samples Loaded", juce::dontSendNotification);
    //else
    //    samplesInfoLabel.setText(juce::String(count) + " sample(s) loaded",
    //        juce::dontSendNotification);
}

void NewProjectAudioProcessorEditor::savePreset()
{
    DBG("=== SAVE PRESET BUTTON CLICKED ===");
    DBG("  loadedSlotIndices.size() = " + juce::String((int)audioProcessor.loadedSlotIndices.size()));

    // Warn the user early if no samples are loaded — the preset would be empty
    if (audioProcessor.loadedSlotIndices.empty())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "No Samples Loaded",
            "Please load at least one sample before saving a preset.");
        return;
    }

    // useOSNativeDialogBox = false: JUCE 8 on macOS has a bug where the native
    // file chooser always creates NSOpenPanel even when saveMode is set, causing
    // "Ignoring NSSavePanel method sent to NSOpenPanel" and empty getResult().
    // JUCE's own cross-platform dialog correctly handles save mode.
    fileChooser = std::make_unique<juce::FileChooser>(
        "Save Preset",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.rrpreset",
        false);   // false = use JUCE dialog, not native macOS panel

    fileChooser->launchAsync(
        juce::FileBrowserComponent::saveMode |
        juce::FileBrowserComponent::canSelectFiles |
        juce::FileBrowserComponent::warnAboutOverwriting,
        [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            DBG("Save dialog result: '" + file.getFullPathName() + "'");
            if (file.getFullPathName().isNotEmpty())
                audioProcessor.savePreset(file.withFileExtension(".rrpreset"));
            else
                DBG("Save dialog: cancelled or returned empty path");
        });
}

void NewProjectAudioProcessorEditor::loadPreset()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Load Preset", juce::File{}, "*.rrpreset");

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                audioProcessor.loadPreset(file);
                updateSamplesInfo();
            }
        });
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint(juce::Graphics& g)
{
    // ── Layout constants (must match resized() exactly) ───────────────────────
    constexpr int margin   = 12;
    constexpr int knobW    = 68;
    constexpr int topY     = 56;
    constexpr int gap      = 8;
    constexpr int headerH  = 48;
    constexpr int footerH  = 22;

    // Content area
    constexpr int secBot   = 400 - footerH - 6;             // 372
    constexpr int contentH = secBot - topY;                  // 316

    // Left panel (sample manager — wide for filenames)
    constexpr int lpW      = 682;

    // Center: Random Algorithm
    constexpr int algoX    = margin + lpW + gap;             // 702
    constexpr int algoW    = 210;

    // Right: 2×2 parameter grid (compact)
    constexpr int gridX    = algoX + algoW + gap;            // 920
    constexpr int gridW    = 1400 - gridX - margin;          // 468
    constexpr int secW     = (gridW - gap) / 2;              // 230
    constexpr int secH     = (contentH - gap) / 2;           // 154

    // Section positions (2×2 grid)
    constexpr int ampX     = gridX;
    constexpr int ampY     = topY;
    constexpr int toneX    = gridX + secW + gap;
    constexpr int toneY    = topY;
    constexpr int pitchX   = gridX;
    constexpr int pitchY   = topY + secH + gap;
    constexpr int trimX    = gridX + secW + gap;
    constexpr int trimY    = topY + secH + gap;

    // Knob X offsets within each section (230px wide)
    constexpr int knobGap  = 16;
    constexpr int secKx0   = (secW - 2 * knobW - knobGap) / 2;
    constexpr int secKx1   = secKx0 + knobW + knobGap;

    // ── Background (warm cream panel with faint horizontal brush grain) ────
    g.fillAll(RRColors::background);
    {
        // Soft brush grain — darker warm tone
        g.setColour(RRColors::backgroundLo.withAlpha(0.35f));
        for (int by = 0; by < getHeight(); by += 3)
            g.fillRect(0, by, getWidth(), 1);
        // Gentle edge vignette
        juce::ColourGradient vignette(
            juce::Colours::transparentBlack,       getWidth() * 0.5f, getHeight() * 0.5f,
            juce::Colours::black.withAlpha(0.15f), 0.0f, 0.0f, true);
        g.setGradientFill(vignette);
        g.fillRect(getLocalBounds());
    }

    // ── Header bar (darker warm gray strip) ────────────────────────────────
    {
        juce::ColourGradient hdrGrad(
            RRColors::headerBg.brighter(0.05f), 0.0f, 0.0f,
            RRColors::headerBg.darker(0.2f),    0.0f, (float)headerH,
            false);
        g.setGradientFill(hdrGrad);
        g.fillRect(0, 0, getWidth(), headerH);
    }
    // Divider: dark ink line
    g.setColour(juce::Colour(0xff3c3428));
    g.fillRect(0, headerH, getWidth(), 1);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.fillRect(0, headerH + 1, getWidth(), 1);

    // Brushed nameplate behind the title — darker warm tone
    {
        juce::Rectangle<float> plate(8.0f, 6.0f, 260.0f, 36.0f);
        juce::ColourGradient plateGrad(
            juce::Colour(0xff4a4438), plate.getX(), plate.getY(),
            juce::Colour(0xff2a2418), plate.getX(), plate.getBottom(),
            false);
        g.setGradientFill(plateGrad);
        g.fillRoundedRectangle(plate, 2.0f);
        g.setColour(juce::Colour(0xff1a140c));
        g.drawRoundedRectangle(plate, 2.0f, 0.8f);
        g.setColour(juce::Colours::white.withAlpha(0.04f));
        for (int py = (int)plate.getY() + 1; py < (int)plate.getBottom(); py += 2)
            g.fillRect((int)plate.getX() + 1, py, (int)plate.getWidth() - 2, 1);
    }

    juce::Font rrFont(juce::FontOptions(24.0f));
    rrFont = rrFont.boldened();
    g.setFont(rrFont);
    g.setColour(RRColors::screenPrint);
    g.drawText("Robin Control", 16, 9, 240, 28, juce::Justification::left);

    const int liteX = 16 + rrFont.getStringWidth("Robin Control") + 6;
    g.setColour(RRColors::liteShade);
    g.setFont(juce::Font(juce::FontOptions(15.0f)).italicised());
    g.drawText("Lite", liteX, 13, 60, 22, juce::Justification::left);

    // ── Green LED LEVEL meter (stand-in for future real meter) ─────────────
    {
        constexpr int meterY = 18;
        constexpr int meterH = 16;
        constexpr int bars   = 14;
        constexpr int barW   = 6;
        constexpr int barGap = 2;
        const int meterW  = bars * (barW + barGap) - barGap;
        const int meterX  = 290;

        // Meter well — deep black with warm bevel
        juce::Rectangle<float> well((float)meterX - 6, (float)meterY - 3,
                                    (float)meterW + 12, (float)meterH + 6);
        g.setColour(juce::Colour(0xff050403));
        g.fillRoundedRectangle(well, 2.0f);
        g.setColour(juce::Colour(0xff3c3428));
        g.drawRoundedRectangle(well, 2.0f, 0.6f);

        // LED segments — 9 green (lit), 1 red at index 9 (over-0dB indicator),
        // remaining dim. The red position is reserved for future real-audio wiring.
        const juce::Colour dimGreen(0xff184828);
        constexpr int litCount     = 9;    // 0..8 lit green
        constexpr int overZeroIdx  = 9;    // idx 9 = red "over 0dB" segment
        for (int i = 0; i < bars; ++i)
        {
            juce::Colour c;
            bool lit = true;
            if (i < litCount)
            {
                float t = (float)i / (float)(bars - 1);
                c = dimGreen.interpolatedWith(RRColors::ledGreen, 0.4f + t * 0.6f);
            }
            else if (i == overZeroIdx)
            {
                c = RRColors::s612Red;
            }
            else
            {
                c = juce::Colour(0xff101c14);
                lit = false;
            }
            g.setColour(c);
            g.fillRect(meterX + i * (barW + barGap), meterY, barW, meterH);
            if (lit)
            {
                g.setColour(juce::Colours::white.withAlpha(0.2f));
                g.fillRect(meterX + i * (barW + barGap), meterY, barW, meterH / 2);
            }
        }
    }

    // ── Section box helper (warm-gray recessed well on cream panel) ────────
    auto drawSectionBox = [&](juce::Rectangle<int> r)
    {
        // Outer drop shadow
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillRoundedRectangle(r.toFloat().translated(1.5f, 2.0f), 4.0f);
        // Recessed fill — subtle warm-gray gradient (top slightly lighter)
        {
            juce::ColourGradient fill(
                RRColors::sectionBg.brighter(0.05f),   r.toFloat().getX(), r.toFloat().getY(),
                RRColors::sectionBgDark,               r.toFloat().getX(), r.toFloat().getBottom(),
                false);
            g.setGradientFill(fill);
            g.fillRoundedRectangle(r.toFloat(), 4.0f);
        }
        // Inner top shadow — reads as the upper inside edge of a recess
        {
            auto top = r.toFloat().removeFromTop(1.5f).reduced(5.0f, 0.0f);
            g.setColour(juce::Colours::black.withAlpha(0.22f));
            g.fillRect(top);
        }
        // Inner bottom catchlight — reads as light caught on the lower lip
        {
            auto bot = r.toFloat().removeFromBottom(1.0f).reduced(5.0f, 0.0f);
            g.setColour(juce::Colour(0xfff4eedb).withAlpha(0.45f));
            g.fillRect(bot);
        }
        // Inner dark-bevel border
        g.setColour(RRColors::sectionBorder);
        g.drawRoundedRectangle(r.toFloat(), 4.0f, 1.2f);
    };

    // Bigger section-title font (9pt → 16pt, ~75% larger)
    juce::Font sectionFont(juce::FontOptions(16.0f));
    sectionFont = sectionFont.boldened().withExtraKerningFactor(0.04f);

    // Section title helper: letterpress / engraved look on the warm panel.
    // Cream highlight shifted 1px down simulates the light edge of an engraved recess;
    // dark outline + section-color fill on top reads as ink pressed into the panel.
    auto drawSectionTitle = [&](const juce::String& text, int x, int y, int w, int h,
                                juce::Justification just, juce::Colour fillCol)
    {
        juce::GlyphArrangement ga;
        ga.addFittedText(sectionFont, text, (float)x, (float)y, (float)w, (float)h, just, 1);
        juce::Path path;
        ga.createPath(path);

        juce::Path highlight = path;
        highlight.applyTransform(juce::AffineTransform::translation(0.0f, 1.0f));
        g.setColour(juce::Colour(0xffece5d4).withAlpha(0.55f));
        g.fillPath(highlight);

        g.setColour(juce::Colour(0xff0a0806));
        g.strokePath(path, juce::PathStrokeType(1.0f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour(fillCol);
        g.fillPath(path);
    };

    // ── Left: Sample Pool ───────────────────────────────────────────────────
    drawSectionBox({ margin, topY, lpW, contentH });

    // ── Center: Random Algorithm ────────────────────────────────────────────
    drawSectionBox({ algoX, topY, algoW, contentH });
    drawSectionTitle("RANDOM ALGORITHM", algoX, topY + 6, algoW, 18,
                     juce::Justification::centred, RRColors::algoCol);

    // ── Random Algorithm tick marks (18 ticks) ──────────────────────────────
    {
        constexpr int numTicks = 18;
        constexpr int tbH     = 16;

        auto kb = randomAlgorithmSlider.getBounds();
        auto rp = randomAlgorithmSlider.getRotaryParameters();

        const float w  = (float)kb.getWidth();
        const float h  = (float)(kb.getHeight() - tbH);
        const float cx = kb.getX() + w * 0.5f;
        const float cy = kb.getY() + h * 0.5f;

        const float bodyRadius = juce::jmin(w, h) * 0.5f - 2.5f;
        const float tickInner  = bodyRadius + 2.0f;
        const float tickOuter  = bodyRadius + 7.0f;

        g.setColour(RRColors::algoCol.withAlpha(0.45f));
        for (int i = 0; i < numTicks; ++i)
        {
            float norm  = (float)i / (float)(numTicks - 1);
            float angle = rp.startAngleRadians + norm * (rp.endAngleRadians - rp.startAngleRadians);
            float sinA  = std::sin(angle);
            float cosA  = std::cos(angle);
            g.drawLine(cx + sinA * tickInner, cy - cosA * tickInner,
                       cx + sinA * tickOuter, cy - cosA * tickOuter, 1.5f);
        }
    }

    // ── Right top-left: Amplitude ──────────────────────────────────────────
    drawSectionBox({ ampX, ampY, secW, secH });
    drawSectionTitle("AMPLITUDE", ampX + 8, ampY + 6, secW - 16, 18,
                     juce::Justification::left, RRColors::ampCol);

    // ── Right top-right: Tone ──────────────────────────────────────────────
    drawSectionBox({ toneX, toneY, secW, secH });
    drawSectionTitle("TONE", toneX + 8, toneY + 6, secW - 16, 18,
                     juce::Justification::left, RRColors::toneCol);

    // ── Right bottom-left: Pitch ───────────────────────────────────────────
    drawSectionBox({ pitchX, pitchY, secW, secH });
    drawSectionTitle("PITCH", pitchX + 8, pitchY + 6, secW - 16, 18,
                     juce::Justification::left, RRColors::pitchCol);

    // ── Right bottom-right: Sample Start/End ───────────────────────────────
    drawSectionBox({ trimX, trimY, secW, secH });
    drawSectionTitle("SAMPLE START/END", trimX + 8, trimY + 6, secW - 16, 18,
                     juce::Justification::left, RRColors::trimCol);

    // ── Knob labels ─────────────────────────────────────────────────────────
    g.setFont(juce::Font(juce::FontOptions(12.0f)));
    auto drawKnobLabel = [&](const juce::String& text, int secY, int secHeight,
                             int knobX, juce::Colour col)
    {
        int padTop = (secHeight - 20 - 108) / 2;
        int labelY = secY + 20 + padTop - 4;
        g.setColour(col.withAlpha(0.85f));
        g.drawText(text, knobX - 8, labelY, knobW + 16, 14,
                   juce::Justification::centred);
    };
    drawKnobLabel("Volume",     ampY,    secH, ampX   + secKx0,   RRColors::ampCol);
    drawKnobLabel("Pan",        ampY,    secH, ampX   + secKx1,   RRColors::ampCol);
    drawKnobLabel("Low",        toneY,   secH, toneX  + secKx0,   RRColors::toneCol);
    drawKnobLabel("High",       toneY,   secH, toneX  + secKx1,   RRColors::toneCol);
    drawKnobLabel("Semitone",   pitchY,  secH, pitchX + secKx0,   RRColors::pitchCol);
    drawKnobLabel("Fine Tune",  pitchY,  secH, pitchX + secKx1,   RRColors::pitchCol);
    drawKnobLabel("Start",      trimY,   secH, trimX  + secKx0,   RRColors::trimCol);
    drawKnobLabel("End",        trimY,   secH, trimX  + secKx1,   RRColors::trimCol);

    // ── Footer ──────────────────────────────────────────────────────────────
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRect(0, getHeight() - footerH, getWidth(), 1);
    g.setColour(RRColors::companyText);
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText("Alex Hamadey", 0, getHeight() - footerH + 4, getWidth() - 14, 20,
               juce::Justification::right);
}

//==============================================================================
// PluginEditor.cpp — paintOverChildren()

void NewProjectAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    if (aboutWindow.isVisible())
    {
        juce::RectangleList<int> clip(getLocalBounds());
        clip.subtract(aboutWindow.getBounds());
        g.reduceClipRegion(clip);
    }

    constexpr float arcW   = 3.2f;
    constexpr int   tbH    = 16;

    auto drawArcOutline = [&](juce::Slider& knob,
                              juce::Slider& negSlider,
                              juce::Slider& posSlider,
                              juce::Colour  col)
    {
        auto getNorm = [](juce::Slider& s) -> float {
            double range = s.getMaximum() - s.getMinimum();
            if (range == 0.0) return 0.0f;
            return (float)((s.getValue() - s.getMinimum()) / range);
        };

        float negNorm = getNorm(negSlider);
        float posNorm = getNorm(posSlider);

        // Compute the knob's current angle from its value
        float knobNorm = getNorm(knob);
        auto rp = knob.getRotaryParameters();
        float startA    = rp.startAngleRadians;   // ~7 o'clock
        float endA      = rp.endAngleRadians;     // ~5 o'clock
        float knobAngle = startA + knobNorm * (endA - startA);

        auto  b  = knob.getBounds();
        float w  = (float)b.getWidth();
        float h  = (float)(b.getHeight() - tbH);
        float cx = b.getX() + w * 0.5f;
        float cy = b.getY() + h * 0.5f;

        float knobRadius = juce::jmin(w, h) * 0.5f - 2.5f;  // match body radius
        float radius     = knobRadius + 4.0f;              // pushed outboard of the knob body

        // Faint full-range track: always-on slot that the colored arcs fill into
        {
            juce::Path track;
            track.addArc(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f,
                startA, endA, true);
            g.setColour(col.withAlpha(0.14f));
            g.strokePath(track, juce::PathStrokeType(arcW,
                juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        if (negNorm < 0.01f && posNorm < 0.01f) return;

        // Available arc room from knob position to each rotary limit
        float roomNeg = knobAngle - startA;   // room toward 7 o'clock
        float roomPos = endA - knobAngle;     // room toward 5 o'clock

        float negExtent = negNorm * roomNeg;  // at max rnd → fills to 7 o'clock
        float posExtent = posNorm * roomPos;  // at max rnd → fills to 5 o'clock

        // Neg arc: counter-clockwise from knob position (clamped to start)
        if (negExtent > 0.01f)
        {
            juce::Path negArc;
            negArc.addArc(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f,
                knobAngle - negExtent, knobAngle, true);
            g.setColour(col.withAlpha(0.85f));
            g.strokePath(negArc, juce::PathStrokeType(arcW,
                juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Pos arc: clockwise from knob position (clamped to end)
        if (posExtent > 0.01f)
        {
            juce::Path posArc;
            posArc.addArc(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f,
                knobAngle, knobAngle + posExtent, true);
            g.setColour(col.withAlpha(0.85f));
            g.strokePath(posArc, juce::PathStrokeType(arcW,
                juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    };

    drawArcOutline(semitoneSlider,     semitoneRndNegSlider,     semitoneRndPosSlider,     RRColors::pitchCol);
    drawArcOutline(fineTuneSlider,     fineTuneRndNegSlider,     fineTuneRndPosSlider,     RRColors::pitchCol);
    drawArcOutline(volumeSlider,       volumeRndNegSlider,       volumeRndPosSlider,       RRColors::ampCol);
    drawArcOutline(panSlider,          panRndNegSlider,          panRndPosSlider,          RRColors::ampCol);
    drawArcOutline(toneLowSlider,      toneLowRndNegSlider,      toneLowRndPosSlider,      RRColors::toneCol);
    drawArcOutline(toneHighSlider,     toneHighRndNegSlider,     toneHighRndPosSlider,     RRColors::toneCol);
    drawArcOutline(sampleStartSlider,  sampleStartRndNegSlider,  sampleStartRndPosSlider,  RRColors::trimCol);
    drawArcOutline(sampleEndSlider,    sampleEndRndNegSlider,    sampleEndRndPosSlider,    RRColors::trimCol);
}

void NewProjectAudioProcessorEditor::componentVisibilityChanged(juce::Component& component)
{
    if (&component == &aboutWindow)
        repaint();   // redraws arcs when window opens OR closes
}

//==============================================================================
void NewProjectAudioProcessorEditor::resized()
{
    // ── Layout constants (must match paint() exactly) ─────────────────────────
    constexpr int margin   = 12;
    constexpr int knobW    = 68;
    constexpr int knobH    = 80;   // 64 rotary + 16 text box
    constexpr int knobGap  = 16;
    constexpr int topY     = 56;
    constexpr int gap      = 8;
    constexpr int footerH  = 22;

    // Content area
    constexpr int secBot   = 400 - footerH - 6;             // 372
    constexpr int contentH = secBot - topY;                  // 316

    // Left panel (sample manager — wide for filenames)
    constexpr int lpW      = 682;

    // Center: Random Algorithm
    constexpr int algoX    = margin + lpW + gap;             // 702
    constexpr int algoW    = 210;

    // Right: 2×2 parameter grid (compact)
    constexpr int gridX    = algoX + algoW + gap;            // 920
    constexpr int gridW    = 1400 - gridX - margin;          // 468
    constexpr int secW     = (gridW - gap) / 2;              // 230
    constexpr int secH     = (contentH - gap) / 2;           // 154

    // Section positions (2×2 grid)
    constexpr int ampX     = gridX;
    constexpr int ampY     = topY;
    constexpr int toneX    = gridX + secW + gap;
    constexpr int toneY    = topY;
    constexpr int pitchX   = gridX;
    constexpr int pitchY   = topY + secH + gap;
    constexpr int trimX    = gridX + secW + gap;
    constexpr int trimY    = topY + secH + gap;

    // Knob X offsets within each section (230px wide)
    constexpr int secKx0   = (secW - 2 * knobW - knobGap) / 2;
    constexpr int secKx1   = secKx0 + knobW + knobGap;

    // ── Header buttons ──────────────────────────────────────────────────────
    loadPresetButton.setBounds(getWidth() - 334, 11, 60, 26);
    savePresetButton.setBounds(getWidth() - 266, 11, 60, 26);
    triggerButton.setBounds   (getWidth() - 198, 11, 70, 26);
    panicButton.setBounds     (getWidth() - 118, 11, 26, 26);
    aboutButton.setBounds     (getWidth() -  56, 11, 26, 26);

    // ── Left: Sample Manager ────────────────────────────────────────────────
    sampleManagerPanel.setBounds(margin, topY, lpW, contentH);

    // ── Knob pair placement (centered vertically in section) ────────────────
    auto placeKnobPair = [&](juce::Slider& knob0, juce::Slider& knob1,
                             int secY, int secHeight, int xBase, int xOff0, int xOff1)
    {
        int padTop = (secHeight - 20 - 108) / 2;
        int ky = secY + 20 + padTop + 14;          // 12px label + 2px gap
        knob0.setBounds(xBase + xOff0, ky, knobW, knobH);
        knob1.setBounds(xBase + xOff1, ky, knobW, knobH);
    };

    placeKnobPair(volumeSlider,      panSlider,         ampY,   secH, ampX,   secKx0, secKx1);
    placeKnobPair(toneLowSlider,     toneHighSlider,    toneY,  secH, toneX,  secKx0, secKx1);
    placeKnobPair(semitoneSlider,    fineTuneSlider,    pitchY, secH, pitchX, secKx0, secKx1);
    placeKnobPair(sampleStartSlider, sampleEndSlider,   trimY,  secH, trimX,  secKx0, secKx1);

    // ── Algorithm knob (~50% larger than before, centered, title above) ────
    {
        constexpr int algoKnobW = 130;
        constexpr int algoKnobH = 146;  // 130 rotary + 16 text box
        int algoKnobX = algoX + (algoW - algoKnobW) / 2;
        int algoKnobY = topY + 20 + (contentH - 20 - algoKnobH) / 2;
        randomAlgorithmSlider.setBounds(algoKnobX, algoKnobY, algoKnobW, algoKnobH);
    }

    // ── Dual-thumb rnd sliders (2px below knob text box) ────────────────────
    auto placeRndBar = [&](DualThumbRndSlider& bar, juce::Slider& knob)
    {
        auto b = knob.getBounds();
        constexpr int barH = 12;
        int barY = b.getBottom() + 2;
        bar.setBounds(b.getX() + 2, barY, b.getWidth() - 4, barH);
    };

    placeRndBar(volumeRndBar,       volumeSlider);
    placeRndBar(panRndBar,          panSlider);
    placeRndBar(semitoneRndBar,     semitoneSlider);
    placeRndBar(fineTuneRndBar,     fineTuneSlider);
    placeRndBar(toneLowRndBar,      toneLowSlider);
    placeRndBar(toneHighRndBar,     toneHighSlider);
    placeRndBar(sampleStartRndBar,  sampleStartSlider);
    placeRndBar(sampleEndRndBar,    sampleEndSlider);
}
