/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static constexpr float ResponseCurveRatio = 0.50f;
static constexpr float CutFilterRatio = 0.18f;

static const juce::StringArray SlopeLabels{ "12 dB/Oct", "24 dB/Oct", "36 dB/Oct", "48 dB/Oct" };

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor& p)
    : AudioProcessorEditor(&p),
    audioProcessor(p),
    responseCurveComponent(audioProcessor),
    lowCutSection(audioProcessor.treeState,
        "LowCut Frequency", "LowCut Slope", "LowCut Bypass",
        SlopeLabels, "LOW CUT", Theme::LowCutAccent),
    band1Section(audioProcessor.treeState,
        "Band1 Frequency", "Band1 Gain", "Band1 Quality", "Band1 Bypass",
        "BAND 1", Theme::Band1Accent),
    band2Section(audioProcessor.treeState,
        "Band2 Frequency", "Band2 Gain", "Band2 Quality", "Band2 Bypass",
        "BAND 2", Theme::Band2Accent),
    band3Section(audioProcessor.treeState,
        "Band3 Frequency", "Band3 Gain", "Band3 Quality", "Band3 Bypass",
        "BAND 3", Theme::Band3Accent),
    highCutSection(audioProcessor.treeState,
        "HighCut Frequency", "HighCut Slope", "HighCut Bypass",
        SlopeLabels, "HIGH CUT", Theme::HighCutAccent)
{
    addAndMakeVisible(responseCurveComponent);
    addAndMakeVisible(lowCutSection);
    addAndMakeVisible(band1Section);
    addAndMakeVisible(band2Section);
    addAndMakeVisible(band3Section);
    addAndMakeVisible(highCutSection);

    setSize(900, 750);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    using namespace juce;

    g.fillAll(Theme::Background);

    auto bounds = getLocalBounds();

    // Title bar
    auto titleArea = bounds.removeFromTop(40);

    g.setColour(Theme::TitleText);
    g.setFont(Font(Theme::FontFamily, 20.0f, Font::bold));
    g.drawText("SIMPLE EQ", titleArea.reduced(20, 0), Justification::centredLeft, true);

    g.setColour(Theme::SubtleText.withAlpha(0.6f));
    g.setFont(Font(Theme::FontFamily, 10.0f, Font::plain));
    g.drawText("v1.0", titleArea.reduced(20, 0), Justification::centredRight, true);

    // Response curve card
    auto responseArea = bounds.removeFromTop(
        static_cast<int>(bounds.getHeight() * ResponseCurveRatio));
    responseArea = responseArea.reduced(15, 10);

    g.setColour(Theme::Card);
    g.fillRoundedRectangle(responseArea.toFloat(), 8.0f);

    g.setColour(Theme::CardBorder.withAlpha(0.5f));
    g.drawRoundedRectangle(responseArea.toFloat(), 8.0f, 1.0f);
}

void SimpleEQAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Title bar
    bounds.removeFromTop(40);

    // Response curve
    auto responseArea = bounds.removeFromTop(
        static_cast<int>(bounds.getHeight() * ResponseCurveRatio));
    responseCurveComponent.setBounds(responseArea.reduced(15, 10));

    // Spacing + control area
    bounds.removeFromTop(15);
    bounds = bounds.reduced(15, 10);

    const int cutWidth = static_cast<int>(bounds.getWidth() * CutFilterRatio);
    const int bandWidth = (bounds.getWidth() - 2 * cutWidth) / 3;

    lowCutSection.setBounds(bounds.removeFromLeft(cutWidth));
    band1Section.setBounds(bounds.removeFromLeft(bandWidth));
    band2Section.setBounds(bounds.removeFromLeft(bandWidth));
    band3Section.setBounds(bounds.removeFromLeft(bandWidth));
    highCutSection.setBounds(bounds);
}