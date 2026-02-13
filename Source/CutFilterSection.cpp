#include "CutFilterSection.h"

CutFilterSection::CutFilterSection(juce::AudioProcessorValueTreeState& apvts,
    const juce::String& freqParamId,
    const juce::String& slopeParamId,
    const juce::String& bypassParamId,
    const juce::StringArray& slopeLabels,
    const juce::String& title,
    juce::Colour accentColour)
    : SectionPanel(apvts, bypassParamId, title, accentColour),
    freqSlider(*apvts.getParameter(freqParamId), "Hz", "FREQ"),
    slopeCombo(slopeParamId, "dB/Oct", "SLOPE", accentColour),
    freqAttachment(apvts, freqParamId, freqSlider)
{
    for (int i = 0; i < slopeLabels.size(); ++i)
        slopeCombo.addItem(slopeLabels[i], i + 1);

    slopeAttachment = std::make_unique<ComboBoxAttachment>(apvts, slopeParamId, slopeCombo);

    addAndMakeVisible(freqSlider);
    addAndMakeVisible(slopeCombo);
}

void CutFilterSection::layoutControls(juce::Rectangle<int> area)
{
    static constexpr float GapRatio = 0.12f;
    static constexpr float SliderRatio = 0.75f;

    const int gap = static_cast<int>(area.getHeight() * GapRatio);
    area.removeFromTop(gap);
    area.removeFromBottom(gap);

    freqSlider.setBounds(area.removeFromTop(static_cast<int>(area.getHeight() * SliderRatio)));
    area.removeFromTop(3);
    slopeCombo.setBounds(area);
}