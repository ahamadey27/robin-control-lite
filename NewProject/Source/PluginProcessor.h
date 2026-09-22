#pragma once
#include <JuceHeader.h>
#include "Audio/RRvoice.h"
#include "Audio/RRSound.h"
#include "Data/SampleSlot.h"
#include "Data/SampleLoader.h"
#include "Parameters/ParametersIDs.h"
#include "DSP/ThreeBandEQ.h"
#include "DSP/TransientShaper.h"
#include "DSP/ToneControl.h"
#include "DSP/RandomizationEngine.h"

//==============================================================================
class NewProjectAudioProcessor : public juce::AudioProcessor,
                                  public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    juce::AAXClientExtensions& getAAXClientExtensions() override { return aaxExtensions; }

    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // DEBUG: Parameter change logging (remove before final build)
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    //==============================================================================
    // Public Data (accessed by PluginEditor)
    juce::AudioProcessorValueTreeState apvts;

    static constexpr int NUM_SAMPLE_SLOTS = 20;
    SampleSlot sampleSlots[NUM_SAMPLE_SLOTS];
    std::vector<int> loadedSlotIndices;
    int roundRobinIndex = 0;

    std::vector<int> shuffledIndices;
    int lastPlayedSlot = -1;     // Fisher-Yates shuffled copy
    int maxSampleLength = 0;     // longest loaded sample in samples (for relative start/end)
    void reshuffleIndices();              // add declaration

    void advanceRoundRobin();
    void resetPlaybackPosition();
    void rebuildLoadedIndices();

    void savePreset(const juce::File& file);
    void loadPreset(const juce::File& file);
    void auditionSample(int slotIndex);
    void swapSamples(int indexA, int indexB);
    void insertSample(int fromIndex, int toIndex);
    void requestTrigger() { triggerPending.store(true); }
    void requestPanic()   { panicPending.store(true); }

    // Output peak magnitude (0..~) — updated at the end of each processBlock,
    // read by the editor's level-meter timer. Linear gain, not dB.
    std::atomic<float> outputPeakLevel { 0.0f };

    // Sequence and slot travel together so repeated hits on one row still flash.
    juce::uint64 getPlaybackEvent() const noexcept
    {
        return playbackEvent.load(std::memory_order_acquire);
    }

    static int getPlaybackEventSlot(juce::uint64 event) noexcept
    {
        return static_cast<int>(event & 0xff) - 1;
    }

private:
    struct AAXExtensions final : juce::AAXClientExtensions
    {
        juce::String getPageFileName() const override { return "RobinControlLitePages.xml"; }
    } aaxExtensions;

    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    // Audio Engine — ORDER MATTERS: synthesiser and formatManager must come before sampleLoader
    juce::Synthesiser synthesiser;
    juce::AudioFormatManager formatManager;

public:
    // Declared after private dependencies so it initializes after them
    SampleLoader sampleLoader{ formatManager, synthesiser, sampleSlots, NUM_SAMPLE_SLOTS, getCallbackLock() };

private:
    //==============================================================================
    // DSP Processors
    ThreeBandEQ threeBandEQ;
    TransientShaper transientShaper;
    ToneControl toneControl;

    // Trigger flag (set by UI, consumed by processBlock)
    std::atomic<bool> triggerPending{ false };
    std::atomic<bool> panicPending{ false };
    std::atomic<juce::uint64> playbackEvent { 0 };
    void publishPlaybackEvent(int slotIndex) noexcept;

    // Current global pitch values
    std::atomic<float> globalSemitones{ 0.0f };
    std::atomic<float> globalCents{ 0.0f };
    std::atomic<float> globalAttackMs{ 0.0f };
    std::atomic<float> globalDecayMs{ 100.0f };

    // Parameter Smoothing
    juce::LinearSmoothedValue<float> smoothedSemitone;
    juce::LinearSmoothedValue<float> smoothedFineTune;
    juce::LinearSmoothedValue<float> smoothedVolume;
    juce::LinearSmoothedValue<float> smoothedEnvAttack;
    juce::LinearSmoothedValue<float> smoothedEnvDecay;
    juce::LinearSmoothedValue<float> smoothedTransientAttack;
    juce::LinearSmoothedValue<float> smoothedTransientDecay;

    RandomizationEngine randomizationEngine;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessor)
};
