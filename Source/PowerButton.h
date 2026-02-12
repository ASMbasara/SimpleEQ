#pragma once
#include <JuceHeader.h>

class PowerButton : public juce::ToggleButton
{
public:
    PowerButton();
    ~PowerButton() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override {}

    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

    void setAccentColour(juce::Colour newAccent);
    juce::Colour getAccentColour() const { return accentColour; }

private:
    juce::Colour accentColour{ 100, 180, 255 };
    bool isHovered{ false };
    float animationProgress{ 0.0f };

    juce::VBlankAttachment vBlankCallback;
    void animate();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PowerButton)
};
