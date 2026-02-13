#pragma once
#include <JuceHeader.h>
#include "PowerButton.h"
#include "Theme.h"

//==============================================================================
// Abstract base for section panels with a bypass button.
// Owns the shared chrome (card, accent bar, title, power/bypass button).
// Subclasses implement layoutControls() to position their specific controls.
//==============================================================================
class SectionPanel : public juce::Component
{
public:
    SectionPanel(juce::AudioProcessorValueTreeState& apvts,
        const juce::String& bypassParamId = "",
        const juce::String& title = "",
        juce::Colour accentColour = Theme::GenericAccent);

    void paint(juce::Graphics& g) final;
    void resized() final;

protected:
    virtual void layoutControls(juce::Rectangle<int> controlArea) = 0;

    juce::Colour getAccent() const;
    void setAccent(juce::Colour newAccent, bool bRepaint = false);

	juce::String getTitle() const;
	void setTitle(const juce::String& newTitle, bool bRepaint = false);
    
private:
    juce::String sTitle;
    juce::Colour accentColour;

    PowerButton powerButton;
    juce::AudioProcessorValueTreeState::ButtonAttachment bypassAttachment;

    static constexpr int Padding = 5;
    static constexpr int VPadding = 3;
    static constexpr int AccentBarHeight = 3;
    static constexpr int TitleRowHeight = 28;
    static constexpr int PowerBtnSize = 28;
};