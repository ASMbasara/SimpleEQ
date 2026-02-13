#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class MinimalComboLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MinimalComboLookAndFeel();
    
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;

    void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>& area,
        bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
        const juce::String& text, const juce::String& shortcutKeyText,
        const juce::Drawable* icon, const juce::Colour* textColour) override;
    
    juce::Font getComboBoxFont(juce::ComboBox& box) override;
    
    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;
};

class MinimalCombo : public juce::ComboBox
{
public:
    MinimalCombo(const juce::String& componentName, const juce::String& suffix, const juce::String& parameterLabel = "", juce::Colour accentColour = juce::Colour(100, 180, 255));
    ~MinimalCombo();

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setAccentColour(juce::Colour newAccent);
    juce::Colour getAccentColour() const { return accent; }

    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

protected:
    juce::String label;
    juce::String paramLabel;
    juce::Colour accent;
    juce::Colour defaultTextColour;
    MinimalComboLookAndFeel customLnF;

    bool hovered = false;
    float animProgress = 0.0f;
    juce::VBlankAttachment vBlankCallback;

    void animateColour();
};