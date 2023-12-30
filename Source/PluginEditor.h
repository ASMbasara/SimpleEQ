/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once


#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RotarySliderWithLabels.h"
#include "ResponseCurveComponent.h"
#include "CustomComboBox.h"

using APVTS = juce::AudioProcessorValueTreeState;
using Attachment = APVTS::SliderAttachment;
using ComboBoxAttachment = APVTS::ComboBoxAttachment;


//==============================================================================
/**
*/
class SimpleEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;

    ResponseCurveComponent responseCurveComponent;

    RotarySliderWithLabels band1FreqSlider;
    RotarySliderWithLabels band1GainSlider;
    RotarySliderWithLabels band1QualitySlider;
    RotarySliderWithLabels band2FreqSlider;
    RotarySliderWithLabels band2GainSlider;
    RotarySliderWithLabels band2QualitySlider;
    RotarySliderWithLabels band3FreqSlider;
    RotarySliderWithLabels band3GainSlider;
    RotarySliderWithLabels band3QualitySlider;
    RotarySliderWithLabels lowCutFreqSlider;
    RotarySliderWithLabels highCutFreqSlider;
    
    CustomComboBox lowCutSlopeCombo;
    CustomComboBox highCutSlopeCombo;

    Attachment band1FreqSliderAttachment;
    Attachment band1GainSliderAttachment;
    Attachment band1QualitySliderAttachment;
    Attachment band2FreqSliderAttachment;
    Attachment band2GainSliderAttachment;
    Attachment band2QualitySliderAttachment;
    Attachment band3FreqSliderAttachment;
    Attachment band3GainSliderAttachment;
    Attachment band3QualitySliderAttachment;
    Attachment lowCutFreqSliderAttachment;
    Attachment highCutFreqSliderAttachment;

    ComboBoxAttachment lowCutSlopeSliderAttachment;
    ComboBoxAttachment highCutSlopeSliderAttachment;
               

    
    std::vector<juce::Component*> getComps();



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};


