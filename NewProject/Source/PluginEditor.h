#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Parameters/ParametersIDs.h"
#include "UI/RRLookAndFeel.h"
#include "UI/SampleManagerPanel.h"
#include "UI/DualThumbRndSlider.h"

class NewProjectAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::ComponentListener
{
public:
    NewProjectAudioProcessorEditor(NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void paintOverChildren(juce::Graphics&) override;
    void componentVisibilityChanged(juce::Component&) override;

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
            setSize(340, 200);
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
            g.drawText(mainTitle, tx, 16, titleW, 22, juce::Justification::left);

            g.setFont(juce::Font(juce::FontOptions(13.f)).italicised());
            g.setColour(juce::Colour(0xffffb84a));
            g.drawText("Lite", tx + titleW + 6, 20, liteW, 18, juce::Justification::left);

            g.setColour(juce::Colour(0xffc0b8a8));
            g.setFont(juce::Font(juce::FontOptions(13.f)));
            juce::String body =
                "Load up to 20 one-shot samples. Each sample is\n"
                "mapped to all white keys and played back randomly\n"
                "based on selected playback algorithm type.\n\n"
                "Use the knobs to control pitch, volume, etc.\n"
                "Use the sliders below each knob to set per-note\n"
                "randomization ranges.";
            g.drawFittedText(body, 20, 50, getWidth() - 40, 110,
                juce::Justification::centredTop, 6);
        }

        void resized() override
        {
            closeButton.setBounds(getWidth() / 2 - 40, getHeight() - 36, 80, 24);
        }

    private:
        juce::TextButton closeButton;
    };

    AboutWindow      aboutWindow;
    juce::TextButton aboutButton;
    SampleManagerPanel sampleManagerPanel;

    // Helpers
    void setupSlider(juce::Slider& s);
    void setupKnob(juce::Slider& s);
    void loadSamplesFromFiles();
    void addMoreSamples();
    void updateSamplesInfo();
    void savePreset();
    void loadPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessorEditor)
};
