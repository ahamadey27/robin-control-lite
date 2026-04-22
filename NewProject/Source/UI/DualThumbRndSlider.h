#pragma once
#include <JuceHeader.h>

class DualThumbRndSlider : public juce::Component,
                           private juce::Slider::Listener
{
public:
    DualThumbRndSlider(juce::Slider& negSlider, juce::Slider& posSlider, juce::Colour colour)
        : neg(negSlider), pos(posSlider), col(colour)
    {
        neg.addListener(this);
        pos.addListener(this);
    }

    ~DualThumbRndSlider() override
    {
        neg.removeListener(this);
        pos.removeListener(this);
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced(1.0f, 2.0f);
        float cx = b.getCentreX();
        float halfW = b.getWidth() * 0.5f;

        float negNorm = getNorm(neg);
        float posNorm = getNorm(pos);

        constexpr float thumbW = 6.0f;
        constexpr float gap = 4.0f; // visible gap between thumbs at 0%

        float negX = cx - gap - negNorm * (halfW - gap - thumbW * 0.5f);
        float posX = cx + gap + posNorm * (halfW - gap - thumbW * 0.5f);

        // Track background
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(b, 3.0f);

        // Filled range (from neg thumb to pos thumb)
        if (negNorm > 0.01f || posNorm > 0.01f)
        {
            g.setColour(col.withAlpha(0.25f));
            g.fillRoundedRectangle(negX, b.getY(), posX - negX, b.getHeight(), 2.0f);
        }

        // Neg thumb
        g.setColour(col);
        g.fillRoundedRectangle(negX - thumbW * 0.5f, b.getY(), thumbW, b.getHeight(), 2.0f);

        // Pos thumb
        g.fillRoundedRectangle(posX - thumbW * 0.5f, b.getY(), thumbW, b.getHeight(), 2.0f);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        float cx = getWidth() * 0.5f;
        float x = e.position.x;

        // Determine which thumb is closer
        float halfW = getWidth() * 0.5f;
        constexpr float gap = 4.0f;
        constexpr float thumbW = 6.0f;

        float negX = cx - gap - getNorm(neg) * (halfW - gap - thumbW * 0.5f);
        float posX = cx + gap + getNorm(pos) * (halfW - gap - thumbW * 0.5f);

        float negDist = std::abs(x - negX);
        float posDist = std::abs(x - posX);

        if (negDist < posDist)
            activeThumb = Thumb::neg;
        else
            activeThumb = Thumb::pos;

        dragStartX = x;
        dragStartVal = (activeThumb == Thumb::neg) ? getNorm(neg) : getNorm(pos);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (activeThumb == Thumb::none) return;

        float halfW = getWidth() * 0.5f;
        constexpr float gap = 4.0f;
        constexpr float thumbW = 6.0f;
        float range = halfW - gap - thumbW * 0.5f;
        if (range < 1.0f) return;

        float dx = e.position.x - dragStartX;

        float newNorm;
        if (activeThumb == Thumb::neg)
        {
            newNorm = dragStartVal - dx / range; // dragging left increases neg
            newNorm = juce::jlimit(0.0f, 1.0f, newNorm);
            neg.setValue(neg.getMinimum() + newNorm * (neg.getMaximum() - neg.getMinimum()),
                         juce::sendNotificationAsync);
        }
        else
        {
            newNorm = dragStartVal + dx / range; // dragging right increases pos
            newNorm = juce::jlimit(0.0f, 1.0f, newNorm);
            pos.setValue(pos.getMinimum() + newNorm * (pos.getMaximum() - pos.getMinimum()),
                         juce::sendNotificationAsync);
        }
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        activeThumb = Thumb::none;
    }

    void mouseDoubleClick(const juce::MouseEvent&) override
    {
        neg.setValue(neg.getMinimum(), juce::sendNotificationAsync);
        pos.setValue(pos.getMinimum(), juce::sendNotificationAsync);
    }

private:
    juce::Slider& neg;
    juce::Slider& pos;
    juce::Colour col;

    enum class Thumb { none, neg, pos };
    Thumb activeThumb = Thumb::none;
    float dragStartX = 0.0f;
    float dragStartVal = 0.0f;

    static float getNorm(juce::Slider& s)
    {
        double range = s.getMaximum() - s.getMinimum();
        if (range == 0.0) return 0.0f;
        return (float)((s.getValue() - s.getMinimum()) / range);
    }

    void sliderValueChanged(juce::Slider*) override
    {
        repaint();
        // Also repaint parent for arc outline update
        if (auto* p = getParentComponent())
            p->repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualThumbRndSlider)
};
