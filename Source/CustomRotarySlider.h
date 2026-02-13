#pragma once
#include <JuceHeader.h>

class CustomRotarySliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
};

class CustomRotarySlider : public juce::Slider
{
public:
    CustomRotarySlider(juce::RangedAudioParameter& rap, const juce::String& unitSuffix, const juce::String& parameterLabel = "");
    ~CustomRotarySlider();
    
    void paint(juce::Graphics& g) override;

    int getTextHeight() const { return 14; }

    juce::String getDisplayString() const;
    juce::Rectangle<int> getSliderBounds();

    void setParameterLabel(const juce::String& label);

protected:
    CustomRotarySliderLookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
    juce::String paramLabel;

    struct LabelPos {
        float pos;
        juce::String label;
    };
    juce::Array<LabelPos> labels;
};