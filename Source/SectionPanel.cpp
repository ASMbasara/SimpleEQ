#include "SectionPanel.h"

SectionPanel::SectionPanel(juce::AudioProcessorValueTreeState& apvts,
    const juce::String& bypassParamId,
    const juce::String& title,
    juce::Colour accent)
    : sTitle(title),
    accentColour(accent),
    bypassAttachment(apvts, bypassParamId, powerButton)
{
    powerButton.setAccentColour(accentColour);
    addAndMakeVisible(powerButton);
}

void SectionPanel::paint(juce::Graphics& g)
{
    using namespace juce;

    auto bounds = getLocalBounds().reduced(Padding, VPadding).toFloat();

    // Background
    g.setColour(Theme::Card);
    g.fillRoundedRectangle(bounds, 6.0f);

    // Accent bar
    auto accentBar = bounds.removeFromTop(static_cast<float>(AccentBarHeight));
    g.setColour(accentColour);
    g.fillRoundedRectangle(accentBar, 6.0f);

    // Border
    g.setColour(Theme::CardBorder.withAlpha(0.6f));
    g.drawRoundedRectangle(getLocalBounds().reduced(Padding, VPadding).toFloat(), 6.0f, 1.0f);

    // Title text
    auto titleArea = getLocalBounds().reduced(Padding, VPadding);
    titleArea.removeFromTop(AccentBarHeight);
    titleArea = titleArea.removeFromTop(TitleRowHeight);
    titleArea.removeFromLeft(PowerBtnSize);
    titleArea.removeFromRight(PowerBtnSize);

    g.setColour(accentColour.brighter(0.3f));
    g.setFont(Font(Theme::FontFamily, 11.0f, Font::bold));
    g.drawText(sTitle, titleArea, Justification::centred, true);
}

void SectionPanel::resized()
{
    auto bounds = getLocalBounds().reduced(Padding, VPadding);

    bounds.removeFromTop(AccentBarHeight);
    auto titleRow = bounds.removeFromTop(TitleRowHeight);
    powerButton.setBounds(titleRow.removeFromLeft(PowerBtnSize).reduced(4));

    layoutControls(bounds);
}

juce::Colour SectionPanel::getAccent() const
{
    return accentColour;
}

void SectionPanel::setAccent(juce::Colour newAccent, bool bRepaint)
{
    accentColour = newAccent;
    powerButton.setAccentColour(newAccent);

    if(bRepaint)
        repaint();
}

juce::String SectionPanel::getTitle() const
{
    return sTitle;
}

void SectionPanel::setTitle(const juce::String& newTitle, bool bRepaint)
{
    sTitle = newTitle;

    if(bRepaint)
	    repaint();
}
