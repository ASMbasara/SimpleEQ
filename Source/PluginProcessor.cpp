/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

Coefficients makeBandFilter(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.bandFreq, chainSettings.bandQ, juce::Decibels::decibelsToGain(chainSettings.bandGain));

}auto makeLowCutFilter(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, sampleRate, 2 * (chainSettings.lowCutSlope + 1));

}auto makeHighCutFilter(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, sampleRate, 2 * (chainSettings.highCutSlope + 1));
}

//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.numChannels = 1;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    updateFilters();
}

void SimpleEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateFilters();

    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool SimpleEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
    return new SimpleEQAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    //stores the params value in order to start it in next execution
    juce::MemoryOutputStream mos(destData, true);
    treeState.state.writeToStream(mos);
}

void SimpleEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    //restores last execution params values
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        treeState.replaceState(tree);
        updateFilters();
    }
}

ChainSettings SimpleEQAudioProcessor::getChainSettings(juce::AudioProcessorValueTreeState& treeState) {
    ChainSettings settings;

    settings.lowCutFreq = treeState.getRawParameterValue("LowCut Frequency")->load();
    settings.lowCutSlope = static_cast<Slope>(treeState.getRawParameterValue("LowCut Slope")->load());
    settings.highCutFreq = treeState.getRawParameterValue("HighCut Frequency")->load();
    settings.highCutSlope = static_cast<Slope>(treeState.getRawParameterValue("HighCut Slope")->load());
    settings.bandFreq = treeState.getRawParameterValue("Band Frequency")->load();
    settings.bandGain = treeState.getRawParameterValue("Band Gain")->load();
    settings.bandQ= treeState.getRawParameterValue("Band Quality")->load();

    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Frequency", "LowCut Frequency", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, freqSkewFactor), 20.f, "Hz"));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Frequency", "HighCut Frequency", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, freqSkewFactor), 20000.f, "Hz"));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Band Frequency", "Band Frequency", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, freqSkewFactor), 1000.f, "Hz"));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Band Gain", "Band Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, linSkewFactor), 0.f, "dB"));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Band Quality", "Band Quality", juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, linSkewFactor), 0.707f));

    juce::StringArray dbPerOctave;
    for (int i = 0; i < 4; i++) 
        dbPerOctave.add(std::to_string(12 + i * 12));

    
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", dbPerOctave, 0, "dB/Oct"));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", dbPerOctave, 0, "dB/Oct"));


    return layout;
}

void SimpleEQAudioProcessor::updateBandCoefficients(ChainSettings chainSettings)
{
    auto peakCoefficients = makeBandFilter(chainSettings, getSampleRate());

    *leftChain.get<Band>().coefficients = *peakCoefficients;
    *rightChain.get<Band>().coefficients = *peakCoefficients;
}

void SimpleEQAudioProcessor::updateFilters()
{
    auto chainSettings = getChainSettings(treeState);

    updateLowFilters(chainSettings);
    updateBandFilters(chainSettings);
    updateHighFilters(chainSettings);
}

void SimpleEQAudioProcessor::updateLowFilters(ChainSettings chainSettings)
{
    auto lowCutCoefficients = makeLowCutFilter(chainSettings, getSampleRate());

    auto& leftLowCut = leftChain.get<LowCut>();
    auto& rightLowCut = rightChain.get<LowCut>();

    updateCutCoefficients(leftLowCut, lowCutCoefficients, chainSettings.lowCutSlope);
    updateCutCoefficients(rightLowCut, lowCutCoefficients, chainSettings.lowCutSlope);
}



void SimpleEQAudioProcessor::updateBandFilters(ChainSettings chainSettings)
{
    updateBandCoefficients(chainSettings);
}

void SimpleEQAudioProcessor::updateHighFilters(ChainSettings chainSettings)
{
    auto highCutCoefficients = makeHighCutFilter(chainSettings, getSampleRate());

    auto& leftHighCut = leftChain.get<HighCut>();
    auto& rightHighCut = rightChain.get<HighCut>();

    updateCutCoefficients(leftHighCut, highCutCoefficients, chainSettings.highCutSlope);
    updateCutCoefficients(rightHighCut, highCutCoefficients, chainSettings.highCutSlope);
}

template<int Index, typename ChainType, typename CoefficientType>
void update(ChainType& leftLowCut, const CoefficientType& coefficients) {
    *leftLowCut.get<Index>().coefficients = *coefficients[Index];
    leftLowCut.setBypassed<Index>(false);
}

template<typename ChainType, typename CoefficientType>
void SimpleEQAudioProcessor::updateCutCoefficients(ChainType& leftLowCut, const CoefficientType& lowCutCoefficients, const Slope& lowCutSlope)
{

    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);

    switch (lowCutSlope) {
    case Slope_48:
        update<3>(leftLowCut, lowCutCoefficients);
    case Slope_36:
        update<2>(leftLowCut, lowCutCoefficients);
    case Slope_24:
        update<1>(leftLowCut, lowCutCoefficients);
    case Slope_12:
        update<0>(leftLowCut, lowCutCoefficients);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
    
}
