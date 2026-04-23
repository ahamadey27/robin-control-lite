/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Audio/MidiMapper.h" 
#include "Parameters/ParametersIDs.h"
#include "DSP/TransientShaper.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    synthesiser.addVoice(new RRVoice());
    synthesiser.addSound(new RRSound());

    // NEW: wire randomization engine + APVTS into the voice
    for (int i = 0; i < synthesiser.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<RRVoice*>(synthesiser.getVoice(i)))
            voice->setRandomizationReferences(&randomizationEngine, &apvts);
    }

    formatManager.registerBasicFormats();

    DBG("=== Robin Control Lite Initialized ===");
    DBG("Synthesiser ready with " + juce::String(synthesiser.getNumVoices()) + " voice(s)");
    DBG("UNPITCHED PLAYBACK MODE - All samples play at original pitch");
    DBG("Root reference note: C1 (MIDI 36)");
    DBG("===================================");

    DBG("\n=== Testing Parameter IDs ===");
    DBG("Semitone ID: " + juce::String(ParameterIDs::semitone));
    DBG("Volume ID: " + juce::String(ParameterIDs::volume));
    DBG("Total parameters: " + juce::String(ParameterIDs::totalParameters));
    DBG("============================\n");

    DBG("\n=== Testing APVTS ===");
    DBG("APVTS initialized: " + juce::String(apvts.state.isValid() ? "YES" : "NO"));
    DBG("Current parameter count: " + juce::String(apvts.state.getNumChildren()));
    DBG("=====================\n");

    DBG("=== Sample Slots Initialized ===");
    DBG("Slots available: " + juce::String(NUM_SAMPLE_SLOTS));
    DBG("All slots empty on startup — load via UI");
    DBG("================================");

    // DEBUG: Register parameter change listener (remove before final build)
    for (auto* param : getParameters())
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param))
            apvts.addParameterListener(p->paramID, this);
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
    // DEBUG: Remove parameter listeners (remove before final build)
    for (auto* param : getParameters())
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param))
            apvts.removeParameterListener(p->paramID, this);
}

// DEBUG: Log parameter changes to console (remove before final build)
void NewProjectAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    DBG("[PARAM] " + parameterID + " = " + juce::String(newValue, 4));
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName(int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    DBG("prepareToPlay called - Sample Rate: " + juce::String(sampleRate) +
        " Hz, Buffer Size: " + juce::String(samplesPerBlock) + " samples");

    sampleLoader.setSampleRate(sampleRate);
    synthesiser.setCurrentPlaybackSampleRate(sampleRate);

    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //==============================================================================
    // INITIALIZE 3-BAND EQ

    //threeBandEQ.prepareToPlay(sampleRate, samplesPerBlock);

    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //==============================================================================
    // INITIALIZE TRANSIENT SHAPER

    //transientShaper.prepareToPlay(sampleRate, samplesPerBlock);

    //==============================================================================
    // INITIALIZE TONE CONTROL

    toneControl.prepareToPlay(sampleRate, samplesPerBlock);

    //==============================================================================
    // INITIALIZE PARAMETER SMOOTHING

    const double rampTimeSeconds = 0.05; // 50 milliseconds

    // Pitch Controls
    smoothedSemitone.reset(sampleRate, rampTimeSeconds);
    smoothedSemitone.setCurrentAndTargetValue(apvts.getRawParameterValue(ParameterIDs::semitone)->load());

    smoothedFineTune.reset(sampleRate, rampTimeSeconds);
    smoothedFineTune.setCurrentAndTargetValue(apvts.getRawParameterValue(ParameterIDs::fineTune)->load());

    // Volume
    smoothedVolume.reset(sampleRate, rampTimeSeconds);
    smoothedVolume.setCurrentAndTargetValue(apvts.getRawParameterValue(ParameterIDs::volume)->load());

    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    // Amplitude Envelope
    //smoothedEnvAttack.reset(sampleRate, rampTimeSeconds);
    //smoothedEnvAttack.setCurrentAndTargetValue(apvts.getRawParameterValue(ParameterIDs::envAttack)->load());

    //smoothedEnvDecay.reset(sampleRate, rampTimeSeconds);
    //smoothedEnvDecay.setCurrentAndTargetValue(apvts.getRawParameterValue(ParameterIDs::envDecay)->load());

    // NOTE: Transient attack/decay read directly from APVTS — no smoothers needed

    DBG("All parameter smoothing initialized");
}

void NewProjectAudioProcessor::releaseResources()
{
    DBG("releaseResources called");

    threeBandEQ.reset();
    transientShaper.reset();
    toneControl.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void NewProjectAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //==============================================================================
    // UPDATE SMOOTHED PARAMETER VALUES

    smoothedSemitone.setTargetValue(apvts.getRawParameterValue(ParameterIDs::semitone)->load());
    smoothedFineTune.setTargetValue(apvts.getRawParameterValue(ParameterIDs::fineTune)->load());
    smoothedVolume.setTargetValue(apvts.getRawParameterValue(ParameterIDs::volume)->load());
    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //smoothedEnvAttack.setTargetValue(apvts.getRawParameterValue(ParameterIDs::envAttack)->load());
    //smoothedEnvDecay.setTargetValue(apvts.getRawParameterValue(ParameterIDs::envDecay)->load());

    //==============================================================================
    // CLEAR OUTPUT BUFFERS

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    for (int i = 0; i < totalNumInputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    //==============================================================================
    // STORE GLOBAL PARAMETER VALUES FOR VOICES

    globalSemitones.store(smoothedSemitone.getCurrentValue());
    globalCents.store(smoothedFineTune.getCurrentValue());
    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //globalAttackMs.store(smoothedEnvAttack.getCurrentValue());
    //globalDecayMs.store(smoothedEnvDecay.getCurrentValue());

    //==============================================================================
    // UPDATE ALL VOICES WITH CURRENT GLOBAL PARAMETERS

    float semitones = static_cast<float>(apvts.getRawParameterValue(ParameterIDs::semitone)->load());
    float cents = static_cast<float>(apvts.getRawParameterValue(ParameterIDs::fineTune)->load());
    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //float attackMs = apvts.getRawParameterValue(ParameterIDs::envAttack)->load();
    //float decayMs = apvts.getRawParameterValue(ParameterIDs::envDecay)->load();

    for (int i = 0; i < synthesiser.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<RRVoice*>(synthesiser.getVoice(i)))
        {
            voice->updateGlobalParameters(semitones, cents, 0.0f, 0.0f); // LITE: attack/decay removed from APVTS
            voice->setMaxPoolSampleLength(maxSampleLength);
        }
    }

    //==============================================================================
    // INJECT TRIGGER NOTE-ON (from UI button)

    if (triggerPending.exchange(false))
    {
        midiMessages.addEvent(juce::MidiMessage::noteOn(1, 36, 0.8f), 0);
    }

    //==============================================================================
    // ROUND ROBIN: advance sample on each note-on before rendering

    for (const auto metadata : midiMessages)
    {
        if (metadata.getMessage().isNoteOn())
        {
            advanceRoundRobin();
            break; // monophonic — only one note-on per block matters
        }
    }

    //==============================================================================
    // PANIC: kill all sound before rendering

    if (panicPending.exchange(false))
    {
        midiMessages.clear();
        synthesiser.allNotesOff(0, false);
    }

    //==============================================================================
    // RENDER AUDIO FROM SYNTHESISER

    synthesiser.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //==============================================================================
    // APPLY 3-BAND EQ (AFTER SYNTH, BEFORE TRANSIENT)

    //// Read live from APVTS — always reflects current knob positions
    //float eqLowGain = apvts.getRawParameterValue(ParameterIDs::lowGain)->load();
    //float eqLowFreq = apvts.getRawParameterValue(ParameterIDs::lowFreq)->load();
    //float eqMidGain = apvts.getRawParameterValue(ParameterIDs::midGain)->load();
    //float eqMidFreq = apvts.getRawParameterValue(ParameterIDs::midFreq)->load();
    //float eqHighGain = apvts.getRawParameterValue(ParameterIDs::highGain)->load();
    //float eqHighFreq = apvts.getRawParameterValue(ParameterIDs::highFreq)->load();
    //float transAtk = apvts.getRawParameterValue(ParameterIDs::transientAttack)->load();
    //float transDec = apvts.getRawParameterValue(ParameterIDs::transientDecay)->load();

    //// OVERRIDE with randomized values when a voice is actively playing
    //if (auto* voice = dynamic_cast<RRVoice*>(synthesiser.getVoice(0)))
    //{
    //    if (voice->isVoiceActive())
    //    {
    //        eqLowGain = voice->getRandomizedLowGain();
    //        eqLowFreq = voice->getRandomizedLowFreq();
    //        eqMidGain = voice->getRandomizedMidGain();
    //        eqMidFreq = voice->getRandomizedMidFreq();
    //        eqHighGain = voice->getRandomizedHighGain();
    //        eqHighFreq = voice->getRandomizedHighFreq();
    //        transAtk = voice->getRandomizedTransientAttack();
    //        transDec = voice->getRandomizedTransientDecay();
    //    }

    //    threeBandEQ.updateFilters(eqLowGain, eqLowFreq, eqMidGain, eqMidFreq, eqHighGain, eqHighFreq);
    //    threeBandEQ.processBlock(buffer);
    //}

    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //==============================================================================
    // APPLY TRANSIENT SHAPING (AFTER EQ, BEFORE VOLUME)

    //float transAtk = apvts.getRawParameterValue(ParameterIDs::transientAttack)->load();
    //float transDec = apvts.getRawParameterValue(ParameterIDs::transientDecay)->load();

    //transientShaper.processBlock(buffer, transAtk, transDec);

    //==============================================================================
    // APPLY VOLUME CONTROL
    // FIXED: advance smoother once per sample, apply to all channels — prevents
    // stereo divergence from getNextValue() being called once per channel per sample

    const int numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float volumeGain = smoothedVolume.getNextValue();

        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            buffer.getWritePointer(channel)[sample] *= volumeGain;
    }

    //==============================================================================
    // APPLY TONE CONTROL (AFTER VOLUME/PAN, BEFORE OUTPUT)

    float toneLowGain  = apvts.getRawParameterValue(ParameterIDs::toneLow)->load();
    float toneHighGain = apvts.getRawParameterValue(ParameterIDs::toneHigh)->load();

    // Override with randomized values when a voice is actively playing
    if (auto* voice = dynamic_cast<RRVoice*>(synthesiser.getVoice(0)))
    {
        if (voice->isVoiceActive())
        {
            toneLowGain  = voice->getRandomizedToneLow();
            toneHighGain = voice->getRandomizedToneHigh();
        }
    }

    toneControl.updateFilters(toneLowGain, toneHighGain);
    toneControl.processBlock(buffer);

    //==============================================================================
    // CAPTURE OUTPUT PEAK FOR LEVEL METER

    float peak = 0.0f;
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        peak = std::max(peak, buffer.getMagnitude(channel, 0, numSamples));
    outputPeakLevel.store(peak, std::memory_order_relaxed);
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor(*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    DBG("=== SAVING PLUGIN STATE ===");

    auto state = apvts.copyState();

    // Remove any stale CustomData left from a previous setStateInformation call,
    // otherwise getChildWithName() on load will find the old (empty) node first.
    auto existingCustomData = state.getChildWithName("CustomData");
    if (existingCustomData.isValid())
        state.removeChild(existingCustomData, nullptr);

    juce::ValueTree customData("CustomData");
    customData.setProperty("version", 1, nullptr);
    
    // ADD: save sample file paths
    juce::ValueTree sampleData("SampleData");
    for (int i = 0; i < NUM_SAMPLE_SLOTS; ++i)
    {
        if (sampleSlots[i].isLoaded)
        {
            DBG("  Saving slot " + juce::String(i) + ": " + sampleSlots[i].sourceFile.getFullPathName());
            juce::ValueTree slot("Slot");
            slot.setProperty("index", i, nullptr);
            slot.setProperty("path", sampleSlots[i].sourceFile.getFullPathName(), nullptr);
            slot.setProperty("displayName", sampleSlots[i].displayName, nullptr);
            sampleData.appendChild(slot, nullptr);
        }
    }
    customData.appendChild(sampleData, nullptr);
    state.appendChild(customData, nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    if (xml != nullptr)
    {
        copyXmlToBinary(*xml, destData);
        DBG("State saved successfully");
        DBG("  Parameters saved: " + juce::String(apvts.state.getNumChildren()));
        DBG("  Samples saved:    " + juce::String(sampleData.getNumChildren()));
        DBG("========================");
    }
    else
    {
        DBG("ERROR: Failed to create XML from state");
    }
}

void NewProjectAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    DBG("=== LOADING PLUGIN STATE ===");

    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState == nullptr)
    {
        DBG("ERROR: Failed to parse XML from binary data");
        return;
    }

    if (!xmlState->hasTagName(apvts.state.getType()))
    {
        DBG("ERROR: XML tag name doesn't match APVTS state type");
        return;
    }

    juce::ValueTree state = juce::ValueTree::fromXml(*xmlState);

    if (!state.isValid())
    {
        DBG("ERROR: Failed to create ValueTree from XML");
        return;
    }

    // ── Extract sample data BEFORE apvts.replaceState ─────────────────────────
    // apvts.replaceState() reorganises the tree internally and empties deep
    // children (Slot nodes inside CustomData/SampleData lose their children).
    // Taking a deep copy first keeps the data safe across that call.
    juce::ValueTree sampleDataCopy;
    {
        juce::ValueTree customData = state.getChildWithName("CustomData");
        DBG("  CustomData found: " + juce::String(customData.isValid() ? "yes" : "no"));
        if (customData.isValid())
        {
            juce::ValueTree sampleData = customData.getChildWithName("SampleData");
            DBG("  SampleData found: " + juce::String(sampleData.isValid() ? "yes" : "no")
                + "  children: " + juce::String(sampleData.getNumChildren()));
            if (sampleData.isValid())
                sampleDataCopy = sampleData.createCopy();   // deep copy — survives replaceState
        }
    }

    apvts.replaceState(state);

    // ── Restore sample file paths from the pre-extracted copy ─────────────────
    if (sampleDataCopy.isValid())
    {
        for (int i = 0; i < NUM_SAMPLE_SLOTS; ++i)
            sampleLoader.clearSlot(i);

        juce::StringArray missingSamples;

        for (auto slot : sampleDataCopy)
        {
            int index        = slot.getProperty("index",       -1);
            juce::String path = slot.getProperty("path",       "");
            juce::String name = slot.getProperty("displayName","");

            DBG("  Restoring slot " + juce::String(index) + ": " + path);

            if (index < 0 || index >= NUM_SAMPLE_SLOTS)
                continue;

            juce::File file(path);
            if (file.existsAsFile())
            {
                sampleLoader.loadSample(index, file);
            }
            else
            {
                DBG("  FILE NOT FOUND: " + path);
                missingSamples.add("Slot " + juce::String(index + 1) + ": " + name);
            }
        }

        rebuildLoadedIndices();
        DBG("  Loaded slots after restore: " + juce::String((int)loadedSlotIndices.size()));
        sampleLoader.updateSynthesiserSounds();
        DBG("  Synth sounds after restore: " + juce::String(synthesiser.getNumSounds()));

        if (!missingSamples.isEmpty())
        {
            juce::String msg = "The following samples could not be found:\n\n";
            msg += missingSamples.joinIntoString("\n");
            msg += "\n\nPlease reload them manually.";

            juce::MessageManager::callAsync([msg]()
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Missing Samples", msg);
                });
        }
    }
    DBG("State loaded successfully");
    DBG("  Parameters restored: " + juce::String(apvts.state.getNumChildren()));
    DBG("========================");

}

//==============================================================================
// PARAMETER LAYOUT CREATION

juce::AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    //==============================================================================
    // GLOBAL PITCH CONTROLS

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(ParameterIDs::semitone, 1),
        "Semitone",
        -12, 12, 0,
        juce::String(),
        [](int value, int) { return juce::String(value) + " st"; }
    ));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(ParameterIDs::fineTune, 1),
        "Fine Tune",
        -100, 100, 0,
        juce::String(),
        [](int value, int) { return juce::String(value) + " cents"; }
    ));

    //==============================================================================
    // VOLUME

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::volume, 1),
        "Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.75f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB";
        },
        [](const juce::String& text) {
            return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue());
        }
    ));

    //==============================================================================
    // PAN

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::pan, 1),
        "Pan",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value < -0.01f) return juce::String(int(std::abs(value) * 100)) + "% L";
            if (value > 0.01f)  return juce::String(int(value * 100)) + "% R";
            return juce::String("Center");
        }
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::panRndNeg, 1),
        "Pan Rnd Neg", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::panRndPos, 1),
        "Pan Rnd Pos", 0.0f, 1.0f, 0.0f));

    //==============================================================================
    // TONE CONTROL

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::toneLow, 1),
        "Tone Low",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " dB"; },
        [](const juce::String& text) { return text.dropLastCharacters(3).getFloatValue(); }
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::toneHigh, 1),
        "Tone High",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " dB"; },
        [](const juce::String& text) { return text.dropLastCharacters(3).getFloatValue(); }
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::toneLowRndNeg, 1),
        "Tone Low Rnd Neg", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::toneLowRndPos, 1),
        "Tone Low Rnd Pos", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::toneHighRndNeg, 1),
        "Tone High Rnd Neg", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::toneHighRndPos, 1),
        "Tone High Rnd Pos", 0.0f, 1.0f, 0.0f));

    //==============================================================================
    // SAMPLE START/END

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::sampleStart, 1),
        "Sample Start",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " %"; },
        [](const juce::String& text) { return text.dropLastCharacters(2).getFloatValue(); }
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::sampleEnd, 1),
        "Sample End",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " %"; },
        [](const juce::String& text) { return text.dropLastCharacters(2).getFloatValue(); }
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::sampleStartRndNeg, 1),
        "Sample Start Rnd Neg", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::sampleStartRndPos, 1),
        "Sample Start Rnd Pos", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::sampleEndRndNeg, 1),
        "Sample End Rnd Neg", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::sampleEndRndPos, 1),
        "Sample End Rnd Pos", 0.0f, 1.0f, 0.0f));

    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //==============================================================================
    // AMPLITUDE ENVELOPE

    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::envAttack, 1),
    //    "Env Attack",
    //    juce::NormalisableRange<float>(0.0f, 1000.0f, 0.1f, 0.3f),
    //    0.0f,
    //    "ms",
    //    juce::AudioProcessorParameter::genericParameter,
    //    [](float value, int) { return juce::String(value, 1) + " ms"; }
    //));

    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::envDecay, 1),
    //    "Env Decay",
    //    juce::NormalisableRange<float>(0.0f, 5000.0f, 1.0f, 0.4f),
    //    5000.0f,
    //    "ms",
    //    juce::AudioProcessorParameter::genericParameter,
    //    [](float value, int) { return juce::String(value, 1) + " ms"; }
    //));


    //==============================================================================
    // RANDOM ALGORITHM

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(ParameterIDs::randomAlgorithm, 1),
        "Random Algorithm",
        0, 17, 0));

    //==============================================================================
    // PLAYBACK MODE -- Set true to select random

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(ParameterIDs::playbackMode, 1),
        "Playback Mode", false));  // false = Series, true = Random

    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //==============================================================================
    // 3-BAND EQ

    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::lowGain, 1),
    //    "Low Gain",
    //    juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
    //    0.0f, "dB"
    //));

    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::lowFreq, 1),
    //    "Low Freq",
    //    juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.25f),
    //    100.0f, "Hz"
    //));

    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::midGain, 1),
    //    "Mid Gain",
    //    juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
    //    0.0f, "dB"
    //));

    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::midFreq, 1),
    //    "Mid Freq",
    //    juce::NormalisableRange<float>(200.0f, 5000.0f, 1.0f, 0.3f),
    //    1000.0f, "Hz"
    //));

    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::highGain, 1),
    //    "High Gain",
    //    juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
    //    0.0f, "dB"
    //));

    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::highFreq, 1),
    //    "High Freq",
    //    juce::NormalisableRange<float>(2000.0f, 20000.0f, 1.0f, 0.3f),
    //    5000.0f, "Hz"
    //));

    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //==============================================================================
    // TRANSIENT MASTER

    //layout.add(std::make_unique<juce::AudioParameterInt>(
    //    juce::ParameterID(ParameterIDs::transientAttack, 1),
    //    "Transient Attack",
    //    -127, 127, 0
    //));

    //layout.add(std::make_unique<juce::AudioParameterInt>(
    //    juce::ParameterID(ParameterIDs::transientDecay, 1),
    //    "Transient Decay",
    //    -127, 127, 0
    //));

    DBG("Parameter layout created with " + juce::String(ParameterIDs::totalParameters) + " parameters");

    //==============================================================================
    // RANDOMIZATION PARAMETERS

    // Semitone
    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(ParameterIDs::semitoneRndNeg, 1),
        "Semitone Rnd Neg", 0, 12, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(ParameterIDs::semitoneRndPos, 1),
        "Semitone Rnd Pos", 0, 12, 0));

    // Fine Tune
    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(ParameterIDs::fineTuneRndNeg, 1),
        "Fine Tune Rnd Neg", 0, 100, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(ParameterIDs::fineTuneRndPos, 1),
        "Fine Tune Rnd Pos", 0, 100, 0));

    // Volume
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::volumeRndNeg, 1),
        "Volume Rnd Neg", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::volumeRndPos, 1),
        "Volume Rnd Pos", 0.0f, 1.0f, 0.0f));

    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    // Envelope Attack
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::envAttackRndNeg, 1),
    //    "Env Attack Rnd Neg", 0.0f, 1000.0f, 0.0f));
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::envAttackRndPos, 1),
    //    "Env Attack Rnd Pos", 0.0f, 1000.0f, 0.0f));

    // Envelope Decay
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::envDecayRndNeg, 1),
    //    "Env Decay Rnd Neg", 0.0f, 5000.0f, 0.0f));
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::envDecayRndPos, 1),
    //    "Env Decay Rnd Pos", 0.0f, 5000.0f, 0.0f));

    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    // Low EQ Gain
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::lowGainRndNeg, 1),
    //    "Low Gain Rnd Neg", 0.0f, 24.0f, 0.0f));
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::lowGainRndPos, 1),
    //    "Low Gain Rnd Pos", 0.0f, 24.0f, 0.0f));

    // Low EQ Frequency
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::lowFreqRndNeg, 1),
    //    "Low Freq Rnd Neg", 0.0f, 480.0f, 0.0f));
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::lowFreqRndPos, 1),
    //    "Low Freq Rnd Pos", 0.0f, 480.0f, 0.0f));

    // Mid EQ Gain
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::midGainRndNeg, 1),
    //    "Mid Gain Rnd Neg", 0.0f, 24.0f, 0.0f));
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::midGainRndPos, 1),
    //    "Mid Gain Rnd Pos", 0.0f, 24.0f, 0.0f));

    // Mid EQ Frequency
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::midFreqRndNeg, 1),
    //    "Mid Freq Rnd Neg", 0.0f, 4800.0f, 0.0f));
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::midFreqRndPos, 1),
    //    "Mid Freq Rnd Pos", 0.0f, 4800.0f, 0.0f));

    // High EQ Gain
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::highGainRndNeg, 1),
    //    "High Gain Rnd Neg", 0.0f, 24.0f, 0.0f));
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::highGainRndPos, 1),
    //    "High Gain Rnd Pos", 0.0f, 24.0f, 0.0f));

    // High EQ Frequency
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::highFreqRndNeg, 1),
    //    "High Freq Rnd Neg", 0.0f, 18000.0f, 0.0f));
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID(ParameterIDs::highFreqRndPos, 1),
    //    "High Freq Rnd Pos", 0.0f, 18000.0f, 0.0f));

    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    // Transient Attack
    //layout.add(std::make_unique<juce::AudioParameterInt>(
    //    juce::ParameterID(ParameterIDs::transientAttackRndNeg, 1),
    //    "Trans Atk Rnd Neg", 0, 127, 0));
    //layout.add(std::make_unique<juce::AudioParameterInt>(
    //    juce::ParameterID(ParameterIDs::transientAttackRndPos, 1),
    //    "Trans Atk Rnd Pos", 0, 127, 0));

    // Transient Decay
    //layout.add(std::make_unique<juce::AudioParameterInt>(
    //    juce::ParameterID(ParameterIDs::transientDecayRndNeg, 1),
    //    "Trans Dec Rnd Neg", 0, 127, 0));
    //layout.add(std::make_unique<juce::AudioParameterInt>(
    //    juce::ParameterID(ParameterIDs::transientDecayRndPos, 1),
    //    "Trans Dec Rnd Pos", 0, 127, 0));

    return layout;
}

//==============================================================================
void NewProjectAudioProcessor::rebuildLoadedIndices()
{
    loadedSlotIndices.clear();
    maxSampleLength = 0;

    for (int i = 0; i < NUM_SAMPLE_SLOTS; ++i)
    {
        if (sampleSlots[i].isLoaded)
        {
            loadedSlotIndices.push_back(i);
            maxSampleLength = std::max(maxSampleLength,
                                       sampleSlots[i].audioBuffer.getNumSamples());
        }
    }

    if (loadedSlotIndices.empty())
        roundRobinIndex = 0;
    else
        roundRobinIndex = roundRobinIndex % (int)loadedSlotIndices.size();
}

void NewProjectAudioProcessor::reshuffleIndices()
{
    shuffledIndices = loadedSlotIndices;  // copy ordered list

    juce::Random rng;
    for (int i = (int)shuffledIndices.size() - 1; i > 0; --i)
    {
        int j = rng.nextInt(i + 1);
        std::swap(shuffledIndices[i], shuffledIndices[j]);
    }
    // Ensure first item of new loop doesn't match last item of previous loop
    if (shuffledIndices.size() > 1 && shuffledIndices[0] == lastPlayedSlot)
    {
        // Swap with a random other position
        int swapWith = 1 + rng.nextInt((int)shuffledIndices.size() - 1);
        std::swap(shuffledIndices[0], shuffledIndices[swapWith]);
    }
    roundRobinIndex = 0;
}

void NewProjectAudioProcessor::resetPlaybackPosition()
{
    roundRobinIndex = 0;
    reshuffleIndices();
    lastPlayedSlot = -1;
}

void NewProjectAudioProcessor::advanceRoundRobin()
{
    if (loadedSlotIndices.empty()) return;

    bool isRandom = apvts.getRawParameterValue(ParameterIDs::playbackMode)->load() > 0.5f;
    int slotIndex = 0;

    if (!isRandom)
    {
        // Series: cycle in order
        slotIndex = loadedSlotIndices[roundRobinIndex % (int)loadedSlotIndices.size()];
        roundRobinIndex = (roundRobinIndex + 1) % (int)loadedSlotIndices.size();
    }
    else
    {
        // Random: exhaust shuffled list before reshuffling (no immediate repeats)
        if (shuffledIndices.empty() || roundRobinIndex >= (int)shuffledIndices.size())
            reshuffleIndices();

        slotIndex = shuffledIndices[roundRobinIndex++];
    }

    lastPlayedSlot = slotIndex;  // ← ADD, track before playing

    if (synthesiser.getNumSounds() == 0)
    {
        auto* newSound = new RRSound();
        newSound->setFromSlot(sampleSlots[slotIndex]);
        synthesiser.addSound(newSound);
    }
    else
    {
        if (auto* sound = dynamic_cast<RRSound*>(synthesiser.getSound(0).get()))
            sound->setFromSlot(sampleSlots[slotIndex]);
    }

    DBG("RR [" + juce::String(isRandom ? "Random" : "Series") + "]: slot "
        + juce::String(slotIndex) + " (" + sampleSlots[slotIndex].displayName + ")");
}

//==============================================================================
void NewProjectAudioProcessor::auditionSample(int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= NUM_SAMPLE_SLOTS || !sampleSlots[slotIndex].isLoaded)
        return;

    // Force the synth to play this specific slot without advancing round-robin
    if (synthesiser.getNumSounds() == 0)
    {
        auto* newSound = new RRSound();
        newSound->setFromSlot(sampleSlots[slotIndex]);
        synthesiser.addSound(newSound);
    }
    else
    {
        if (auto* sound = dynamic_cast<RRSound*>(synthesiser.getSound(0).get()))
            sound->setFromSlot(sampleSlots[slotIndex]);
    }

    synthesiser.noteOn(1, 60, 1.0f);
}

void NewProjectAudioProcessor::swapSamples(int indexA, int indexB)
{
    if (indexA < 0 || indexA >= NUM_SAMPLE_SLOTS ||
        indexB < 0 || indexB >= NUM_SAMPLE_SLOTS || indexA == indexB)
        return;

    std::swap(sampleSlots[indexA], sampleSlots[indexB]);
    sampleLoader.updateSynthesiserSounds();
    rebuildLoadedIndices();
    reshuffleIndices();
}

void NewProjectAudioProcessor::insertSample(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= NUM_SAMPLE_SLOTS ||
        toIndex < 0 || toIndex >= NUM_SAMPLE_SLOTS || fromIndex == toIndex)
        return;

    SampleSlot temp = sampleSlots[fromIndex];

    if (fromIndex < toIndex)
    {
        for (int i = fromIndex; i < toIndex; ++i)
            sampleSlots[i] = sampleSlots[i + 1];
    }
    else
    {
        for (int i = fromIndex; i > toIndex; --i)
            sampleSlots[i] = sampleSlots[i - 1];
    }

    sampleSlots[toIndex] = temp;
    sampleLoader.updateSynthesiserSounds();
    rebuildLoadedIndices();
    reshuffleIndices();
}

//==============================================================================
// User Presets
void NewProjectAudioProcessor::savePreset(const juce::File& file)
{
    DBG("--- savePreset() called: " + file.getFullPathName());

    juce::MemoryBlock data;
    getStateInformation(data);

    if (file.replaceWithData(data.getData(), data.getSize()))
        DBG("Preset saved: " + file.getFullPathName());
    else
        DBG("ERROR: Failed to save preset: " + file.getFullPathName());
}

void NewProjectAudioProcessor::loadPreset(const juce::File& file)
{
    if (!file.existsAsFile())
    {
        DBG("ERROR: Preset file not found: " + file.getFullPathName());
        return;
    }

    juce::MemoryBlock data;
    if (file.loadFileAsData(data))
    {
        setStateInformation(data.getData(), (int)data.getSize());
        DBG("Preset loaded: " + file.getFullPathName());
    }
    else
        DBG("ERROR: Failed to read preset file: " + file.getFullPathName());
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
