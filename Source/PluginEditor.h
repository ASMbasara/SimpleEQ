/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

using APVTS = juce::AudioProcessorValueTreeState;
using Attachment = APVTS::SliderAttachment;




struct LookAndFeel : juce::LookAndFeel_V4 {

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
    

};

struct RotarySliderWithLabels : juce::Slider {
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) :
        juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(&rap),
        suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }

    ~RotarySliderWithLabels() {
        setLookAndFeel(nullptr);
    }

    struct LabelPos {
        float pos;
        juce::String label;
    };

    juce::Array<LabelPos> labels;
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;
    juce::Rectangle<int> getSliderBounds();
private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};

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
