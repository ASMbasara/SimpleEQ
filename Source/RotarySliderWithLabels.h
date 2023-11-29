#pragma once
#include <JuceHeader.h>
#include "Looks.h"

struct LookAndFeel : public juce::LookAndFeel_V4
{

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
};

class RotarySliderWithLabels : public juce::Slider
{
public:
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix);

    ~RotarySliderWithLabels() {
        setLookAndFeel(nullptr);
    }

    struct LabelPos {
        float pos;
        juce::String label;
    };

    juce::Array<LabelPos> labels;
    void paint(juce::Graphics& g) override;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;
    juce::Rectangle<int> getSliderBounds();
private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};