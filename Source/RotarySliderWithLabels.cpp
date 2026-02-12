#include "RotarySliderWithLabels.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);
    auto center = bounds.getCentre();
    auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;

    // Modern track arc (background)
    Path trackArc;
    trackArc.addCentredArc(
        center.x, center.y,
        radius * 0.85f, radius * 0.85f,
        0.0f,
        rotaryStartAngle, rotaryEndAngle,
        true
    );

    // Subtle track background
    g.setColour(Colour(30, 35, 42).withAlpha(0.8f));
    g.strokePath(trackArc, PathStrokeType(3.5f, PathStrokeType::curved, PathStrokeType::rounded));

    // Value arc (filled portion)
    if (sliderPos > 0.0f)
    {
        Path valueArc;
        auto currentAngle = jmap(sliderPos, 0.0f, 1.0f, rotaryStartAngle, rotaryEndAngle);
        valueArc.addCentredArc(
            center.x, center.y,
            radius * 0.85f, radius * 0.85f,
            0.0f,
            rotaryStartAngle, currentAngle,
            true
        );

        // Gradient stroke for modern look
        ColourGradient gradient(
            Colour(70, 150, 255),
            center.x - radius, center.y,
            Colour(100, 200, 255),
            center.x + radius, center.y,
            false
        );
        g.setGradientFill(gradient);
        g.strokePath(valueArc, PathStrokeType(4.0f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // Modern knob body (larger)
    auto knobRadius = radius * 0.68f;

    // Outer shadow/depth
    g.setColour(Colours::black.withAlpha(0.3f));
    g.fillEllipse(center.x - knobRadius - 1, center.y - knobRadius - 1,
        knobRadius * 2 + 2, knobRadius * 2 + 2);

    // Knob gradient fill
    ColourGradient knobGradient(
        Colour(35, 38, 45),
        center.x, center.y - knobRadius,
        Colour(25, 28, 35),
        center.x, center.y + knobRadius,
        false
    );
    g.setGradientFill(knobGradient);
    g.fillEllipse(center.x - knobRadius, center.y - knobRadius, knobRadius * 2, knobRadius * 2);

    // Subtle inner highlight
    g.setColour(Colour(50, 55, 65).withAlpha(0.3f));
    g.fillEllipse(center.x - knobRadius * 0.92f, center.y - knobRadius * 0.92f,
        knobRadius * 1.84f, knobRadius * 1.84f);

    // Outer border
    g.setColour(Colour(45, 50, 60).withAlpha(0.6f));
    g.drawEllipse(center.x - knobRadius, center.y - knobRadius,
        knobRadius * 2, knobRadius * 2, 1.2f);

    if (RotarySliderWithLabels* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)) {
        // Modern pointer/indicator
        auto sliderAngRad = jmap(sliderPos, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        Path pointer;
        auto pointerLength = knobRadius * 0.65f;
        auto pointerThickness = 2.5f;

        pointer.addRoundedRectangle(
            -pointerThickness * 0.5f, -pointerLength,
            pointerThickness, pointerLength * 0.75f,
            pointerThickness * 0.5f
        );

        pointer.applyTransform(AffineTransform::rotation(sliderAngRad).translated(center.x, center.y));

        // Pointer glow
        g.setColour(Colour(100, 200, 255).withAlpha(0.4f));
        g.fillPath(pointer);

        // Main pointer
        g.setColour(Colour(150, 220, 255));
        g.fillPath(pointer);

        // Value display with modern styling
        g.setFont(Font("Exo 2", rswl->getTextHeight() * 0.85f, Font::bold));
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        Rectangle<float> textRect;
        textRect.setSize(strWidth + 8, rswl->getTextHeight());
        textRect.setCentre(center.x, center.y + knobRadius + rswl->getTextHeight() * 0.8f);

        // Subtle background for value
        g.setColour(Colour(25, 28, 35).withAlpha(0.7f));
        g.fillRoundedRectangle(textRect, 3.0f);

        // Value text
        g.setColour(Colour(200, 210, 225));
        g.drawFittedText(text, textRect.toNearestInt(), Justification::centred, 1);
    }
}


RotarySliderWithLabels::RotarySliderWithLabels(juce::RangedAudioParameter& rap,
    const juce::String& unitSuffix,
    const juce::String& parameterLabel) :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap),
    suffix(unitSuffix),
    paramLabel(parameterLabel)
{
    setLookAndFeel(&lnf);
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto bounds = getLocalBounds();

    // Draw parameter label at the top with minimal spacing
    if (paramLabel.isNotEmpty())
    {
        g.setFont(Font("Exo 2", 8.5f, Font::plain));
        g.setColour(Colour(140, 150, 165).withAlpha(0.8f));
        auto labelArea = bounds.removeFromTop(11);
        g.drawText(paramLabel, labelArea, Justification::centredTop, true);
        bounds.removeFromTop(3); // Reduced from 6 to 3 for tighter spacing
    }

    float startAng = degreesToRadians(180.f + 45.f);
    float endAng = startAng - MathConstants<double>::halfPi + MathConstants<double>::twoPi;

    auto range = getRange();
    auto sliderBounds = getSliderBounds();

    // Draw the rotary slider
    getLookAndFeel().drawRotarySlider(g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        param->convertTo0to1(getValue()),
        startAng,
        endAng,
        *this);
}

void RotarySliderWithLabels::resized()
{
    // Component handles its own layout including label space
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (juce::AudioParameterChoice* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param)) {
        juce::String val = choiceParam->getCurrentChoiceName();
        if (choiceParam->getParameterID().containsWholeWord("Slope")) {
            val.append(" ", 1);
            val.append(suffix, suffix.length());
        }
        return val;
    }

    juce::String str;
    bool addK = false;

    if (juce::AudioParameterFloat* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param)) {
        float value = getValue();

        if (value >= 1000.f) {
            value /= 1000.f;
            addK = true;
        }

        if (getName().contains("Slope"))
            value = 12 + value * 12;
        str = juce::String(value, (addK ? 2 : 0));
    }
    else {
        jassertfalse;
    }

    if (suffix.isNotEmpty()) {
        str << " ";
        if (addK)
            str << "k";
        str << suffix;
    }

    return str;
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds()
{
    auto bounds = getLocalBounds();

    // Account for label at top with tighter spacing
    if (paramLabel.isNotEmpty())
    {
        bounds.removeFromTop(11); // Label height
        bounds.removeFromTop(3);  // Reduced padding from 6 to 3
    }

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    size -= getTextHeight() * 1.8;

    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(JUCE_LIVE_CONSTANT(10));

    return r;
}