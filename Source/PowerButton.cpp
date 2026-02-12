#include "PowerButton.h"

PowerButton::PowerButton()
    : vBlankCallback(this, [this] { animate(); })
{
    setClickingTogglesState(true);
    setToggleState(true, juce::dontSendNotification);  // Default: ON (not bypassed)
}

void PowerButton::paint(juce::Graphics& g)
{
    using namespace juce;

    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    auto center = bounds.getCentre();
    auto size = jmin(bounds.getWidth(), bounds.getHeight());
    auto radius = size * 0.5f;

    // Determine colors based on toggle state
    bool isOn = getToggleState();

    // Background glow when ON
    if (isOn)
    {
        float bgAlpha = 0.05f + animationProgress * 0.03f;
        g.setColour(accentColour.withAlpha(bgAlpha));
        g.fillEllipse(center.x - radius, center.y - radius, size, size);
    }

    // Outer ring
    float ringThickness = 1.5f;
    Colour ringColour = isOn
        ? accentColour.withAlpha(0.4f).interpolatedWith(
            accentColour.withAlpha(0.6f), animationProgress)
        : Colour(60, 65, 75).interpolatedWith(Colour(80, 85, 95), animationProgress);

    g.setColour(ringColour);
    g.drawEllipse(center.x - radius + ringThickness,
        center.y - radius + ringThickness,
        size - ringThickness * 2,
        size - ringThickness * 2,
        ringThickness);

    // Power symbol - classic IEC 5009 broken circle with gap at TOP
    auto symbolRadius = radius * 0.5f;
    auto symbolThickness = 2.2f;

    // In JUCE, addCentredArc uses 0 = top (12 o'clock), positive = clockwise.
    // 60° gap centered at 0 (top).
    constexpr float gapAngle = MathConstants<float>::pi / 3.0f;                        // 60° total gap
    constexpr float arcStartAngle = gapAngle * 0.5f;                                   // right edge of gap
    constexpr float arcEndAngle = MathConstants<float>::twoPi - gapAngle * 0.5f;       // left edge of gap

    Path powerArc;
    powerArc.addCentredArc(center.x, center.y,
        symbolRadius, symbolRadius,
        0.0f,
        arcStartAngle, arcEndAngle,
        true);

    // Vertical line pointing UP through the gap
    Path powerLine;
    powerLine.startNewSubPath(center.x, center.y);                    // Center
    powerLine.lineTo(center.x, center.y - symbolRadius * 1.15f);     // Up through gap

    // Color and draw the symbol
    Colour symbolColour = isOn
        ? accentColour.withAlpha(JUCE_LIVE_CONSTANT(0.7f)).interpolatedWith(
            accentColour.withAlpha(JUCE_LIVE_CONSTANT(0.85f)), animationProgress)
        : Colour(90, 95, 105).interpolatedWith(Colour(120, 125, 135), animationProgress);

    g.setColour(symbolColour);
    g.strokePath(powerArc, PathStrokeType(symbolThickness, PathStrokeType::curved, PathStrokeType::rounded));
    g.strokePath(powerLine, PathStrokeType(symbolThickness, PathStrokeType::curved, PathStrokeType::rounded));

    // Inner glow when ON
    if (isOn)
    {
        float glowAlpha = 0.02f + animationProgress * 0.02f;
        g.setColour(accentColour.withAlpha(glowAlpha));
        auto glowRadius = symbolRadius * 1.3f;
        g.fillEllipse(center.x - glowRadius, center.y - glowRadius, glowRadius * 2, glowRadius * 2);
    }
}

void PowerButton::mouseEnter(const juce::MouseEvent&)
{
    isHovered = true;
}

void PowerButton::mouseExit(const juce::MouseEvent&)
{
    isHovered = false;
}

void PowerButton::animate()
{
    if (isHovered && animationProgress < 1.0f)
    {
        animationProgress = juce::jmin(1.0f, animationProgress + 0.1f);
        repaint();
    }
    else if (!isHovered && animationProgress > 0.0f)
    {
        animationProgress = juce::jmax(0.0f, animationProgress - 0.1f);
        repaint();
    }
}

void PowerButton::setAccentColour(juce::Colour newAccent)
{
    accentColour = newAccent;
    repaint();
}