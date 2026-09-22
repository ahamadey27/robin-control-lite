#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"
#include <array>

namespace
{
    constexpr std::array<float, 6> kUIScaleOptions { 0.25f, 0.50f, 0.75f, 1.00f, 1.25f, 1.50f };

    // Per-user preferences, owned by the editor so JUCE timers die before GUI shutdown.
    std::unique_ptr<juce::PropertiesFile> createUIPrefs()
    {
        juce::PropertiesFile::Options opts;
        opts.applicationName     = "RobinControlLite";
        opts.filenameSuffix      = "settings";
        opts.osxLibrarySubFolder = "Application Support";
        opts.folderName          = "RobinControlLite";
        opts.storageFormat       = juce::PropertiesFile::storeAsXML;
        return std::make_unique<juce::PropertiesFile>(opts);
    }
}

NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor(NewProjectAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), uiPrefs(createUIPrefs()),
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
    // "Load Samples" appends to the pool (matches the in-pool "click to add"
    // placeholder). Users clear with the Clear button to start fresh.
    sampleManagerPanel.onLoadSamplesClicked = [this]() { addMoreSamples(); };
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
                "Replace Sample", juce::File{}, "*.wav;*.aif;*.aiff;*.flac;*.ogg;*.mp3");
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

    // Header accents match Robin Control: amber Trigger, red Save, orange Load.
    triggerButton.setButtonText("Trigger");
    triggerButton.setColour(juce::TextButton::buttonColourId, RRColors::amber);
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
            if (aboutWindow.isVisible())
            {
                aboutWindow.setVisible(false);
            }
            else
            {
                aboutWindow.setTopLeftPosition((getWidth()  - aboutWindow.getWidth())  / 2,
                                               (getHeight() - aboutWindow.getHeight()) / 2);
                addAndMakeVisible(aboutWindow);
                aboutWindow.toFront(true);
            }
            repaint();
        };

    addAndMakeVisible(aboutButton);
    aboutWindow.addComponentListener(this);

    // User Presets — Robin Control's Save and Browse accent colors.
    savePresetButton.setButtonText("Save");
    savePresetButton.setColour(juce::TextButton::buttonColourId, RRColors::s612RedDim);
    savePresetButton.setColour(juce::TextButton::textColourOnId, RRColors::screenPrint);
    savePresetButton.setLookAndFeel(&buttonLAF);
    savePresetButton.onClick = [this]() { savePreset(); };
    addAndMakeVisible(savePresetButton);

    loadPresetButton.setButtonText("Load");
    loadPresetButton.setColour(juce::TextButton::buttonColourId, RRColors::algoCol);
    loadPresetButton.setColour(juce::TextButton::textColourOnId, RRColors::screenPrint);
    loadPresetButton.setLookAndFeel(&buttonLAF);
    loadPresetButton.onClick = [this]() { loadPreset(); };
    addAndMakeVisible(loadPresetButton);

    // Size dropdown (UI scale 25–150%). Button text reflects current scale.
    sizeButton.setButtonText("100%");
    sizeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3e5a40));
    sizeButton.setColour(juce::TextButton::textColourOnId, RRColors::screenPrint);
    sizeButton.setLookAndFeel(&buttonLAF);
    sizeButton.onClick = [this]()
        {
            juce::PopupMenu menu;
            for (size_t i = 0; i < kUIScaleOptions.size(); ++i)
            {
                const float s = kUIScaleOptions[i];
                const bool isCurrent = std::abs(s - userScale) < 0.001f;
                menu.addItem((int) i + 1,
                             juce::String(juce::roundToInt(s * 100)) + "%",
                             true, isCurrent);
            }
            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(sizeButton),
                [safeThis = juce::Component::SafePointer<NewProjectAudioProcessorEditor>(this)](int result)
                {
                    if (safeThis == nullptr || result <= 0) return;
                    const size_t idx = (size_t) (result - 1);
                    if (idx < kUIScaleOptions.size())
                        safeThis->setEditorScale(kUIScaleOptions[idx]);
                });
        };
    addAndMakeVisible(sizeButton);

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
    randomAlgorithmSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 22);
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

#if RCL_ENABLE_MOONBASE
    licenseButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d7a7a));
    licenseButton.setLookAndFeel(&buttonLAF);
    activationOverlay = std::make_unique<moonbase::juce_integration::ActivationComponent>(
        audioProcessor.getLicense().controller());
    activationOverlay->onClose = [this] { activationOverlay->dismiss(); repaint(); };
    licenseButton.onClick = [this]
    {
        aboutWindow.setVisible(false);
        activationOverlay->toFront(true);
        activationOverlay->appear();
        repaint();
    };
    addAndMakeVisible(licenseButton);
    addChildComponent(*activationOverlay);
    if (! audioProcessor.getLicense().isLicensed())
        activationOverlay->setVisible(true);
#endif

    setSize(1400, 400);
    resized();

    // Do not replay an old hit when the editor is reopened.
    lastPlaybackEvent = audioProcessor.getPlaybackEvent();
    startTimerHz(30);

    const float savedScale = (float) uiPrefs->getDoubleValue("uiScale", 1.0);
    for (const float option : kUIScaleOptions)
        if (std::abs(savedScale - option) < 0.001f)
            userScale = option;
    sizeButton.setButtonText(juce::String(juce::roundToInt(userScale * 100)) + "%");
    applyCombinedScale();
}

//==============================================================================
NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    stopTimer();

#if RCL_ENABLE_MOONBASE
    activationOverlay.reset();
    licenseButton.setLookAndFeel(nullptr);
#endif

    for (auto* s : { &semitoneSlider, &fineTuneSlider, &volumeSlider, &panSlider,
                     &toneLowSlider, &toneHighSlider,
                     &sampleStartSlider, &sampleEndSlider,
                     &randomAlgorithmSlider })
        s->setLookAndFeel(nullptr);

    for (auto* b : { &triggerButton, &panicButton, &savePresetButton, &loadPresetButton, &aboutButton, &sizeButton })
        b->setLookAndFeel(nullptr);
}

//==============================================================================
void NewProjectAudioProcessorEditor::setEditorScale(float newScale)
{
    userScale = newScale;
    sizeButton.setButtonText(juce::String(juce::roundToInt(newScale * 100)) + "%");
    uiPrefs->setValue("uiScale", (double) newScale);
    uiPrefs->saveIfNeeded();
    applyCombinedScale();
}

void NewProjectAudioProcessorEditor::setScaleFactor(float newScale)
{
    // Host-driven DPI scaling. Record it and reapply combined transform so
    // user-chosen scale survives host scale changes.
    hostScale = newScale;
    applyCombinedScale();
}

void NewProjectAudioProcessorEditor::applyCombinedScale()
{
    // Forward to base so hostScaleTransform and the editor's transform are set
    // together — that's what satisfies the jassert in editorResized().
    juce::AudioProcessorEditor::setScaleFactor(hostScale * userScale);
}

void NewProjectAudioProcessorEditor::timerCallback()
{
#if RCL_ENABLE_MOONBASE
    const bool licensed = audioProcessor.getLicense().isLicensed();
    licenseButton.setButtonText(licensed ? "License" : "Activate");
    if (! licensed && ! activationOverlay->isVisible())
    {
        activationOverlay->toFront(true);
        activationOverlay->appear();
        repaint();
    }
#endif
    const auto event = audioProcessor.getPlaybackEvent();
    if (event != lastPlaybackEvent)
    {
        lastPlaybackEvent = event;
        sampleManagerPanel.triggerPlayedSampleHighlight(
            NewProjectAudioProcessor::getPlaybackEventSlot(event));
    }
    sampleManagerPanel.advancePlayedSampleHighlight();

    // Pull the latest peak from the processor and run it through a fast-attack /
    // slow-release envelope so the LEDs rise instantly but fall smoothly.
    const float target = audioProcessor.outputPeakLevel.load(std::memory_order_relaxed);

    if (target >= displayedLevel)
        displayedLevel = target;                  // instant attack — catch peaks
    else
        displayedLevel += (target - displayedLevel) * 0.25f;  // ~4-tick (~130ms) release

    if (! meterBounds.isEmpty())
        repaint(meterBounds);
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
void NewProjectAudioProcessorEditor::addMoreSamples()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Add Samples",
        juce::File{},
        "*.wav;*.aif;*.aiff;*.flac;*.ogg;*.mp3"
    );

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles |
        juce::FileBrowserComponent::canSelectMultipleItems,
        [this](const juce::FileChooser& fc)
        {
            addSamplesFromFiles(fc.getResults());
        }
    );
}

// Append each supported file to the next empty slot. Additive — never clears
// the pool. Shared by the file chooser (addMoreSamples) and drag-and-drop
// (filesDropped) so both entry points behave identically.
void NewProjectAudioProcessorEditor::addSamplesFromFiles(const juce::Array<juce::File>& files)
{
    // Find first empty slot
    int slot = 0;
    for (; slot < NewProjectAudioProcessor::NUM_SAMPLE_SLOTS; ++slot)
        if (!audioProcessor.sampleSlots[slot].isLoaded)
            break;

    for (const auto& file : files)
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

//==============================================================================
// Drag-and-drop of audio files.
//
// SCOPE: This is the OS-level file-drop API. It reliably accepts files dragged
// from the OS file manager (Windows Explorer / macOS Finder) even while the
// plugin is hosted inside a DAW. Dragging from a DAW's OWN internal browser
// (e.g. FL Studio's browser panel) only works if that host chooses to deliver
// the drag as an OS file drop — many hosts, FL included, often do NOT, so that
// path is host-dependent and outside the plugin's control. There is no separate
// plugin-side API for it; FileDragAndDropTarget is the best (and only) hook.

static bool isSupportedAudioFile(const juce::String& path)
{
    const juce::String ext = juce::File(path).getFileExtension().toLowerCase();
    return ext == ".wav"  || ext == ".aif" || ext == ".aiff"
        || ext == ".flac" || ext == ".ogg" || ext == ".mp3";
}

bool NewProjectAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
        if (isSupportedAudioFile(f))
            return true;
    return false;
}

void NewProjectAudioProcessorEditor::fileDragEnter(const juce::StringArray& files, int, int)
{
    if (isInterestedInFileDrag(files) && !isFileDragHovering)
    {
        isFileDragHovering = true;
        repaint();
    }
}

void NewProjectAudioProcessorEditor::fileDragExit(const juce::StringArray&)
{
    if (isFileDragHovering)
    {
        isFileDragHovering = false;
        repaint();
    }
}

void NewProjectAudioProcessorEditor::filesDropped(const juce::StringArray& files, int, int)
{
    isFileDragHovering = false;

    juce::Array<juce::File> audioFiles;
    for (const auto& f : files)
        if (isSupportedAudioFile(f))
            audioFiles.add(juce::File(f));

    if (!audioFiles.isEmpty())
        addSamplesFromFiles(audioFiles);

    repaint();
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

    fileChooser = std::make_unique<juce::FileChooser>(
        "Save Preset",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.rrpreset");

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
    // Divider: dark ink line + warm catchlight — reads as a crisp panel ridge
    g.setColour(juce::Colour(0xff1d160d));
    g.fillRect(0, headerH, getWidth(), 2);
    g.setColour(juce::Colour(0xfff4eedb).withAlpha(0.32f));
    g.fillRect(0, headerH + 2, getWidth(), 1);

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

    const int liteX = 16 + juce::GlyphArrangement::getStringWidthInt(rrFont, "Robin Control") + 6;
    g.setColour(RRColors::liteShade);
    g.setFont(juce::Font(juce::FontOptions(15.0f)).italicised());
    g.drawText("Lite", liteX, 13, 60, 22, juce::Justification::left);

    // ── LED LEVEL meter ────────────────────────────────────────────────────
    // 14 segments; 0..8 green below 0dB, 9..13 red at/above 0dB. Each bar has
    // a dB threshold and lights only when the current output peak reaches it.
    {
        constexpr int meterY = 18;
        constexpr int meterH = 16;
        constexpr int bars   = 14;
        constexpr int barW   = 6;
        constexpr int barGap = 2;
        const int meterW  = bars * (barW + barGap) - barGap;
        const int meterX  = 290;

        // Cache meter rect so timerCallback can repaint just this region.
        meterBounds = juce::Rectangle<int>(meterX - 6, meterY - 3,
                                           meterW + 12, meterH + 6);

        // Meter well — deep black with warm bevel
        juce::Rectangle<float> well = meterBounds.toFloat();
        g.setColour(juce::Colour(0xff050403));
        g.fillRoundedRectangle(well, 2.0f);
        g.setColour(juce::Colour(0xff3c3428));
        g.drawRoundedRectangle(well, 2.0f, 0.6f);

        // Current level in dB (clamp very quiet signals to avoid -inf math)
        const float levelDb = juce::Decibels::gainToDecibels(displayedLevel, -100.0f);

        // Bar thresholds: 3 dB per step, with bar 9 landing exactly on 0 dB.
        // Below 0 dB → green; at/above 0 dB → red.
        constexpr int firstRedIdx = 9;                 // 9..13 are red
        const juce::Colour dimGreen(0xff0f2a18);
        const juce::Colour dimRed  (0xff3a0c08);

        for (int i = 0; i < bars; ++i)
        {
            const float thresholdDb = (float)(i - firstRedIdx) * 3.0f;  // bar 9 = 0 dB
            const bool  isRed       = i >= firstRedIdx;
            const bool  lit         = levelDb >= thresholdDb;

            juce::Colour c;
            if (lit)
            {
                if (isRed)
                {
                    c = RRColors::s612Red;
                }
                else
                {
                    float t = (float)i / (float)(firstRedIdx - 1);  // 0..1 across green range
                    c = dimGreen.interpolatedWith(RRColors::ledGreen, 0.4f + t * 0.6f);
                }
            }
            else
            {
                c = isRed ? dimRed : juce::Colour(0xff0a140c);
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

        // Cream drop-shadow underneath (1px down)
        juce::Path shadow = path;
        shadow.applyTransform(juce::AffineTransform::translation(0.0f, 1.0f));
        g.setColour(juce::Colour(0xffece5d4).withAlpha(0.55f));
        g.fillPath(shadow);

        // Dark outline stroke
        g.setColour(juce::Colour(0xff0a0806));
        g.strokePath(path, juce::PathStrokeType(1.0f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // S612 button-strip treatment — vertical gradient fill (lighter top → darker bottom)
        const auto pathBounds = path.getBounds();
        {
            juce::ColourGradient grad(
                fillCol.brighter(0.15f), pathBounds.getX(), pathBounds.getY(),
                fillCol.darker(0.10f),   pathBounds.getX(), pathBounds.getBottom(),
                false);
            g.setGradientFill(grad);
            g.fillPath(path);
        }

        // Faint white sheen along top edge of glyphs (clipped to text shape)
        {
            juce::Graphics::ScopedSaveState state(g);
            g.reduceClipRegion(path);
            g.setColour(juce::Colours::white.withAlpha(0.25f));
            g.fillRect(pathBounds.getX(), pathBounds.getY() + 0.5f,
                       pathBounds.getWidth(), 0.8f);
        }
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

    // conduit.dsp logo.
    //
    // Using the email-white asset because its glyphs are pure black on white
    // (bolder than the charcoal/transparent variant, which reads as too
    // translucent once scaled down). Pipeline:
    //   1. Key white → alpha. For each pixel, alpha = 255 - min(R,G,B) and
    //      un-composite to recover the opaque foreground color — text becomes
    //      pure black, the teal waveform stays teal.
    //   2. Crop tightly to content bounding box (source has transparent
    //      padding top/bottom, so drawing at N px actually gives N px of
    //      glyph, not N/2).
    //   3. 1-pixel morphological dilate + alpha ×1.4 boost for final bold.
    static const juce::Image logo = [] {
        auto src = juce::ImageCache::getFromMemory(
            BinaryData::FULLLOGOemailwhite_png,
            BinaryData::FULLLOGOemailwhite_pngSize);
        const int W0 = src.getWidth(), H0 = src.getHeight();

        juce::Image keyed(juce::Image::ARGB, W0, H0, true);
        for (int y = 0; y < H0; ++y)
            for (int x = 0; x < W0; ++x)
            {
                const auto p = src.getPixelAt(x, y);
                const int r = p.getRed(), g = p.getGreen(), b = p.getBlue();
                const int whiteness = juce::jmin(juce::jmin(r, g), b);
                const int a = 255 - whiteness;
                if (a <= 0) continue;
                const int nr = juce::jlimit(0, 255, (r - whiteness) * 255 / a);
                const int ng = juce::jlimit(0, 255, (g - whiteness) * 255 / a);
                const int nb = juce::jlimit(0, 255, (b - whiteness) * 255 / a);
                keyed.setPixelAt(x, y, juce::Colour::fromRGBA(
                    (juce::uint8)nr, (juce::uint8)ng, (juce::uint8)nb, (juce::uint8)a));
            }

        int minX = W0, minY = H0, maxX = -1, maxY = -1;
        for (int y = 0; y < H0; ++y)
            for (int x = 0; x < W0; ++x)
                if (keyed.getPixelAt(x, y).getAlpha() > 8)
                {
                    minX = juce::jmin(minX, x); minY = juce::jmin(minY, y);
                    maxX = juce::jmax(maxX, x); maxY = juce::jmax(maxY, y);
                }
        if (maxX < minX) { minX = 0; maxX = W0 - 1; minY = 0; maxY = H0 - 1; }

        minX = juce::jmax(0, minX - 1);
        minY = juce::jmax(0, minY - 1);
        maxX = juce::jmin(W0 - 1, maxX + 1);
        maxY = juce::jmin(H0 - 1, maxY + 1);
        auto cropped = keyed.getClippedImage({ minX, minY, maxX - minX + 1, maxY - minY + 1 });

        const int W = cropped.getWidth(), H = cropped.getHeight();
        juce::Image out(juce::Image::ARGB, W, H, true);
        constexpr int dilateRadius = 3;    // wider neighborhood = thicker strokes
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x)
            {
                juce::Colour best;
                juce::uint8 maxA = 0;
                for (int dy = -dilateRadius; dy <= dilateRadius; ++dy)
                    for (int dx = -dilateRadius; dx <= dilateRadius; ++dx)
                    {
                        const int nx = juce::jlimit(0, W - 1, x + dx);
                        const int ny = juce::jlimit(0, H - 1, y + dy);
                        const auto c = cropped.getPixelAt(nx, ny);
                        if (c.getAlpha() > maxA) { maxA = c.getAlpha(); best = c; }
                    }
                const int boosted = juce::jmin(255, (int)(maxA * 3.0f));
                out.setPixelAt(x, y, best.withAlpha((juce::uint8)boosted));
            }
        return out;
    }();

    constexpr int logoH = 16;                                      // 10% smaller; fits 22-px footer with 3 px padding top/bottom
    const int logoW = juce::roundToInt(logoH * (float)logo.getWidth()
                                              / (float)logo.getHeight());
    const int logoX = getWidth() - logoW - 14;
    const int logoY = getHeight() - footerH + (footerH - logoH) / 2;

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage(logo,
                juce::Rectangle<float>((float)logoX, (float)logoY,
                                       (float)logoW, (float)logoH),
                juce::RectanglePlacement::stretchToFit, false);
}

//==============================================================================
// PluginEditor.cpp — paintOverChildren()

void NewProjectAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
#if RCL_ENABLE_MOONBASE
    if (activationOverlay != nullptr && activationOverlay->isVisible())
        return;
#endif
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

    // Drop-zone highlight — drawn last so it sits over everything while a valid
    // audio-file drag hovers the editor.
    if (isFileDragHovering)
    {
        auto r = getLocalBounds().toFloat().reduced(2.0f);
        g.setColour(RRColors::s612RedDim.withAlpha(0.10f));
        g.fillRoundedRectangle(r, 6.0f);
        g.setColour(juce::Colour(0xffece5d4).withAlpha(0.85f));
        g.drawRoundedRectangle(r, 6.0f, 2.0f);
    }
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
    sizeButton.setBounds(getWidth() - 402, 11, 60, 26);
#if RCL_ENABLE_MOONBASE
    licenseButton.setBounds(getWidth() - 486, 11, 76, 26);
    if (activationOverlay != nullptr)
        activationOverlay->setBounds(getLocalBounds());
#endif
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
