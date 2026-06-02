#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Parameters/ParametersIDs.h"
#include "UI/RRLookAndFeel.h"
#include "UI/SampleManagerPanel.h"
#include "UI/DualThumbRndSlider.h"

class NewProjectAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        public juce::ComponentListener,
                                        public juce::FileDragAndDropTarget,
                                        private juce::Timer
{
public:
    NewProjectAudioProcessorEditor(NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void paintOverChildren(juce::Graphics&) override;
    void componentVisibilityChanged(juce::Component&) override;
    void timerCallback() override;

    // Drag-and-drop of audio files from the OS file manager (Explorer/Finder),
    // and from any DAW whose browser delivers an OS-level file drop (host-dependent).
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    NewProjectAudioProcessor& audioProcessor;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    RRKnobLAF knobLAF;
    RRButtonLAF buttonLAF;

    // Buttons & labels
    juce::TextButton triggerButton;
    juce::TextButton panicButton;
    juce::TextButton savePresetButton;
    juce::TextButton loadPresetButton;
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Main sliders
    juce::Slider semitoneSlider, fineTuneSlider;
    juce::Slider volumeSlider, panSlider;
    juce::Slider toneLowSlider, toneHighSlider;
    juce::Slider sampleStartSlider, sampleEndSlider;
    juce::Slider randomAlgorithmSlider;

    SliderAttachment semitoneAttachment, fineTuneAttachment;
    SliderAttachment volumeAttachment, panAttachment;
    SliderAttachment toneLowAttachment, toneHighAttachment;
    SliderAttachment sampleStartAttachment, sampleEndAttachment;
    SliderAttachment randomAlgorithmAttachment;

    // Hidden rnd sliders (APVTS-bound, values read by DualThumbRndSlider and arc outline)
    juce::Slider semitoneRndNegSlider, semitoneRndPosSlider;
    juce::Slider fineTuneRndNegSlider, fineTuneRndPosSlider;
    juce::Slider volumeRndNegSlider, volumeRndPosSlider;
    juce::Slider panRndNegSlider, panRndPosSlider;
    juce::Slider toneLowRndNegSlider, toneLowRndPosSlider;
    juce::Slider toneHighRndNegSlider, toneHighRndPosSlider;
    juce::Slider sampleStartRndNegSlider, sampleStartRndPosSlider;
    juce::Slider sampleEndRndNegSlider, sampleEndRndPosSlider;

    SliderAttachment semitoneRndNegAttachment, semitoneRndPosAttachment;
    SliderAttachment fineTuneRndNegAttachment, fineTuneRndPosAttachment;
    SliderAttachment volumeRndNegAttachment, volumeRndPosAttachment;
    SliderAttachment panRndNegAttachment, panRndPosAttachment;
    SliderAttachment toneLowRndNegAttachment, toneLowRndPosAttachment;
    SliderAttachment toneHighRndNegAttachment, toneHighRndPosAttachment;
    SliderAttachment sampleStartRndNegAttachment, sampleStartRndPosAttachment;
    SliderAttachment sampleEndRndNegAttachment, sampleEndRndPosAttachment;

    // Dual-thumb randomization sliders (below each knob)
    DualThumbRndSlider semitoneRndBar   { semitoneRndNegSlider,     semitoneRndPosSlider,     RRColors::pitchCol };
    DualThumbRndSlider fineTuneRndBar   { fineTuneRndNegSlider,     fineTuneRndPosSlider,     RRColors::pitchCol };
    DualThumbRndSlider volumeRndBar     { volumeRndNegSlider,       volumeRndPosSlider,       RRColors::ampCol };
    DualThumbRndSlider panRndBar        { panRndNegSlider,          panRndPosSlider,          RRColors::ampCol };
    DualThumbRndSlider toneLowRndBar    { toneLowRndNegSlider,      toneLowRndPosSlider,      RRColors::toneCol };
    DualThumbRndSlider toneHighRndBar   { toneHighRndNegSlider,     toneHighRndPosSlider,     RRColors::toneCol };
    DualThumbRndSlider sampleStartRndBar{ sampleStartRndNegSlider,  sampleStartRndPosSlider,  RRColors::trimCol };
    DualThumbRndSlider sampleEndRndBar  { sampleEndRndNegSlider,    sampleEndRndPosSlider,    RRColors::trimCol };

    // About window
    class AboutWindow : public juce::Component
    {
    public:
        AboutWindow()
        {
            closeButton.setButtonText("Close");
            closeButton.onClick = [this] { setVisible(false); };
            addAndMakeVisible(closeButton);

            // Measure body text height precisely so the gap above the close button
            // matches the gap below the title.
            juce::AttributedString attr;
            attr.setText(bodyText);
            attr.setFont(juce::Font(juce::FontOptions(13.f)));
            attr.setJustification(juce::Justification::centredTop);

            juce::TextLayout layout;
            layout.createLayout(attr, (float) (boxWidth - 40));
            bodyHeight = (int) std::ceil(layout.getHeight());

            setSize(boxWidth, marginTop + titleH + gap + bodyHeight + gap + buttonH + marginBottom);
        }

        void paint(juce::Graphics& g) override
        {
            // Matte black dialog on brushed dark panel — matches main UI
            g.setColour(juce::Colour(0xff14110e));
            g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.f);
            g.setColour(juce::Colour(0xff3a2e20));
            g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 6.f, 0.8f);

            // "Robin Control" + "Lite" title — Lite in amber
            juce::Font titleFont(juce::FontOptions(17.f));
            titleFont = titleFont.boldened();
            g.setFont(titleFont);
            const juce::String mainTitle = "Robin Control";
            const int titleW = titleFont.getStringWidth(mainTitle);
            const int liteW  = 42;
            const int totalW = titleW + 6 + liteW;
            const int tx = (getWidth() - totalW) / 2;

            g.setColour(juce::Colour(0xffe6e1d4));
            g.drawText(mainTitle, tx, marginTop, titleW, titleH, juce::Justification::left);

            g.setFont(juce::Font(juce::FontOptions(13.f)).italicised());
            g.setColour(juce::Colour(0xffffb84a));
            g.drawText("Lite", tx + titleW + 6, marginTop + 4, liteW, 18, juce::Justification::left);

            g.setColour(juce::Colour(0xffc0b8a8));
            g.setFont(juce::Font(juce::FontOptions(13.f)));
            g.drawFittedText(bodyText, 20, marginTop + titleH + gap, getWidth() - 40, bodyHeight,
                juce::Justification::centredTop, 16);

            // Version stamp, bottom-right corner — dim, doesn't perturb measured layout.
            // JucePlugin_VersionString is a const char* emitted by juce_add_plugin(VERSION ...).
            g.setFont(juce::Font(juce::FontOptions(9.f)));
            g.setColour(juce::Colour(0xff7a7468));
            g.drawText(juce::String("v") + JucePlugin_VersionString,
                       getWidth() - 50, getHeight() - 14, 40, 10,
                       juce::Justification::right);
        }

        void resized() override
        {
            const int by = marginTop + titleH + gap + bodyHeight + gap;
            closeButton.setBounds(getWidth() / 2 - 40, by, 80, buttonH);
        }

    private:
        static constexpr int boxWidth     = 340;
        static constexpr int marginTop    = 16;
        static constexpr int titleH       = 22;
        static constexpr int gap          = 12;
        static constexpr int buttonH      = 24;
        static constexpr int marginBottom = 16;

        const juce::String bodyText =
            "Load up to 20 samples. Samples are mapped to all\n"
            "keys and played back randomly based on selected\n"
            "playback type (Series or Random)\n\n"
            "Use the knobs to control pitch, volume, etc.\n"
            "Use the sliders below each knob to set per-note\n"
            "randomization ranges\n\n"
            "Random Algorithm knob introduces randomization\n"
            "settings that increase in intensity with every click\n\n"
            "'RESET' button resets the sequence. Useful in series\n"
            "mode to reset playback to the first sample";

        int bodyHeight { 0 };
        juce::TextButton closeButton;
    };

    AboutWindow      aboutWindow;
    juce::TextButton aboutButton;
    SampleManagerPanel sampleManagerPanel;

    // Level-meter state — timerCallback pulls the processor's peak into
    // displayedLevel with a fast-attack / slow-release envelope, then repaints
    // just the meter rect.
    float displayedLevel { 0.0f };   // linear gain
    juce::Rectangle<int> meterBounds;

    // Helpers
    void setupSlider(juce::Slider& s);
    void setupKnob(juce::Slider& s);
    void addMoreSamples();

    // Shared loader used by both the file chooser and drag-and-drop: appends each
    // supported file to the next empty slot (additive — never clears the pool).
    void addSamplesFromFiles(const juce::Array<juce::File>& files);

    // True while a valid audio-file drag is hovering the editor (drives the
    // drop-zone highlight in paintOverChildren).
    bool isFileDragHovering = false;
    void updateSamplesInfo();
    void savePreset();
    void loadPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessorEditor)
};
