#include "RRLookAndFeel.h"

//==============================================================================
// RRLookAndFeel.cpp

void RRKnobLAF::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    const float cx = x + width  * 0.5f;
    const float cy = y + height * 0.5f;

    const float bodyRadius  = juce::jmin((float)width, (float)height) * 0.5f - 2.5f;
    const float trackRadius = bodyRadius * 0.92f;

    // ── Drop shadow ─────────────────────────────────────────────────────────
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillEllipse(cx - bodyRadius + 1.0f, cy - bodyRadius + 2.5f,
                  bodyRadius * 2.0f, bodyRadius * 2.0f);

    // ── Outer rim (subtle metallic ring) ────────────────────────────────────
    g.setColour(RRColors::knobRim);
    g.fillEllipse(cx - bodyRadius, cy - bodyRadius,
                  bodyRadius * 2.0f, bodyRadius * 2.0f);

    // ── Knob body — radial gradient for metallic depth ──────────────────────
    {
        const float inner = bodyRadius - 1.5f;
        juce::ColourGradient grad(
            juce::Colour(0xff2a2a2a), cx, cy - inner * 0.4f,   // lighter top
            juce::Colour(0xff111111), cx, cy + inner * 0.8f,    // darker bottom
            false);
        g.setGradientFill(grad);
        g.fillEllipse(cx - inner, cy - inner, inner * 2.0f, inner * 2.0f);
    }

    // ── Subtle top highlight (metallic sheen) ───────────────────────────────
    {
        const float hl = bodyRadius * 0.65f;
        juce::ColourGradient sheen(
            juce::Colours::white.withAlpha(0.06f), cx, cy - bodyRadius * 0.5f,
            juce::Colours::transparentBlack,       cx, cy + bodyRadius * 0.1f,
            false);
        g.setGradientFill(sheen);
        g.fillEllipse(cx - hl, cy - bodyRadius + 2.0f, hl * 2.0f, hl * 1.4f);
    }

    // ── Inner ring (dark edge around body) ──────────────────────────────────
    g.setColour(juce::Colour(0xff0a0a0a));
    g.drawEllipse(cx - bodyRadius + 1.5f, cy - bodyRadius + 1.5f,
                  (bodyRadius - 1.5f) * 2.0f, (bodyRadius - 1.5f) * 2.0f, 0.8f);

    // ── Track arc (recessed groove) ─────────────────────────────────────────
    juce::Path track;
    track.addArc(cx - trackRadius, cy - trackRadius,
                 trackRadius * 2.0f, trackRadius * 2.0f,
                 rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff1a1a1a));
    g.strokePath(track, juce::PathStrokeType(3.0f,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // ── Indicator line (from center to edge) ───────────────────────────────
    const juce::Colour lineCol = slider.findColour(juce::Slider::rotarySliderFillColourId);
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const float lineInner = bodyRadius * 0.20f;
    const float lineOuter = bodyRadius - 2.0f;    // extends to knob edge
    g.setColour(lineCol);
    g.drawLine(cx + std::sin(angle) * lineInner,
               cy - std::cos(angle) * lineInner,
               cx + std::sin(angle) * lineOuter,
               cy - std::cos(angle) * lineOuter,
               2.4f);
}

//==============================================================================
// RRButtonLAF — hardware-style buttons

void RRButtonLAF::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour& baseColour, bool isMouseOver, bool isButtonDown)
{
    auto b = button.getLocalBounds().toFloat().reduced(0.5f);
    auto col = baseColour;

    if (isButtonDown)
        col = col.brighter(0.15f);
    else if (isMouseOver)
        col = col.brighter(0.07f);

    // Recessed background
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(b.translated(0.0f, 1.0f), 3.0f);

    // Button body gradient
    juce::ColourGradient grad(col.brighter(0.1f), b.getX(), b.getY(),
                               col.darker(0.15f),  b.getX(), b.getBottom(),
                               false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(b, 3.0f);

    // Top highlight
    g.setColour(juce::Colours::white.withAlpha(isButtonDown ? 0.02f : 0.08f));
    g.fillRoundedRectangle(b.removeFromTop(b.getHeight() * 0.45f), 3.0f);

    // Border
    g.setColour(col.darker(0.4f));
    g.drawRoundedRectangle(button.getLocalBounds().toFloat().reduced(0.5f), 3.0f, 0.8f);
}

void RRButtonLAF::drawButtonText(juce::Graphics& g, juce::TextButton& button,
    bool isMouseOver, bool isButtonDown)
{
    g.setFont(juce::Font(juce::FontOptions(10.0f)).boldened());
    g.setColour(button.findColour(juce::TextButton::textColourOnId)
                    .withAlpha(button.isEnabled() ? 1.0f : 0.5f));
    g.drawText(button.getButtonText(), button.getLocalBounds(),
               juce::Justification::centred);
}

//==============================================================================
static void drawRndSlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, bool isNeg)
{
    const float trackY = y + height * 0.5f;
    const float trackH = 3.5f;
    const float thumbR = 6.0f;

    // Track background
    g.setColour(juce::Colour(38, 38, 48));
    g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f, (float)width, trackH, 2.0f);

    // Fill
    if (isNeg)
    {
        // Neg: fill from thumb rightward (shows how much negative range is set)
        g.setColour(juce::Colour(140, 35, 35));
        g.fillRoundedRectangle(sliderPos, trackY - trackH * 0.5f,
            (float)(x + width) - sliderPos, trackH, 2.0f);
    }
    else
    {
        // Pos: fill from left to thumb
        g.setColour(juce::Colour(35, 75, 165));
        g.fillRoundedRectangle((float)x, trackY - trackH * 0.5f,
            sliderPos - (float)x, trackH, 2.0f);
    }

    // Thumb ball
    const juce::Colour thumbCol = isNeg ? juce::Colour(210, 65, 65)
        : juce::Colour(65, 135, 235);
    g.setColour(thumbCol);
    g.fillEllipse(sliderPos - thumbR, trackY - thumbR, thumbR * 2.0f, thumbR * 2.0f);
    g.setColour(juce::Colours::white.withAlpha(0.35f));
    g.drawEllipse(sliderPos - thumbR, trackY - thumbR, thumbR * 2.0f, thumbR * 2.0f, 1.0f);
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

    // SERIES label — left of pill, bright when active
    g.setFont(juce::Font(juce::FontOptions(9.0f)).boldened());
    g.setColour(isRandom ? juce::Colour(0xff484848) : RRColors::algoCol.withAlpha(0.8f));
    g.drawText("SERIES", pillX - pillGap - textW, b.getY(), textW, b.getHeight(),
               juce::Justification::centredRight);

    // RANDOM label — right of pill, bright when active
    g.setColour(isRandom ? RRColors::algoCol.withAlpha(0.8f) : juce::Colour(0xff484848));
    g.drawText("RANDOM", pillX + pillW + pillGap, b.getY(), textW, b.getHeight(),
               juce::Justification::centredLeft);

    // Pill track
    g.setColour(juce::Colour(0xff0e1610));
    g.fillRoundedRectangle(pillX, pillY, pillW, pillH, pillH * 0.5f);
    g.setColour(RRColors::sectionBorder);
    g.drawRoundedRectangle(pillX, pillY, pillW, pillH, pillH * 0.5f, 1.0f);

    // Knob inside pill — left = SERIES, right = RANDOM
    const float knobD = pillH - 4.0f;
    const float knobY = pillY + 2.0f;
    const float knobX = isRandom
        ? pillX + pillW - 2.0f - knobD   // right
        : pillX + 2.0f;                  // left
    g.setColour(RRColors::algoCol);
    g.fillEllipse(knobX, knobY, knobD, knobD);
}

void RRToggleLAF::drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool)
{
    // Text drawn in drawButtonBackground — intentionally empty
}
