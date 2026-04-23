#pragma once
#include <JuceHeader.h>

namespace RRColors
{
    // Backgrounds — INVERTED tones: warm-gray outer frame, cream header + wells
    inline const juce::Colour background    { 0xff8a8478 };   // warm gray outer panel / footer field
    inline const juce::Colour backgroundLo  { 0xff7a7468 };   // darker warm gray (brush grain)
    inline const juce::Colour headerBg      { 0xffc7c2b2 };   // cream header strip
    inline const juce::Colour sectionBg     { 0xffe4dfcd };   // brighter cream wells (readable surface for colored labels)
    inline const juce::Colour sectionBgDark { 0xffcdc7b4 };   // slightly darker cream (gradient bottom)
    inline const juce::Colour sectionBorder { 0xff2a2418 };   // dark warm bevel for depth
    inline const juce::Colour knobBody      { 0xff8a8478 };   // warm gray cap (pops against cream wells)
    inline const juce::Colour knobRim       { 0xff1f1710 };   // warm near-black rim (tinted umber for the cream panel)
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

    // Section label / accent colors — darker matte tones chosen for contrast on cream wells
    // Green / red / orange are direct S612 profile-1 picks. Tone (deep teal) and trim
    // (deep bronze) are custom but sit squarely in the S612/S3000 hardware aesthetic.
    inline const juce::Colour pitchCol  { 0xff9c2a2e };   // deeper red (was #c84045 — too bright/washed on cream)
    inline const juce::Colour ampCol    { 0xff00704f };   // deeper S612 green (was #009065)
    inline const juce::Colour envCol    { 0xffb88030 };   // amber (premium — unused in Lite)
    inline const juce::Colour transCol  { 0xff8858b0 };
    inline const juce::Colour eqLowCol  { 0xff5e8a40 };
    inline const juce::Colour eqMidCol  { 0xff4880b0 };
    inline const juce::Colour eqHighCol { 0xffb87830 };
    inline const juce::Colour toneCol   { 0xff2a5a72 };   // deep slate-teal (cool counterweight to the warm sections)
    inline const juce::Colour trimCol   { 0xff7a4820 };   // deep bronze/amber (reads strongly on cream)
    inline const juce::Colour algoCol   { 0xffb84a18 };   // deeper S612 orange (was #da5f20)

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

    void drawLabel(juce::Graphics&, juce::Label&) override;
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
