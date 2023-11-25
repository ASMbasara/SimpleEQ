/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>



enum ChainPositions {
    LowCut,
    Band,
    HighCut
};

enum Slope {
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

struct ChainSettings {
    float bandFreq = 0;
    float bandGain = 0;
    float bandQ = 0.707;
    float lowCutFreq = 0;
    Slope lowCutSlope = Slope_12;
    float highCutFreq = 0;
    Slope highCutSlope = Slope_12;
};


//==============================================================================
/**
*/
class SimpleEQAudioProcessor  : public juce::AudioProcessor
{
public:
    // To coordinate GUI and DSP variables
    
    float minFreq = 20.f;
    float maxFreq = 20000.f;
    float midFreq = sqrt(minFreq * maxFreq);
    float freqSkewFactor = log(0.5) / log((midFreq - minFreq) / (maxFreq - minFreq)); //source: https://jucestepbystep.wordpress.com/logarithmic-sliders/
    float linSkewFactor = 1.0f;

    juce::AudioProcessorValueTreeState treeState{ *this, nullptr, "PARAMETERS", createParameterLayout() };
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& treeState);

    //==============================================================================

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

    MonoChain leftChain, rightChain;

    void updateBandCoefficients(ChainSettings chainSettings);
    void updateLowCutCoefficients(ChainSettings chainSettings);
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleEQAudioProcessor)
        
};
