#pragma once
#include <JuceHeader.h>
#include "Looks.h"
#include "PluginProcessor.h"

class ResponseCurveComponent : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer
{
public:
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