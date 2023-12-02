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

    RotarySliderWithLabels bandFreqSlider;
    RotarySliderWithLabels bandGainSlider;
    RotarySliderWithLabels bandQualitySlider;
    RotarySliderWithLabels lowCutFreqSlider;
    RotarySliderWithLabels highCutFreqSlider;
    
    CustomComboBox lowCutSlopeCombo;
    CustomComboBox highCutSlopeCombo;

    Attachment bandFreqSliderAttachment;
    Attachment bandGainSliderAttachment;
    Attachment bandQualitySliderAttachment;
    Attachment lowCutFreqSliderAttachment;
    Attachment highCutFreqSliderAttachment;

    ComboBoxAttachment lowCutSlopeSliderAttachment;
    ComboBoxAttachment highCutSlopeSliderAttachment;
               

    
    std::vector<juce::Component*> getComps();



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};


