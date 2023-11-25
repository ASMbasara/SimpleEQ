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

struct CustomRotarySlider : juce::Slider {
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox) {

    }
};

//==============================================================================
/**
*/
class SimpleEQAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::AudioProcessorParameter::Listener, juce::Timer
{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    /** Receives a callback when a parameter has been changed.

            IMPORTANT NOTE: This will be called synchronously when a parameter changes, and
            many audio processors will change their parameter during their audio callback.
            This means that not only has your handler code got to be completely thread-safe,
            but it's also got to be VERY fast, and avoid blocking. If you need to handle
            this event on your message thread, use this callback to trigger an AsyncUpdater
            or ChangeBroadcaster which you can respond to on the message thread.
        */
    void parameterValueChanged(int parameterIndex, float newValue) override;

    /** Indicates that a parameter change gesture has started.

        E.g. if the user is dragging a slider, this would be called with gestureIsStarting
        being true when they first press the mouse button, and it will be called again with
        gestureIsStarting being false when they release it.

        IMPORTANT NOTE: This will be called synchronously, and many audio processors will
        call it during their audio callback. This means that not only has your handler code
        got to be completely thread-safe, but it's also got to be VERY fast, and avoid
        blocking. If you need to handle this event on your message thread, use this callback
        to trigger an AsyncUpdater or ChangeBroadcaster which you can respond to later on the
        message thread.
    */
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }

    //==============================================================================
    /** The user-defined callback routine that actually gets called periodically.

        It's perfectly ok to call startTimer() or stopTimer() from within this
        callback to change the subsequent intervals.
    */
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;
    MonoChain monoChain;

    juce::Atomic<bool> parametersChanged{ false };

    CustomRotarySlider bandFreqSlider, bandGainSlider, bandQualitySlider, lowCutFreqSlider, highCutFreqSlider, lowCutSlopeSlider, highCutSlopeSlider;

    Attachment bandFreqSliderAttachment,
               bandGainSliderAttachment,
               bandQualitySliderAttachment,
               lowCutFreqSliderAttachment,
               highCutFreqSliderAttachment,
               lowCutSlopeSliderAttachment,
               highCutSlopeSliderAttachment;


    std::vector<juce::Component*> getComps();

    template<int Index, typename Filter>
    void getMagForFreqSingle(double &mag, Filter &filter, const double freq);
    template<typename Filter>
    void getMagForFreq(double& mag, Filter &filter, const int nFilters, const double freq);



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};

template<int Index, typename Filter>
inline void SimpleEQAudioProcessorEditor::getMagForFreqSingle(double &mag, Filter &filter, const double freq)
{
    if (!filter.isBypassed<Index>())
        mag *= filter.get<Index>().coefficients->getMagnitudeForFrequency(freq, audioProcessor.getSampleRate());
}

template<typename Filter>
inline void SimpleEQAudioProcessorEditor::getMagForFreq(double& mag, Filter &filter, const int nFilters, const double freq)
{
    
    getMagForFreqSingle<0>(mag, filter, freq);
    getMagForFreqSingle<1>(mag, filter, freq);
    getMagForFreqSingle<2>(mag, filter, freq);
    getMagForFreqSingle<3>(mag, filter, freq);
}
