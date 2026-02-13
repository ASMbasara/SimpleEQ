
/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ResponseCurveComponent.h"
#include "BandFilterSection.h"
#include "CutFilterSection.h"

//==============================================================================
class SimpleEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SimpleEQAudioProcessor& audioProcessor;

    ResponseCurveComponent responseCurveComponent;

    CutFilterSection  lowCutSection;
    BandFilterSection band1Section;
    BandFilterSection band2Section;
    BandFilterSection band3Section;
    CutFilterSection  highCutSection;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleEQAudioProcessorEditor)
};


