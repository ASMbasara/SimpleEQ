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
    bandFreqSliderAttachment(audioProcessor.treeState, "Band Frequency", bandFreqSlider),
    bandGainSliderAttachment(audioProcessor.treeState, "Band Gain", bandGainSlider),
    bandQualitySliderAttachment(audioProcessor.treeState, "Band Quality", bandQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.treeState,"LowCut Frequency", lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.treeState, "HighCut Frequency", highCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.treeState,"LowCut Slope" ,lowCutSlopeSlider),
    highCutSlopeSliderAttachment(audioProcessor.treeState,"HighCut Slope", highCutSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
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

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    auto width = responseArea.getWidth();

    auto& lowcut = monoChain.get<LowCut>();
    auto& band = monoChain.get<Band>();
    auto& highcut = monoChain.get<HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> mags(width);

    for (int i = 0; i < width; i++) {
        double mag = 1.f;
        auto freq = juce::mapToLog10(double(i) / double(width), 20.0, 20000.0);

        if (!monoChain.isBypassed<Band>())
            mag *= band.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        getMagForFreq(mag, lowcut, 4, freq);
        getMagForFreq(mag, highcut, 4, freq);

        mags[i] = juce::Decibels::gainToDecibels(mag);
    }

    juce::Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();

    auto map = [outputMin, outputMax](double input)
    {
        return juce::jmap(input, -24.0, 24.0, outputMin, outputMax);
    };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 0; i < mags.size(); i++) {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    g.setColour(juce::Colours::darkcyan);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);

    g.setColour(juce::Colours::white);
    g.strokePath(responseCurve, juce::PathStrokeType(2.f));
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);

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
        &lowCutSlopeSlider,
        &highCutSlopeSlider
    };   

}
