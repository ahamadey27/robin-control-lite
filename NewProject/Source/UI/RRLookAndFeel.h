#pragma once
#include <JuceHeader.h>

namespace RRColors
{
    // Backgrounds — muted olive-charcoal (vintage-modern)
    inline const juce::Colour background    { 0xff1e2220 };
    inline const juce::Colour headerBg      { 0xff171b19 };
    inline const juce::Colour sectionBg     { 0xff191d1b };   // section fill (darker than bg)
    inline const juce::Colour sectionBorder { 0xff2c3830 };
    inline const juce::Colour knobBody      { 0xff1a1a1a };
    inline const juce::Colour knobRim       { 0xff2e2e2e };   // subtle rim highlight
    inline const juce::Colour knobTrack     { 0xff282828 };
    inline const juce::Colour valueText     { 0xff7a7a7a };
    inline const juce::Colour companyText   { 0xff3a4a3e };

    // Section label / accent colors — muted, sophisticated
    inline const juce::Colour pitchCol  { 0xffb86e50 };   // warm terra cotta
    inline const juce::Colour ampCol    { 0xff5ea85e };    // muted sage green
    inline const juce::Colour envCol    { 0xffb89840 };    // warm gold (premium)
    inline const juce::Colour transCol  { 0xff7858b0 };    // muted purple (premium)
    inline const juce::Colour eqLowCol  { 0xff5e8a50 };    // (premium)
    inline const juce::Colour eqMidCol  { 0xff4880b0 };    // (premium)
    inline const juce::Colour eqHighCol { 0xffb88840 };    // (premium)
    inline const juce::Colour toneCol   { 0xff4890a8 };    // dusty teal
    inline const juce::Colour trimCol   { 0xffb89858 };    // warm amber
    inline const juce::Colour algoCol   { 0xff50a878 };    // muted emerald

    // Rand arc neg colors (lighter/desaturated version of each section)
    inline const juce::Colour pitchNeg  { 0xffd8a888 };
    inline const juce::Colour ampNeg    { 0xff98c898 };
    inline const juce::Colour envNeg    { 0xffd8c080 };
    inline const juce::Colour transNeg  { 0xffb098d0 };
    inline const juce::Colour eqLowNeg  { 0xff98b888 };
    inline const juce::Colour eqMidNeg  { 0xff90b8d0 };
    inline const juce::Colour eqHighNeg { 0xffd8b880 };
    inline const juce::Colour toneNeg   { 0xff88c0d0 };
    inline const juce::Colour trimNeg   { 0xffd8c090 };
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
