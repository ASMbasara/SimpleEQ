#pragma once
#include "SectionPanel.h"
#include "CustomRotarySlider.h"

class BandFilterSection : public SectionPanel
{
public:
    BandFilterSection(juce::AudioProcessorValueTreeState& apvts,
        const juce::String& freqParamId,
        const juce::String& gainParamId,
        const juce::String& qualityParamId,
        const juce::String& bypassParamId,
        const juce::String& title,
        juce::Colour accentColour);

protected:
    CustomRotarySlider freqSlider;
    CustomRotarySlider gainSlider;
    CustomRotarySlider qualitySlider;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    SliderAttachment freqAttachment;
    SliderAttachment gainAttachment;
    SliderAttachment qualityAttachment;

    void layoutControls(juce::Rectangle<int> area) override;
};