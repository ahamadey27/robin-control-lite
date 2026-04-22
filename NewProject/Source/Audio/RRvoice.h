/*
  ==============================================================================

    Custom SynthesiserVoice class for Round Robin Lite.
    Handles playback of RRSound samples with pitch shifting and ADSR envelope.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "RRSound.h"

#include "../DSP/RandomizationEngine.h"
#include "../DSP/RandomAlgorithmTable.h"
#include "../Parameters/ParametersIDs.h"
//==============================================================================
/**
 * RRVoice - Plays back RRSound samples when triggered by MIDI notes.
 *
 * This voice handles:
 * - Sample playback with pitch shifting
 * - ADSR envelope application
 * - Monophonic behavior (one note at a time)
 * - Smooth voice termination
 */
class RRVoice : public juce::SynthesiserVoice
{
public:
    // Constructor & Destructor
    RRVoice();
    ~RRVoice() override;

    void updateGlobalParameters(float semitones, float cents, float attackMs, float decayMs);
    void setMaxPoolSampleLength(int length) { maxPoolSampleLength = length; }

    //==============================================================================
    // JUCE SynthesiserVoice Required Methods

    /**
     * Checks if this voice can play the given sound.
     * We only want to play RRSound objects, not other sound types.
     * @param sound The sound to check
     * @return true if sound is an RRSound
     */
    bool canPlaySound(juce::SynthesiserSound* sound) override;

    /**
     * Called when a MIDI note-on event occurs.
     * Sets up the voice to start playing a sample.
     * @param midiNoteNumber The MIDI note (0-127)
     * @param velocity The note velocity (0.0-1.0)
     * @param sound The RRSound to play
     * @param currentPitchWheelPosition Current pitch wheel value (-1.0 to 1.0)
     */
    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound* sound,
        int currentPitchWheelPosition) override;

    /**
     * Called when a MIDI note-off event occurs.
     * Triggers the release phase of the envelope.
     * @param velocity Note-off velocity (usually ignored)
     * @param allowTailOff If true, let envelope release naturally
     */
    void stopNote(float velocity, bool allowTailOff) override;

    /**
     * Renders the next block of audio for this voice.
     * This is where the actual sample playback and processing happens.
     * @param outputBuffer The buffer to write audio into
     * @param startSample Which sample in the buffer to start at
     * @param numSamples How many samples to render
     */
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
        int startSample, int numSamples) override;

    /**
     * Called when pitch wheel is moved (not implemented for MVP).
     */
    void pitchWheelMoved(int newPitchWheelValue) override;

    /**
     * Called when controller data is received (not implemented for MVP).
     */
    void controllerMoved(int controllerNumber, int newControllerValue) override;

    void setRandomizationReferences(RandomizationEngine* engine,
        juce::AudioProcessorValueTreeState* params);

    float getRandomizedVolume() const { return randomizedVolume; }
    // COMMENTED FOR LITE — ACTIVE IN PREMIUM
    //float getRandomizedLowGain() const { return randomizedLowGain; }
    //float getRandomizedLowFreq() const { return randomizedLowFreq; }
    //float getRandomizedMidGain() const { return randomizedMidGain; }
    //float getRandomizedMidFreq() const { return randomizedMidFreq; }
    //float getRandomizedHighGain() const { return randomizedHighGain; }
    //float getRandomizedHighFreq() const { return randomizedHighFreq; }
    //float getRandomizedTransientAttack() const { return randomizedTransientAttack; }
    //float getRandomizedTransientDecay() const { return randomizedTransientDecay; }

    float getRandomizedToneLow()  const { return randomizedToneLow; }
    float getRandomizedToneHigh() const { return randomizedToneHigh; }

private:
    //==============================================================================
   // Member Variables

   // Playback state
    double sourceSamplePosition = 0.0;
    double pitchRatio = 1.0;
    bool isPlaying = false;

    // Global parameters (updated before each note)  ← ADD THESE 4 LINES
    float currentSemitones = 0.0f;
    float currentCents = 0.0f;
    float currentAttackMs = 0.0f;
    float currentDecayMs = 100.0f;

    // ADSR Envelope
    juce::ADSR envelope;
    juce::ADSR::Parameters envelopeParams;

    // Reference to current sound
    RRSound* currentSound = nullptr;

    juce::AudioBuffer<float> voiceBuffer;   // voice owns its own copy
    int cachedSampleLength = 0;

    // Global parameters (non-randomized base values)
    float globalSemitones = 0.0f;
    float globalCents = 0.0f;
    float globalAttackMs = 0.0f;
    float globalDecayMs = 0.0f;

    // Per-note randomized values
    float randomizedSemitones = 0.0f;
    float randomizedCents = 0.0f;
    float randomizedAttackMs = 0.0f;
    float randomizedDecayMs = 5000.0f;
    float randomizedVolume = 0.75f;
    float randomizedPan = 0.0f;
    float randomizedLowGain = 0.0f;
    float randomizedLowFreq = 100.0f;
    float randomizedMidGain = 0.0f;
    float randomizedMidFreq = 1000.0f;
    float randomizedHighGain = 0.0f;
    float randomizedHighFreq = 5000.0f;
    float randomizedTransientAttack = 0.0f;
    float randomizedTransientDecay = 0.0f;
    float randomizedToneLow = 0.0f;
    float randomizedToneHigh = 0.0f;
    float randomizedSampleStart = 0.0f;
    float randomizedSampleEnd = 100.0f;
    int playbackStartSample = 0;
    int playbackEndSample = 0;
    int fadeSamples = 0;
    int maxPoolSampleLength = 0;  // longest sample in pool (for relative start/end)

    bool releaseTriggered = false;

    // Cached parameter pointers (set once, read many times)
    struct RandomizationPointers
    {
        std::atomic<float>* semitone = nullptr;
        std::atomic<float>* fineTune = nullptr;
        std::atomic<float>* volume = nullptr;
        std::atomic<float>* pan = nullptr;
        std::atomic<float>* panNeg = nullptr;
        std::atomic<float>* panPos = nullptr;
        std::atomic<float>* envAttack = nullptr;
        std::atomic<float>* envDecay = nullptr;
        std::atomic<float>* lowGain = nullptr;
        std::atomic<float>* lowFreq = nullptr;
        std::atomic<float>* midGain = nullptr;
        std::atomic<float>* midFreq = nullptr;
        std::atomic<float>* highGain = nullptr;
        std::atomic<float>* highFreq = nullptr;
        std::atomic<float>* transAtk = nullptr;
        std::atomic<float>* transDec = nullptr;

        std::atomic<float>* semitoneNeg = nullptr;  std::atomic<float>* semitonePos = nullptr;
        std::atomic<float>* fineNeg = nullptr;      std::atomic<float>* finePos = nullptr;
        std::atomic<float>* volumeNeg = nullptr;    std::atomic<float>* volumePos = nullptr;
        std::atomic<float>* atkNeg = nullptr;       std::atomic<float>* atkPos = nullptr;
        std::atomic<float>* decNeg = nullptr;       std::atomic<float>* decPos = nullptr;
        std::atomic<float>* lowGainNeg = nullptr;   std::atomic<float>* lowGainPos = nullptr;
        std::atomic<float>* lowFreqNeg = nullptr;   std::atomic<float>* lowFreqPos = nullptr;
        std::atomic<float>* midGainNeg = nullptr;   std::atomic<float>* midGainPos = nullptr;
        std::atomic<float>* midFreqNeg = nullptr;   std::atomic<float>* midFreqPos = nullptr;
        std::atomic<float>* highGainNeg = nullptr;  std::atomic<float>* highGainPos = nullptr;
        std::atomic<float>* highFreqNeg = nullptr;  std::atomic<float>* highFreqPos = nullptr;
        std::atomic<float>* transAtkNeg = nullptr;  std::atomic<float>* transAtkPos = nullptr;
        std::atomic<float>* transDecNeg = nullptr;  std::atomic<float>* transDecPos = nullptr;

        std::atomic<float>* toneLow = nullptr;
        std::atomic<float>* toneHigh = nullptr;
        std::atomic<float>* toneLowNeg = nullptr;   std::atomic<float>* toneLowPos = nullptr;
        std::atomic<float>* toneHighNeg = nullptr;  std::atomic<float>* toneHighPos = nullptr;

        std::atomic<float>* sampleStart = nullptr;
        std::atomic<float>* sampleEnd = nullptr;
        std::atomic<float>* sampleStartNeg = nullptr;  std::atomic<float>* sampleStartPos = nullptr;
        std::atomic<float>* sampleEndNeg = nullptr;    std::atomic<float>* sampleEndPos = nullptr;

        std::atomic<float>* randomAlgorithm = nullptr;
    } rndPtrs;

    // Reference to randomization engine and parameters
    RandomizationEngine* randEngine = nullptr;
    juce::AudioProcessorValueTreeState* apvts = nullptr;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RRVoice)
};
