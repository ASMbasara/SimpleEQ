#pragma once
#include "SectionPanel.h"
#include "CustomRotarySlider.h"
#include "MinimalCombo.h"

class CutFilterSection : public SectionPanel
{
public:
    CutFilterSection(juce::AudioProcessorValueTreeState& apvts,
        const juce::String& freqParamId,
        const juce::String& slopeParamId,
        const juce::String& bypassParamId,
        const juce::StringArray& slopeLabels,
        const juce::String& title,
        juce::Colour accentColour);

protected:
    CustomRotarySlider freqSlider;
    MinimalCombo slopeCombo;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    SliderAttachment                    freqAttachment;
    std::unique_ptr<ComboBoxAttachment> slopeAttachment;

    void layoutControls(juce::Rectangle<int> area) override;
};