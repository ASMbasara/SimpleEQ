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
    bandFreqSlider(*audioProcessor.treeState.getParameter("Band Frequency"), "Hz"),
    bandGainSlider(*audioProcessor.treeState.getParameter("Band Gain"), "dB"),
    bandQualitySlider(*audioProcessor.treeState.getParameter("Band Quality"), ""),
    lowCutFreqSlider(*audioProcessor.treeState.getParameter("LowCut Frequency"), "Hz"),
    highCutFreqSlider(*audioProcessor.treeState.getParameter("HighCut Frequency"), "Hz"),
    lowCutSlopeCombo(juce::String("LowCut Slope"), juce::String("dB/Oct")),
    highCutSlopeCombo(juce::String("HighCut Slope"), juce::String("dB/Oct")),
    responseCurveComponent(audioProcessor),
    bandFreqSliderAttachment(audioProcessor.treeState, "Band Frequency", bandFreqSlider),
    bandGainSliderAttachment(audioProcessor.treeState, "Band Gain", bandGainSlider),
    bandQualitySliderAttachment(audioProcessor.treeState, "Band Quality", bandQualitySlider),
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
    bandFreqSlider.labels.add({ 0.f, minFreq });
    bandFreqSlider.labels.add({ 1.f, maxFreq });
    bandGainSlider.labels.add({ 0.f, minGain });
    bandGainSlider.labels.add({ 1.f, maxGain });
    bandQualitySlider.labels.add({ 0.f, "0.1" });
    bandQualitySlider.labels.add({ 1.f, "10"});
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
    lowCutSlopeCombo.setEditableText(true);
    highCutSlopeCombo.setSelectedId(firstSlopeVal);
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
    juce::Rectangle<int> responseArea = bounds.removeFromTop(bounds.getHeight() * ratio);

    responseCurveComponent.setBounds(responseArea);
    bounds.removeFromTop(5);

    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeCombo.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.2));

    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    juce::Rectangle<int> highSlopeRect = highCutArea.removeFromTop(highCutArea.getHeight() * 0.2);
    highSlopeRect.removeFromRight(highCutArea.getWidth() * 0.66);
    highCutSlopeCombo.setBounds(highCutArea);

    bandFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    bandGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    bandQualitySlider.setBounds(bounds);
}


std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    
    return{
        &bandFreqSlider,
        &bandGainSlider,
        &bandQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeCombo,
        &highCutSlopeCombo,
        &responseCurveComponent
    };   

}

