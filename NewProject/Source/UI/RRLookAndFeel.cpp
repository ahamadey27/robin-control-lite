#include "RRLookAndFeel.h"

//==============================================================================
// RRLookAndFeel.cpp

void RRKnobLAF::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    const float cx = x + width  * 0.5f;
    const float cy = y + height * 0.5f;

    const float bodyRadius = juce::jmin((float)width, (float)height) * 0.5f - 3.0f;

    // ── Recessed well behind the knob (dark ring) ───────────────────────────
    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.fillEllipse(cx - bodyRadius - 2.0f, cy - bodyRadius - 1.0f,
                  (bodyRadius + 2.0f) * 2.0f, (bodyRadius + 2.0f) * 2.0f);

    // ── Drop shadow under cap ───────────────────────────────────────────────
    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.fillEllipse(cx - bodyRadius + 0.5f, cy - bodyRadius + 2.0f,
                  bodyRadius * 2.0f, bodyRadius * 2.0f);

    // ── Dark rim (collar around cap) ────────────────────────────────────────
    g.setColour(RRColors::knobRim);
    g.fillEllipse(cx - bodyRadius, cy - bodyRadius,
                  bodyRadius * 2.0f, bodyRadius * 2.0f);

    // ── Warm stone-gray cap body ────────────────────────────────────────────
    {
        const float inner = bodyRadius - 2.0f;
        juce::ColourGradient grad(
            RRColors::knobBody.brighter(0.18f), cx, cy - inner * 0.6f,
            RRColors::knobBody.darker(0.35f),   cx, cy + inner * 0.9f,
            false);
        g.setGradientFill(grad);
        g.fillEllipse(cx - inner, cy - inner, inner * 2.0f, inner * 2.0f);
    }

    // ── Soft top highlight (plastic sheen) ──────────────────────────────────
    {
        const float hl = bodyRadius * 0.7f;
        juce::ColourGradient sheen(
            juce::Colours::white.withAlpha(0.22f), cx, cy - bodyRadius * 0.6f,
            juce::Colours::transparentWhite,       cx, cy + bodyRadius * 0.1f,
            false);
        g.setGradientFill(sheen);
        g.fillEllipse(cx - hl, cy - bodyRadius + 1.5f, hl * 2.0f, hl * 1.3f);
    }

    // ── Inner ring (subtle dark edge line) ──────────────────────────────────
    g.setColour(juce::Colour(0xff2a2418).withAlpha(0.55f));
    g.drawEllipse(cx - bodyRadius + 2.0f, cy - bodyRadius + 2.0f,
                  (bodyRadius - 2.0f) * 2.0f, (bodyRadius - 2.0f) * 2.0f, 0.8f);

    // ── Indicator: section-color line from mid-radius to edge (no black) ────
    const juce::Colour tipCol = slider.findColour(juce::Slider::rotarySliderFillColourId);
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const float lineInner = bodyRadius * 0.5f;   // halfway down the knob
    const float lineOuter = bodyRadius - 3.0f;   // just inside the rim
    const float sinA = std::sin(angle);
    const float cosA = std::cos(angle);

    // Subtle shadow for depth
    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.drawLine(cx + sinA * lineInner + 0.5f, cy - cosA * lineInner + 0.5f,
               cx + sinA * lineOuter + 0.5f, cy - cosA * lineOuter + 0.5f, 3.6f);

    // Main indicator — section color, thick
    g.setColour(tipCol);
    g.drawLine(cx + sinA * lineInner, cy - cosA * lineInner,
               cx + sinA * lineOuter, cy - cosA * lineOuter, 3.2f);
}

//==============================================================================
// RRButtonLAF — hardware-style buttons

void RRButtonLAF::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour& baseColour, bool isMouseOver, bool isButtonDown)
{
    auto b = button.getLocalBounds().toFloat().reduced(0.5f);
    auto accent = baseColour;   // S612-style accent strip color comes from buttonColourId

    if (isButtonDown)
        accent = accent.brighter(0.2f);
    else if (isMouseOver)
        accent = accent.brighter(0.1f);

    // Drop shadow beneath (sits on panel)
    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.fillRoundedRectangle(b.translated(0.0f, 1.5f), 2.5f);

    // Dark button body — charcoal with slight top-to-bottom gradient
    {
        juce::ColourGradient body(
            juce::Colour(0xff2a2622), b.getX(), b.getY(),
            juce::Colour(0xff14110e), b.getX(), b.getBottom(),
            false);
        g.setGradientFill(body);
        g.fillRoundedRectangle(b, 2.5f);
    }

    // Colored accent strip on top (the S612 label-box treatment)
    {
        auto strip = b.reduced(2.5f, 0.0f);
        strip = strip.removeFromTop(juce::jmin(strip.getHeight() * 0.38f, 9.0f));
        strip.translate(0.0f, 2.0f);

        juce::ColourGradient stripGrad(
            accent.brighter(0.15f), strip.getX(), strip.getY(),
            accent.darker(0.1f),    strip.getX(), strip.getBottom(),
            false);
        g.setGradientFill(stripGrad);
        g.fillRoundedRectangle(strip, 1.5f);

        // Tiny highlight line along top of strip
        g.setColour(juce::Colours::white.withAlpha(isButtonDown ? 0.1f : 0.25f));
        g.fillRect(strip.getX() + 1.0f, strip.getY() + 0.5f,
                   strip.getWidth() - 2.0f, 0.8f);
    }

    // Outer border (warm dark bevel)
    g.setColour(juce::Colour(0xff0a0806));
    g.drawRoundedRectangle(b, 2.5f, 1.0f);

    // Pressed-in shadow when held
    if (isButtonDown)
    {
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.drawRoundedRectangle(b.reduced(1.0f), 2.0f, 1.2f);
    }
}

void RRButtonLAF::drawButtonText(juce::Graphics& g, juce::TextButton& button,
    bool isMouseOver, bool isButtonDown)
{
    // Text sits below the colored accent strip (S612 screen-print label)
    auto area = button.getLocalBounds();
    area.removeFromTop(juce::jmin(area.getHeight() / 3, 10));

    g.setFont(juce::Font(juce::FontOptions(10.0f)).boldened());
    g.setColour(RRColors::screenPrint.withAlpha(button.isEnabled() ? 0.92f : 0.4f));
    g.drawText(button.getButtonText(), area, juce::Justification::centred);
}

//==============================================================================
static void drawRndSlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, bool isNeg)
{
    const float trackY = y + height * 0.5f;
    const float trackH = 4.0f;
    const float thumbW = 10.0f;   // S612 slider thumb — elongated, like the START/SPLICE cap
    const float thumbH = 10.0f;

    // Groove (dark recessed channel with inner shadow)
    g.setColour(juce::Colour(0xff0a0806));
    g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f, (float)width, trackH, 2.0f);
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.drawRoundedRectangle((float)x + 0.5f, trackY - trackH * 0.5f + 0.5f,
        (float)width - 1.0f, trackH - 1.0f, 2.0f, 0.6f);

    // Fill — amber for neg, teal for pos (S612 hardware palette)
    if (isNeg)
    {
        g.setColour(RRColors::s612Red.withAlpha(0.75f));
        g.fillRoundedRectangle(sliderPos, trackY - trackH * 0.5f,
            (float)(x + width) - sliderPos, trackH, 2.0f);
    }
    else
    {
        g.setColour(RRColors::s612Teal.withAlpha(0.75f));
        g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f,
            sliderPos - (float)x, trackH, 2.0f);
    }

    // Cream thumb (S612_Slider.png reference — bone-colored oval cap)
    const float tx = sliderPos - thumbW * 0.5f;
    const float ty = trackY - thumbH * 0.5f;

    // Thumb shadow
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.fillRoundedRectangle(tx + 0.5f, ty + 1.5f, thumbW, thumbH, 2.0f);

    // Thumb body — cream gradient
    juce::ColourGradient thumbGrad(
        RRColors::knobBody.brighter(0.12f), tx, ty,
        RRColors::knobBody.darker(0.2f),    tx, ty + thumbH,
        false);
    g.setGradientFill(thumbGrad);
    g.fillRoundedRectangle(tx, ty, thumbW, thumbH, 2.0f);

    // Thumb rim
    g.setColour(juce::Colour(0xff5a4a38).withAlpha(0.7f));
    g.drawRoundedRectangle(tx, ty, thumbW, thumbH, 2.0f, 0.8f);
}

void RRNegSliderLAF::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float, float, juce::Slider::SliderStyle, juce::Slider&)
{
    drawRndSlider(g, x, y, width, height, sliderPos, true);
}

void RRPosSliderLAF::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float, float, juce::Slider::SliderStyle, juce::Slider&)
{
    drawRndSlider(g, x, y, width, height, sliderPos, false);
}

// ── RRToggleLAF — vertical pill, SERIES top / RANDOM bottom ─────────────────
// RRLookAndFeel.cpp

void RRToggleLAF::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour&, bool, bool)
{
    const bool isRandom = button.getToggleState();
    const auto b = button.getLocalBounds().toFloat();

    // Horizontal layout: [SERIES]  [===pill===]  [RANDOM]
    constexpr float textW  = 44.0f;
    constexpr float pillW  = 34.0f;
    constexpr float pillH  = 16.0f;
    constexpr float pillGap = 6.0f;

    const float pillX = b.getCentreX() - pillW * 0.5f;
    const float pillY = b.getCentreY() - pillH * 0.5f;

    // SERIES label — left of pill. Active = inked dark, inactive = faded into warm panel.
    const juce::Colour activeInk   (0xff1f160d);
    const juce::Colour inactiveInk (0xff857e70);
    g.setFont(juce::Font(juce::FontOptions(9.0f)).boldened());
    g.setColour(isRandom ? inactiveInk : activeInk);
    g.drawText("SERIES", pillX - pillGap - textW, b.getY(), textW, b.getHeight(),
               juce::Justification::centredRight);

    // RANDOM label — right of pill.
    g.setColour(isRandom ? activeInk : inactiveInk);
    g.drawText("RANDOM", pillX + pillW + pillGap, b.getY(), textW, b.getHeight(),
               juce::Justification::centredLeft);

    // Pill track (recessed dark channel)
    g.setColour(juce::Colour(0xff07060a));
    g.fillRoundedRectangle(pillX, pillY, pillW, pillH, pillH * 0.5f);
    g.setColour(juce::Colour(0xff3a2e20));
    g.drawRoundedRectangle(pillX, pillY, pillW, pillH, pillH * 0.5f, 1.0f);

    // Cream thumb inside pill — left = SERIES, right = RANDOM
    const float knobD = pillH - 4.0f;
    const float knobY = pillY + 2.0f;
    const float knobX = isRandom
        ? pillX + pillW - 2.0f - knobD
        : pillX + 2.0f;

    // Thumb shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillEllipse(knobX + 0.5f, knobY + 1.0f, knobD, knobD);

    // Thumb body (cream gradient)
    juce::ColourGradient thumbGrad(
        RRColors::knobBody.brighter(0.15f), knobX, knobY,
        RRColors::knobBody.darker(0.2f),    knobX, knobY + knobD,
        false);
    g.setGradientFill(thumbGrad);
    g.fillEllipse(knobX, knobY, knobD, knobD);

    // Amber accent dot (active-state indicator)
    g.setColour(RRColors::amber);
    g.fillEllipse(knobX + knobD * 0.3f, knobY + knobD * 0.3f, knobD * 0.4f, knobD * 0.4f);
}

void RRToggleLAF::drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool)
{
    // Text drawn in drawButtonBackground — intentionally empty
}
