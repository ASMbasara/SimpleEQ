/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once


#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RotarySliderWithLabels.h"

using APVTS = juce::AudioProcessorValueTreeState;
using Attachment = APVTS::SliderAttachment;

//====================COLOURS=====================



struct ResponseCurveComponent : juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer 
{
    ResponseCurveComponent(SimpleEQAudioProcessor&);
    ~ResponseCurveComponent();

    void parameterValueChanged(int parameterIndex, float newValue) override;

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }

    void timerCallback() override;

    void updateChain();

    void paint(juce::Graphics& g) override;
    void resized() override;

    template<int Index, typename Filter>
    inline void getMagForFreqSingle(double& mag, Filter& filter, const double freq);
    template<typename Filter>
    inline void getMagForFreq(double& mag, Filter& filter, const int nFilters, const double freq);
private:
    SimpleEQAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged{ false };
    MonoChain monoChain;

    juce::Image background;

    juce::Rectangle<int> getRenderArea();
    juce::Rectangle<int> getAnalysisArea();
};

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

    RotarySliderWithLabels bandFreqSlider, bandGainSlider, bandQualitySlider, lowCutFreqSlider, highCutFreqSlider, lowCutSlopeSlider, highCutSlopeSlider;

    Attachment bandFreqSliderAttachment,
               bandGainSliderAttachment,
               bandQualitySliderAttachment,
               lowCutFreqSliderAttachment,
               highCutFreqSliderAttachment,
               lowCutSlopeSliderAttachment,
               highCutSlopeSliderAttachment;

    
    std::vector<juce::Component*> getComps();



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};

template<int Index, typename Filter>
inline void ResponseCurveComponent::getMagForFreqSingle(double& mag, Filter& filter, const double freq)
{
    if (!filter.isBypassed<Index>())
        mag *= filter.get<Index>().coefficients->getMagnitudeForFrequency(freq, audioProcessor.getSampleRate());
}

template<typename Filter>
inline void ResponseCurveComponent::getMagForFreq(double& mag, Filter& filter, const int nFilters, const double freq)
{
    getMagForFreqSingle<0>(mag, filter, freq);
    getMagForFreqSingle<1>(mag, filter, freq);
    getMagForFreqSingle<2>(mag, filter, freq);
    getMagForFreqSingle<3>(mag, filter, freq);
}
