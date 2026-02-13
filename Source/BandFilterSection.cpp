#include "BandFilterSection.h"

BandFilterSection::BandFilterSection(juce::AudioProcessorValueTreeState& apvts,
    const juce::String& freqParamId,
    const juce::String& gainParamId,
    const juce::String& qualityParamId,
    const juce::String& bypassParamId,
    const juce::String& title,
    juce::Colour accentColour)
    : SectionPanel(apvts, bypassParamId, title, accentColour),
    freqSlider(*apvts.getParameter(freqParamId), "Hz", "FREQ"),
    gainSlider(*apvts.getParameter(gainParamId), "dB", "GAIN"),
    qualitySlider(*apvts.getParameter(qualityParamId), "", "Q"),
    freqAttachment(apvts, freqParamId, freqSlider),
    gainAttachment(apvts, gainParamId, gainSlider),
    qualityAttachment(apvts, qualityParamId, qualitySlider)
{
    addAndMakeVisible(freqSlider);
    addAndMakeVisible(gainSlider);
    addAndMakeVisible(qualitySlider);
}

void BandFilterSection::layoutControls(juce::Rectangle<int> area)
{
    static constexpr float GapRatio = 0.08f;
    static constexpr float FreqRatio = 0.33f;
    static constexpr float GainRatio = 0.50f;

    area.removeFromTop(static_cast<int>(area.getHeight() * GapRatio));

    freqSlider.setBounds(area.removeFromTop(static_cast<int>(area.getHeight() * FreqRatio)));
    gainSlider.setBounds(area.removeFromTop(static_cast<int>(area.getHeight() * GainRatio)));
    qualitySlider.setBounds(area);
}