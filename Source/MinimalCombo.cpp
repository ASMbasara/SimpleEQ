#pragma once
#include <JuceHeader.h>
#include "MinimalCombo.h"

MinimalComboLookAndFeel::MinimalComboLookAndFeel()
{
    setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::textColourId, juce::Colour(200, 210, 225));
    setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::buttonColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::arrowColourId, juce::Colours::transparentBlack);

    // Popup menu colors
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(22, 25, 30));
    setColour(juce::PopupMenu::textColourId, juce::Colour(200, 210, 225));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(70, 150, 255).withAlpha(0.2f));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(255, 255, 255));
}

void MinimalComboLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box)
{
    using namespace juce;

    auto bounds = Rectangle<float>(0, 0, width, height);
    auto cornerSize = 3.0f;

    auto accentColor = Colours::aquamarine;

    g.setColour(Colour(25, 28, 32).withAlpha(0.3f));
    g.fillRoundedRectangle(bounds, cornerSize);

}

void MinimalComboLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText, const juce::Drawable* icon, const juce::Colour* textColour)
{
    using namespace juce;

    if (isSeparator)
    {
        g.setColour(Colour(50, 55, 65).withAlpha(0.3f));
        auto y = area.getCentreY();
        g.drawLine(area.getX() + 12.0f, y, area.getRight() - 12.0f, y, 1.0f);
        return;
    }

    auto textColourToUse = textColour != nullptr ? *textColour
        : findColour(PopupMenu::textColourId);

    // Minimal highlight
    if (isHighlighted && isActive)
    {
        g.setColour(Colour(70, 150, 255).withAlpha(0.12f));
        g.fillRoundedRectangle(area.reduced(6, 1).toFloat(), 2.0f);

        // Left accent
        g.setColour(Colour(100, 180, 255).withAlpha(0.5f));
        g.fillRoundedRectangle(area.reduced(8, 3).removeFromLeft(2).toFloat(), 1.0f);

        textColourToUse = Colour(255, 255, 255);
    }

    g.setColour(textColourToUse);
    g.setFont(Font("Exo 2", 11.5f, Font::plain));

    auto r = area.reduced(18, 0);

    // Small dot indicator
    if (isTicked)
    {
        g.setColour(Colour(100, 180, 255));
        g.fillEllipse(r.removeFromLeft(7).reduced(2).toFloat());
    }

    g.drawFittedText(text, r, Justification::centredLeft, 1);
}

juce::Font MinimalComboLookAndFeel::getComboBoxFont(juce::ComboBox& box)
{
    return juce::Font("Exo 2", 11.0f, juce::Font::plain);
}

void MinimalComboLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(0, 0, box.getWidth(), box.getHeight());
    label.setFont(getComboBoxFont(box));
    label.setJustificationType(juce::Justification::centred);
}

MinimalCombo::MinimalCombo(const juce::String& componentName, const juce::String& suffix, const juce::String& parameterLabel, juce::Colour accentColour)
    : juce::ComboBox(componentName),
    label(suffix),
    paramLabel(parameterLabel),
    accent(accentColour),
    defaultTextColour(200, 210, 225),
    vBlankCallback(this, [this] { animateColour(); })
{
    setLookAndFeel(&customLnF);
    addMouseListener(this, true);
}

MinimalCombo::~MinimalCombo()
{
    removeMouseListener(this);
    setLookAndFeel(nullptr);
}

void MinimalCombo::paint(juce::Graphics& g)
{
    using namespace juce;

    auto bounds = getLocalBounds();

    if (paramLabel.isNotEmpty())
    {
        g.setFont(Font("Exo 2", 8.5f, Font::plain));
        g.setColour(Colour(140, 150, 165).withAlpha(0.8f));
        auto labelArea = bounds.removeFromTop(11);
        g.drawText(paramLabel, labelArea, Justification::centredTop, true);
        bounds.removeFromTop(3);
        bounds.removeFromTop(JUCE_LIVE_CONSTANT(10));
    }

    ComboBox::paint(g);
}

void MinimalCombo::resized()
{
    auto bounds = getLocalBounds();

    if (paramLabel.isNotEmpty())
    {
        bounds.removeFromTop(11);
        bounds.removeFromTop(3);
        bounds.removeFromTop(10);
    }

    ComboBox::resized();
}

void MinimalCombo::setAccentColour(juce::Colour newAccent)
{
    accent = newAccent;
    repaint();
}

void MinimalCombo::mouseEnter(const juce::MouseEvent&)
{
    hovered = true;
}

void MinimalCombo::mouseExit(const juce::MouseEvent&)
{
    if (!isMouseOver(true))
        hovered = false;
}

void MinimalCombo::animateColour()
{
    const float speed = 0.075f;
    animProgress = hovered
        ? juce::jmin(1.0f, animProgress + speed)
        : juce::jmax(0.0f, animProgress - speed);

    auto blended = defaultTextColour.interpolatedWith(accent, animProgress);
    setColour(juce::ComboBox::textColourId, blended);
    repaint();
}
