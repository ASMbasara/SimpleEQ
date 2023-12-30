/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"







//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    :
    AudioProcessorEditor (&p),
    audioProcessor (p),
    band1FreqSlider(*audioProcessor.treeState.getParameter("Band1 Frequency"), "Hz"),
    band1GainSlider(*audioProcessor.treeState.getParameter("Band1 Gain"), "dB"),
    band1QualitySlider(*audioProcessor.treeState.getParameter("Band1 Quality"), ""),
    band2FreqSlider(*audioProcessor.treeState.getParameter("Band2 Frequency"), "Hz"),
    band2GainSlider(*audioProcessor.treeState.getParameter("Band2 Gain"), "dB"),
    band2QualitySlider(*audioProcessor.treeState.getParameter("Band2 Quality"), ""),
    band3FreqSlider(*audioProcessor.treeState.getParameter("Band3 Frequency"), "Hz"),
    band3GainSlider(*audioProcessor.treeState.getParameter("Band3 Gain"), "dB"),
    band3QualitySlider(*audioProcessor.treeState.getParameter("Band3 Quality"), ""),
    lowCutFreqSlider(*audioProcessor.treeState.getParameter("LowCut Frequency"), "Hz"),
    highCutFreqSlider(*audioProcessor.treeState.getParameter("HighCut Frequency"), "Hz"),
    lowCutSlopeCombo(juce::String("LowCut Slope"), juce::String("dB/Oct")),
    highCutSlopeCombo(juce::String("HighCut Slope"), juce::String("dB/Oct")),
    responseCurveComponent(audioProcessor),
    band1FreqSliderAttachment(audioProcessor.treeState, "Band1 Frequency", band1FreqSlider),
    band1GainSliderAttachment(audioProcessor.treeState, "Band1 Gain", band1GainSlider),
    band1QualitySliderAttachment(audioProcessor.treeState, "Band1 Quality", band1QualitySlider),
    band2FreqSliderAttachment(audioProcessor.treeState, "Band2 Frequency", band2FreqSlider),
    band2GainSliderAttachment(audioProcessor.treeState, "Band2 Gain", band2GainSlider),
    band2QualitySliderAttachment(audioProcessor.treeState, "Band2 Quality", band2QualitySlider),
    band3FreqSliderAttachment(audioProcessor.treeState, "Band3 Frequency", band3FreqSlider),
    band3GainSliderAttachment(audioProcessor.treeState, "Band3 Gain", band3GainSlider),
    band3QualitySliderAttachment(audioProcessor.treeState, "Band3 Quality", band3QualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.treeState,"LowCut Frequency", lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.treeState, "HighCut Frequency", highCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.treeState,"LowCut Slope" ,lowCutSlopeCombo),
    highCutSlopeSliderAttachment(audioProcessor.treeState,"HighCut Slope", highCutSlopeCombo)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    juce::String minFreq = "20 Hz";
    juce::String maxFreq = "20 kHz";
    juce::String minGain = "-24 dB";
    juce::String maxGain = "24 dB";
    band1FreqSlider.labels.add({ 0.f, minFreq });
    band1FreqSlider.labels.add({ 1.f, maxFreq });
    band1GainSlider.labels.add({ 0.f, minGain });
    band1GainSlider.labels.add({ 1.f, maxGain });
    band1QualitySlider.labels.add({ 0.f, "0.1" });
    band1QualitySlider.labels.add({ 1.f, "10"});
    band2FreqSlider.labels.add({ 0.f, minFreq });
    band2FreqSlider.labels.add({ 1.f, maxFreq });
    band2GainSlider.labels.add({ 0.f, minGain });
    band2GainSlider.labels.add({ 1.f, maxGain });
    band2QualitySlider.labels.add({ 0.f, "0.1" });
    band2QualitySlider.labels.add({ 1.f, "10" });
    band3FreqSlider.labels.add({ 0.f, minFreq });
    band3FreqSlider.labels.add({ 1.f, maxFreq });
    band3GainSlider.labels.add({ 0.f, minGain });
    band3GainSlider.labels.add({ 1.f, maxGain });
    band3QualitySlider.labels.add({ 0.f, "0.1" });
    band3QualitySlider.labels.add({ 1.f, "10" });
    lowCutFreqSlider.labels.add({ 0.f, minFreq });
    lowCutFreqSlider.labels.add({ 1.f, maxFreq });
    highCutFreqSlider.labels.add({ 0.f, minFreq });
    highCutFreqSlider.labels.add({ 1.f, maxFreq });


    
    int slopeVal=12, firstSlopeVal=slopeVal;
    juce::Array<juce::String> slopeStr = { "12", "24", "36", "48" };
    juce::String suffix(" dB/Oct");
    for (int i = 0; i < 4; i++) {
        slopeVal = slopeVal + 12 * i;
        lowCutSlopeCombo.addItem(slopeStr[i] + suffix, slopeVal);
        highCutSlopeCombo.addItem(slopeStr[i] + suffix, slopeVal);
    }

    lowCutSlopeCombo.setSelectedId(firstSlopeVal);
    highCutSlopeCombo.setSelectedId(firstSlopeVal);

       

    
    for (auto* comp : getComps()) {
        addAndMakeVisible(comp);
    }

    
    setSize (800, 600);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{

}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);

    
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    float ratio = 40.f / 100.f;//JUCE_LIVE_CONSTANT(33) / 100.f;  //25.f / 100.f;
    float slopeWidthCut = 0.2;
    int nBands = 3;
    juce::Rectangle<int> responseArea = bounds.removeFromTop(bounds.getHeight() * ratio);

    responseCurveComponent.setBounds(responseArea);
    bounds.removeFromTop(5);
    bounds.removeFromLeft(5);
    bounds.removeFromRight(5);


    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.2);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.25);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    juce::Rectangle<int> lowSlopeRect = lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.2);
    lowSlopeRect.removeFromRight(lowCutArea.getWidth() * slopeWidthCut);
    lowSlopeRect.removeFromLeft(lowCutArea.getWidth() * slopeWidthCut);
    lowCutSlopeCombo.setBounds(lowSlopeRect);
    

    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.4));
    juce::Rectangle<int> highSlopeRect = highCutArea.removeFromTop(highCutArea.getHeight() * 0.2);
    highSlopeRect.removeFromRight(highCutArea.getWidth() * slopeWidthCut);
    highSlopeRect.removeFromLeft(highCutArea.getWidth() * slopeWidthCut);
    highCutSlopeCombo.setBounds(highSlopeRect);

    juce::Rectangle<int> bandRect = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    band1FreqSlider.setBounds(bandRect.removeFromTop(bounds.getHeight() * 0.33));
    band1GainSlider.setBounds(bandRect.removeFromTop(bounds.getHeight() * 0.33));
    band1QualitySlider.setBounds(bandRect);

    bandRect = bounds.removeFromLeft(bounds.getWidth() * 0.5);
    band2FreqSlider.setBounds(bandRect.removeFromTop(bounds.getHeight() * 0.33));
    band2GainSlider.setBounds(bandRect.removeFromTop(bounds.getHeight() * 0.33));
    band2QualitySlider.setBounds(bandRect);

    bandRect = bounds;
    band3FreqSlider.setBounds(bandRect.removeFromTop(bounds.getHeight() * 0.33));
    band3GainSlider.setBounds(bandRect.removeFromTop(bounds.getHeight() * 0.33));
    band3QualitySlider.setBounds(bandRect);
}


std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    
    return{
        &band1FreqSlider,
        &band1GainSlider,
        &band1QualitySlider,
        &band2FreqSlider,
        &band2GainSlider,
        &band2QualitySlider,
        &band3FreqSlider,
        &band3GainSlider,
        &band3QualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeCombo,
        &highCutSlopeCombo,
        &responseCurveComponent
    };   

}

