#include "RotarySliderWithLabels.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    g.setColour(midBlue);
    g.fillEllipse(bounds);

    g.setColour(darkBlue);
    g.drawEllipse(bounds, 5.f);


    if (RotarySliderWithLabels* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)) {
        auto center = bounds.getCentre();

        Path p;


        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY() + bounds.getWidth() * 0.05);
        r.setBottom(center.getY() - rswl->getTextHeight() * 2.5);

        p.addRoundedRectangle(r.toFloat(), 4.f, 1.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPos, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.setColour(Colour(255u, 255u, 255u));
        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        g.setColour(Colour(255u, 255u, 255u));
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);

    }

}


RotarySliderWithLabels::RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap),
    suffix(unitSuffix)
{
    setLookAndFeel(&lnf);
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    float startAng = degreesToRadians(180.f + 45.f);
    float endAng = startAng - MathConstants<double>::halfPi + MathConstants<double>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    /*g.setColour(Colour(0u, 19u, 17u));
    g.drawRect(getLocalBounds());
    g.setColour(Colour(41u, 23u, 19u));
    g.drawRect(getSliderBounds());*/

    getLookAndFeel().drawRotarySlider(g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        param->convertTo0to1(getValue()),
        startAng,
        endAng,
        *this);

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

    g.setColour(lightBlue);
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; i++) {
        auto pos = labels[i].pos;
        bool above = pos > 0.25f && pos < 0.75f;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);

        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

        auto c = center.getPointOnCircumference(radius + getTextHeight() * 1, ang);

        Rectangle<float> r;
        auto text = labels[i].label;
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        r.setSize(strWidth + 4, getTextHeight() + 2);
        r.setCentre(c);
        above ? r.setY(r.getY() - getTextHeight() * 0.33) : r.setY(r.getY() + getTextHeight() * 0.33);


        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
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

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), bounds.getCentreY());
    r.setY(2);

    return r;
}