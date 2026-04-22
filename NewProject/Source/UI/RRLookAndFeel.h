#pragma once
#include <JuceHeader.h>

namespace RRColors
{
    // Backgrounds — warm cream panel (AKAI S3000-style) with deep recessed wells
    inline const juce::Colour background    { 0xffc7c2b2 };   // warm cream/stone main panel
    inline const juce::Colour backgroundLo  { 0xffb0ab9c };   // slightly darker variant
    inline const juce::Colour headerBg      { 0xff8a8478 };   // darker warm gray header
    inline const juce::Colour sectionBg     { 0xff0a0a0e };   // near-black recessed wells
    inline const juce::Colour sectionBorder { 0xff3c3428 };   // dark warm bevel
    inline const juce::Colour knobBody      { 0xff8a8478 };   // warm stone-gray knob cap
    inline const juce::Colour knobRim       { 0xff141110 };   // dark rim
    inline const juce::Colour knobTrack     { 0xff1a1713 };
    inline const juce::Colour valueText     { 0xffe6f0ff };   // LED readout (pale blue-white)
    inline const juce::Colour companyText   { 0xff4a4030 };   // dark warm ink on cream

    // S612 palette primitives
    inline const juce::Colour s612Red       { 0xffd83030 };   // record / panic — matches original
    inline const juce::Colour s612RedDim    { 0xff8a3030 };   // trigger variant (original muted)
    inline const juce::Colour s612Teal      { 0xff2fa9a1 };   // retained for accents (not for Save/Load)
    inline const juce::Colour amber         { 0xffffb84a };   // amber (retained for accents)
    inline const juce::Colour ledGreen      { 0xff3de070 };   // digital LED green (meter)
    inline const juce::Colour screenPrint   { 0xffe6e1d4 };   // warm white silk-screen label
    inline const juce::Colour liteShade     { 0xffd83030 };   // "Lite" subtitle tint (matches Panic red)
    inline const juce::Colour panelInk      { 0xff2a2418 };   // dark ink text on cream panel

    // LED screen (Sample Pool display) — blue LCD scheme from LED Scren.png reference
    inline const juce::Colour lcdBg         { 0xff1e4eb0 };   // deep blue LCD field
    inline const juce::Colour lcdBgDark     { 0xff163a8c };   // darker blue (inner shadow)
    inline const juce::Colour lcdText       { 0xffe6f0ff };   // white/pale text on blue
    inline const juce::Colour lcdTextDim    { 0xff8cb0e8 };   // dimmed text
    inline const juce::Colour lcdHighlight  { 0xffb0d0ff };   // highlight/brighter text
    inline const juce::Colour lcdRed        { 0xffff9080 };   // warning on blue

    // Section label / accent colors — tuned for visibility on cream panel + dark wells
    inline const juce::Colour pitchCol  { 0xffc8382a };   // red
    inline const juce::Colour ampCol    { 0xff2fa9a1 };   // teal
    inline const juce::Colour envCol    { 0xffb88030 };   // amber (darker for cream readability)
    inline const juce::Colour transCol  { 0xff8858b0 };
    inline const juce::Colour eqLowCol  { 0xff5e8a40 };
    inline const juce::Colour eqMidCol  { 0xff4880b0 };
    inline const juce::Colour eqHighCol { 0xffb87830 };
    inline const juce::Colour toneCol   { 0xff3a8880 };   // muted teal
    inline const juce::Colour trimCol   { 0xffb88030 };   // darker amber — readable on cream
    inline const juce::Colour algoCol   { 0xffc85830 };   // warm orange

    // Rand arc neg colors (lighter/desaturated version of each section)
    inline const juce::Colour pitchNeg  { 0xffe88878 };
    inline const juce::Colour ampNeg    { 0xff7ac8c0 };
    inline const juce::Colour envNeg    { 0xffffd088 };
    inline const juce::Colour transNeg  { 0xffc0a0d8 };
    inline const juce::Colour eqLowNeg  { 0xffa0c888 };
    inline const juce::Colour eqMidNeg  { 0xff98b8d8 };
    inline const juce::Colour eqHighNeg { 0xffe8b880 };
    inline const juce::Colour toneNeg   { 0xff88c8c0 };
    inline const juce::Colour trimNeg   { 0xffffd088 };
}

// Main parameter knobs
// Set rotarySliderFillColourId on each slider to control indicator line color
class RRKnobLAF : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle,
        float rotaryEndAngle, juce::Slider&) override;
};

// Vertical pill toggle: off=SERIES (knob top), on=RANDOM (knob bottom)
class RRToggleLAF : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics&, juce::Button&,
        const juce::Colour&, bool, bool) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
};

// Hardware-style buttons (Load, Save, Trigger, etc.)
class RRButtonLAF : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics&, juce::Button&,
        const juce::Colour&, bool isMouseOver, bool isButtonDown) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
};

class RRNegSliderLAF : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
        float sliderPos, float, float,
        juce::Slider::SliderStyle, juce::Slider&) override;
};

class RRPosSliderLAF : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
        float sliderPos, float, float,
        juce::Slider::SliderStyle, juce::Slider&) override;
};
