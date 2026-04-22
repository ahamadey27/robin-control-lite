# Round Robin Lite

[![JUCE](https://img.shields.io/badge/JUCE-8.0+-blue.svg)](https://juce.com/)
[![VST3](https://img.shields.io/badge/VST3-✓-green.svg)](https://www.steinberg.net/vst3sdk)
[![License](https://img.shields.io/badge/License-Proprietary-red.svg)]()

A monophonic MIDI instrument VST3 plugin designed specifically for footstep sounds and similar audio samples. Built with the JUCE framework in C++.

## 🎯 Project Overview

Round Robin Lite is a sample-based MIDI instrument that allows users to quickly map and manipulate audio samples across a MIDI keyboard with intelligent pitch mapping and creative sound-shaping tools. This MVP (Minimum Viable Product) version serves as both a functional audio plugin and a learning platform for JUCE audio development.

### Key Features (MVP)

- **20 Sample Slots** - Load up to 20 mono audio samples (WAV, AIFF, FLAC, OGG)
- **Intelligent MIDI Mapping** - Samples mapped from C0 to C4 in paired white keys with automatic pitch shifting
- **Monophonic Playback** - Single-voice playback ideal for footsteps and percussive sounds
- **Global Pitch Control** - Semitone and Fine Tune controls with randomization
- **3-Band EQ** - Low, Mid, and High bands with adjustable gain and frequency
- **Transient Master** - Attack and Decay controls for sound shaping
- **Parameter Randomization** - Add variation to pitch, volume, EQ, and transients within custom ranges
- **VST3 Format** - Compatible with major DAWs
- **Standalone Mode** - Test and use without a DAW

## 🎹 MIDI Note Mapping System

Round Robin Lite uses a unique paired-key mapping system optimized for creating realistic footstep patterns:

| Key Pair | MIDI Notes | Semitone Offset |
|----------|------------|-----------------|
| C0/D0    | 36/38      | -7              |
| E0/F0    | 40/41      | -6              |
| G0/A0    | 43/45      | -5              |
| B0/C1    | 47/48      | -4              |
| E1/F1    | 52/53      | -3              |
| G1/A1    | 55/57      | -2              |
| B1/C2    | 59/60      | -1              |
| **C2/D2**| **48/50**  | **0 (Root)**    |
| E2/F2    | 64/65      | +1              |
| G2/A2    | 67/69      | +2              |

**Why paired keys?** This design allows users to alternate between two notes to create natural-sounding left/right footstep patterns or add variation to repetitive sounds.

## 🛠️ Tech Stack

- **Framework:** JUCE 8.0+ (C++)
- **IDE:** Visual Studio 2022/2026
- **Plugin Format:** VST3 (with Standalone)
- **Build System:** Projucer + MSBuild
- **Version Control:** Git/GitHub

## 📋 Development Status

This project follows an 11-phase development roadmap with approximately 8-10 weeks to MVP completion.

**Current Phase:** Phase 1 - Foundation & Setup ✅  
**Progress:** See [spec.md](spec.md) for detailed task tracking

### Completed Phases
- [x] Phase 1: Foundation & Setup (Development environment, project structure, version control)

### In Progress
- [ ] Phase 2: Core Audio Engine (Synthesiser architecture, voice/sound classes)

### Upcoming
- Phase 3: Parameter System with APVTS
- Phase 4: Sample Management System
- Phase 5: Basic User Interface
- Phase 6: DSP Implementation (EQ & Transient Master)
- Phase 7: Randomization System
- Phase 8: Advanced Features & Polish
- Phase 9: Testing & Quality Assurance
- Phase 10: Release Preparation
- Phase 11: Post-Launch & Iteration

## 🚀 Getting Started

### Prerequisites

- **Visual Studio 2022** with C++ Desktop Development workload
- **JUCE Framework** (latest stable version from [juce.com](https://juce.com))
- **Projucer** (included with JUCE)
- **VST3 SDK** (included with JUCE)

### Installation & Build

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/round-robin-lite.git
   cd round-robin-lite
   ```

2. **Set up JUCE paths**
   - Open Projucer
   - Go to File → Global Paths
   - Set JUCE Modules path to your JUCE installation's `modules` folder

3. **Open and configure the project**
   - Open `NewProject/NewProject.jucer` in Projucer
   - Verify all module paths are correct
   - Click "Save Project and Open in IDE"

4. **Build in Visual Studio**
   - Open the generated `.sln` file
   - Select Debug or Release configuration
   - Build the solution (Ctrl+Shift+B)

5. **Locate the plugin**
   - VST3: `NewProject/Builds/VisualStudio2026/x64/Debug/VST3/`
   - Standalone: `NewProject/Builds/VisualStudio2026/x64/Debug/Standalone Plugin/`

### Testing

Use the **JUCE AudioPluginHost** (found in `JUCE/extras/AudioPluginHost`) to test the plugin:
1. Build and launch AudioPluginHost
2. Options → Edit the list of available plug-ins
3. Scan for the Round Robin Lite VST3
4. Add the plugin to the audio graph
5. Connect a MIDI input device or use the virtual keyboard

## 📁 Project Structure

```
round-robin-lite/
├── README.md              # This file
├── CHANGELOG.md           # Version history
├── spec.md                # Detailed development specification
├── .gitignore            # Git ignore rules
└── NewProject/           # JUCE plugin project
    ├── NewProject.jucer  # Projucer project file
    └── Source/
        ├── Audio/        # Audio engine classes (synthesiser, voices)
        ├── DSP/          # Signal processing (EQ, transient shaping)
        ├── UI/           # GUI components
        ├── Data/         # Sample management and data structures
        ├── Parameters/   # Parameter definitions and IDs
        ├── PluginProcessor.h/cpp    # Main audio processor
        └── PluginEditor.h/cpp       # Main UI editor
```

## 🎛️ Parameters

### Pitch Controls
- **Semitone:** -12 to +12 semitones (global)
- **Fine Tune:** -100 to +100 cents (global)

### Volume
- **Volume:** 0.0 to 1.0 (linear) or -60dB to 0dB

### 3-Band EQ
- **Low Band:** 20Hz - 500Hz, ±24dB gain
- **Mid Band:** 200Hz - 5kHz, ±24dB gain
- **High Band:** 2kHz - 20kHz, ±24dB gain

### Transient Master
- **Attack:** -127 to +127 (transient enhancement/reduction)
- **Decay:** -127 to +127 (sustain/release shaping)

### Randomization
All parameters support randomization within user-defined min/max bounds, allowing for organic variation in sound.

## 🎨 Future Features (Pro Version)

- More sample slots (40-60+)
- True round-robin with multiple samples per note
- 5-band parametric EQ
- Advanced modulation system (LFOs, envelopes)
- Per-sample parameter control
- Velocity sensitivity
- Built-in effects (reverb, delay, compression)
- Sample editor with trim, fade, normalize
- Custom MIDI mapping
- Sample library browser

## 📚 Learning Resources

This project serves as a learning platform for JUCE development. Key concepts covered:

- JUCE Synthesiser architecture (SynthesiserVoice, SynthesiserSound)
- AudioProcessorValueTreeState (APVTS) for parameter management
- Custom UI components and graphics
- DSP implementation (filters, envelope followers)
- File I/O and sample management
- MIDI message handling
- Plugin state save/recall
- Cross-platform audio development

### Recommended Resources
- [JUCE Documentation](https://docs.juce.com)
- [JUCE Tutorials](https://juce.com/learn/tutorials)
- [The Audio Programmer (YouTube)](https://www.youtube.com/c/TheAudioProgrammer)
- "Designing Audio Effect Plugins in C++" by Will Pirkle

## 🤝 Contributing

This is a personal learning project, but feedback and suggestions are welcome! Feel free to:
- Open issues for bugs or feature requests
- Submit pull requests with improvements
- Share your experience using the plugin

## 📄 License

**Proprietary** - All rights reserved. This is commercial software in development.

The MVP "Round Robin Lite" will be released as a free/paid product. A more advanced "Pro" version is planned for future development.

## 👤 Author

**Alex** - [GitHub Profile](https://github.com/yourusername)

## 🙏 Acknowledgments

- Built with [JUCE Framework](https://juce.com) by Roli
- VST is a trademark of Steinberg Media Technologies GmbH

---

**Status:** 🚧 In Active Development | **Target MVP Release:** Q2 2025

For detailed development progress, see [spec.md](spec.md)