/*
  ==============================================================================

    Parameter ID Constants for Round Robin Lite

    This file defines all parameter identifiers used in the plugin's
    AudioProcessorValueTreeState (APVTS) system.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//==============================================================================
/**
 * ParameterIDs namespace - Contains all parameter identifier strings.
 *
 * These IDs are used for:
 * - APVTS parameter registration
 * - DAW automation
 * - Preset save/load
 * - UI control attachment
 *
 * IMPORTANT: Never change these strings after release, as they are
 * stored in DAW projects and presets!
 */
namespace ParameterIDs
{
    //==============================================================================
    // GLOBAL PITCH CONTROLS

    /**
     * SEMITONE - Global pitch shift in semitones
     * Range: -12 to +12 semitones
     * Default: 0 (no shift)
     * Step: 1 semitone
     *
     * Shifts all samples up or down by whole semitones.
     * Combined with Fine Tune for precise pitch adjustment.
     */
    inline constexpr auto semitone = "semitone";

    /**
     * FINE_TUNE - Global fine pitch adjustment in cents
     * Range: -100 to +100 cents (1 semitone = 100 cents)
     * Default: 0 (no adjustment)
     * Step: 1 cent
     *
     * Allows precise pitch tuning within a semitone range.
     * 100 cents = 1 semitone.
     */
    inline constexpr auto fineTune = "fineTune";

    //==============================================================================
    // VOLUME

    /**
     * VOLUME - Global output volume
     * Range: 0.0 to 1.0 (linear) OR -60dB to 0dB (will decide in implementation)
     * Default: 0.75 (roughly -6dB)
     *
     * Controls the overall output level of all samples.
     */
    inline constexpr auto volume = "volume";

    //==============================================================================
    // Pan
    inline constexpr auto pan = "pan";
    inline constexpr auto panRndNeg = "panRndNeg";
    inline constexpr auto panRndPos = "panRndPos";




    //==============================================================================
    // 3-BAND EQ

    /**
     * LOW_GAIN - Low frequency band gain
     * Range: -24dB to +24dB
     * Default: 0dB (no boost/cut)
     * Filter Type: Low Shelf
     *
     * Boosts or cuts low frequencies below the Low Freq cutoff.
     */
    inline constexpr auto lowGain = "lowGain";

    /**
     * LOW_FREQ - Low frequency band cutoff frequency
     * Range: 20Hz to 500Hz
     * Default: 100Hz
     * Scaling: Logarithmic (sounds more natural)
     *
     * Sets the frequency where low shelf filter operates.
     */
    inline constexpr auto lowFreq = "lowFreq";

    /**
     * MID_GAIN - Mid frequency band gain
     * Range: -24dB to +24dB
     * Default: 0dB (no boost/cut)
     * Filter Type: Peak/Bell
     *
     * Boosts or cuts midrange frequencies around Mid Freq center.
     */
    inline constexpr auto midGain = "midGain";

    /**
     * MID_FREQ - Mid frequency band center frequency
     * Range: 200Hz to 5000Hz
     * Default: 1000Hz (1kHz)
     * Scaling: Logarithmic
     *
     * Sets the center frequency for the mid peak filter.
     */
    inline constexpr auto midFreq = "midFreq";

    /**
     * HIGH_GAIN - High frequency band gain
     * Range: -24dB to +24dB
     * Default: 0dB (no boost/cut)
     * Filter Type: High Shelf
     *
     * Boosts or cuts high frequencies above the High Freq cutoff.
     */
    inline constexpr auto highGain = "highGain";

    /**
     * HIGH_FREQ - High frequency band cutoff frequency
     * Range: 2000Hz to 20000Hz
     * Default: 5000Hz (5kHz)
     * Scaling: Logarithmic
     *
     * Sets the frequency where high shelf filter operates.
     */
    inline constexpr auto highFreq = "highFreq";

    //==============================================================================
// AMPLITUDE ENVELOPE

/**
 * ENV_ATTACK - Amplitude envelope attack time
 * Range: 0.0 to 1000.0 milliseconds
 * Default: 0.0ms (instant attack)
 * Scaling: Linear or slightly exponential
 *
 * Controls how quickly the sample reaches full volume after note-on.
 * 0ms = instant (no fade-in)
 * Higher values = slower fade-in
 */
    inline constexpr auto envAttack = "envAttack";

    /**
     * ENV_DECAY - Amplitude envelope decay/release time
     * Range: 0.0 to 5000.0 milliseconds
     * Default: 5000.0ms (short natural release)
     * Scaling: Linear or exponential
     *
     * Controls how long the sample takes to fade out after note-off.
     * Lower values = quick fade-out (tight, punchy)
     * Higher values = long fade-out (sustained, ambient)
     */
    inline constexpr auto envDecay = "envDecay";

    //==============================================================================
    // PLAYBACK MODE

    inline constexpr auto playbackMode = "playbackMode";  // false = Series, true = Random

    //==============================================================================
    // TRANSIENT MASTER

    /**
     * TRANSIENT_ATTACK - Attack transient enhancement/reduction
     * Range: -127 to +127
     * Default: 0 (no change)
     *
     * Positive values: Emphasize/sharpen attack transients (punchier)
     * Negative values: Soften/reduce attack transients (smoother)
     *
     * Useful for making footsteps more or less percussive.
     */
    inline constexpr auto transientAttack = "transientAttack";

    /**
     * TRANSIENT_DECAY - Sustain/decay envelope shaping
     * Range: -127 to +127
     * Default: 0 (no change)
     *
     * Positive values: Extend sustain/decay (longer tail)
     * Negative values: Shorten sustain/decay (tighter sound)
     *
     * Useful for adjusting how quickly sounds fade out.
     */
    inline constexpr auto transientDecay = "transientDecay";

    // RANDOMIZATION PARAMETER IDs (22 total: 2 per randomizable param)
    const juce::String semitoneRndNeg = "semitoneRndNeg";
    const juce::String semitoneRndPos = "semitoneRndPos";
    const juce::String fineTuneRndNeg = "fineTuneRndNeg";
    const juce::String fineTuneRndPos = "fineTuneRndPos";
    const juce::String volumeRndNeg = "volumeRndNeg";
    const juce::String volumeRndPos = "volumeRndPos";
    const juce::String lowGainRndNeg = "lowGainRndNeg";
    const juce::String lowGainRndPos = "lowGainRndPos";
    const juce::String lowFreqRndNeg = "lowFreqRndNeg";
    const juce::String lowFreqRndPos = "lowFreqRndPos";
    const juce::String midGainRndNeg = "midGainRndNeg";
    const juce::String midGainRndPos = "midGainRndPos";
    const juce::String midFreqRndNeg = "midFreqRndNeg";
    const juce::String midFreqRndPos = "midFreqRndPos";
    const juce::String highGainRndNeg = "highGainRndNeg";
    const juce::String highGainRndPos = "highGainRndPos";
    const juce::String highFreqRndNeg = "highFreqRndNeg";
    const juce::String highFreqRndPos = "highFreqRndPos";
    const juce::String transientAttackRndNeg = "transientAttackRndNeg";
    const juce::String transientAttackRndPos = "transientAttackRndPos";
    const juce::String transientDecayRndNeg = "transientDecayRndNeg";
    const juce::String transientDecayRndPos = "transientDecayRndPos";
    const juce::String envAttackRndNeg = "envAttackRndNeg";
    const juce::String envAttackRndPos = "envAttackRndPos";
    const juce::String envDecayRndNeg = "envDecayRndNeg";
    const juce::String envDecayRndPos = "envDecayRndPos";

    //==============================================================================
    // TONE CONTROL (Lite replacement for 3-Band EQ)

    inline constexpr auto toneLow  = "toneLow";
    inline constexpr auto toneHigh = "toneHigh";

    const juce::String toneLowRndNeg  = "toneLowRndNeg";
    const juce::String toneLowRndPos  = "toneLowRndPos";
    const juce::String toneHighRndNeg = "toneHighRndNeg";
    const juce::String toneHighRndPos = "toneHighRndPos";

    //==============================================================================
    // SAMPLE START/END

    inline constexpr auto sampleStart = "sampleStart";
    inline constexpr auto sampleEnd   = "sampleEnd";

    const juce::String sampleStartRndNeg = "sampleStartRndNeg";
    const juce::String sampleStartRndPos = "sampleStartRndPos";
    const juce::String sampleEndRndNeg   = "sampleEndRndNeg";
    const juce::String sampleEndRndPos   = "sampleEndRndPos";

    //==============================================================================
    // RANDOM ALGORITHM

    inline constexpr auto randomAlgorithm = "randomAlgorithm";

    //==============================================================================
    // PARAMETER COUNT (useful for validation)

    /**
     * Total number of parameters in the plugin.
     * Update this if you add/remove parameters!
     */
    inline constexpr int totalParameters = 48;

} 