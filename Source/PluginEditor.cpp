/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static constexpr float ResponseCurveRatio = 0.50f;

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor& p)
    :
    AudioProcessorEditor(&p),
    audioProcessor(p),
    band1FreqSlider(*audioProcessor.treeState.getParameter("Band1 Frequency"), "Hz", "FREQ"),
    band1GainSlider(*audioProcessor.treeState.getParameter("Band1 Gain"), "dB", "GAIN"),
    band1QualitySlider(*audioProcessor.treeState.getParameter("Band1 Quality"), "", "Q"),
    band2FreqSlider(*audioProcessor.treeState.getParameter("Band2 Frequency"), "Hz", "FREQ"),
    band2GainSlider(*audioProcessor.treeState.getParameter("Band2 Gain"), "dB", "GAIN"),
    band2QualitySlider(*audioProcessor.treeState.getParameter("Band2 Quality"), "", "Q"),
    band3FreqSlider(*audioProcessor.treeState.getParameter("Band3 Frequency"), "Hz", "FREQ"),
    band3GainSlider(*audioProcessor.treeState.getParameter("Band3 Gain"), "dB", "GAIN"),
    band3QualitySlider(*audioProcessor.treeState.getParameter("Band3 Quality"), "", "Q"),
    lowCutFreqSlider(*audioProcessor.treeState.getParameter("LowCut Frequency"), "Hz", "FREQ"),
    highCutFreqSlider(*audioProcessor.treeState.getParameter("HighCut Frequency"), "Hz", "FREQ"),
    lowCutSlopeCombo(juce::String("LowCut Slope"), juce::String("dB/Oct"), "SLOPE"),
    highCutSlopeCombo(juce::String("HighCut Slope"), juce::String("dB/Oct"), "SLOPE"),
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
    lowCutFreqSliderAttachment(audioProcessor.treeState, "LowCut Frequency", lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.treeState, "HighCut Frequency", highCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.treeState, "LowCut Slope", lowCutSlopeCombo),
    highCutSlopeSliderAttachment(audioProcessor.treeState, "HighCut Slope", highCutSlopeCombo)
{
    // Populate Slope Combos
    juce::Array<juce::String> slopeStr = { "12", "24", "36", "48" };
    juce::String suffix(" dB/Oct");
    for (int i = 0; i < 4; i++) {
        int slopeVal = 12 + (12 * i);
        lowCutSlopeCombo.addItem(slopeStr[i] + suffix, slopeVal);
        highCutSlopeCombo.addItem(slopeStr[i] + suffix, slopeVal);
    }

    lowCutSlopeCombo.setSelectedId(12);
    highCutSlopeCombo.setSelectedId(12);

    for (auto* comp : getComps()) {
        addAndMakeVisible(comp);
    }

    setSize(900, 750);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    using namespace juce;

    // Clean dark background
    g.fillAll(Colour(15, 15, 17));

    auto bounds = getLocalBounds();

    // Minimal title bar
    Rectangle<int> titleArea = bounds.removeFromTop(40);

    // Title with subtle accent
    g.setColour(Colour(220, 225, 235));
    g.setFont(Font("Exo 2", 20.0f, Font::bold));
    g.drawText("SIMPLE EQ", titleArea.reduced(20, 0), Justification::centredLeft, true);

    // Subtle version/info
    g.setColour(Colour(120, 130, 145).withAlpha(0.6f));
    g.setFont(Font("Exo 2", 10.0f, Font::plain));
    g.drawText("v1.0", titleArea.reduced(20, 0), Justification::centredRight, true);

    // Response curve section
    Rectangle<int> responseArea = bounds.removeFromTop(bounds.getHeight() * ResponseCurveRatio);
    responseArea = responseArea.reduced(15, 10);

    // Simple card with subtle border
    g.setColour(Colour(20, 22, 25));
    g.fillRoundedRectangle(responseArea.toFloat(), 8.0f);

    g.setColour(Colour(45, 50, 60).withAlpha(0.5f));
    g.drawRoundedRectangle(responseArea.toFloat(), 8.0f, 1.0f);

    // Control sections
    bounds.removeFromTop(15);
    auto controlBounds = bounds.reduced(15, 10);

    auto lowCutWidth = controlBounds.getWidth() * 0.18f;
    auto highCutWidth = controlBounds.getWidth() * 0.18f;
    auto bandWidth = (controlBounds.getWidth() - lowCutWidth - highCutWidth) / 3.0f;

    Rectangle<int> lowCutArea = controlBounds.removeFromLeft(lowCutWidth);
    Rectangle<int> band1Area = controlBounds.removeFromLeft(bandWidth);
    Rectangle<int> band2Area = controlBounds.removeFromLeft(bandWidth);
    Rectangle<int> band3Area = controlBounds.removeFromLeft(bandWidth);
    Rectangle<int> highCutArea = controlBounds;

    // Spectrum-mapped color palette
    Colour lowCutColor(255, 80, 80);      // Red
    Colour band1Color(255, 160, 70);      // Orange
    Colour band2Color(100, 220, 120);     // Green
    Colour band3Color(80, 180, 255);      // Cyan/Blue
    Colour highCutColor(180, 100, 255);   // Purple/Violet

    // Helper to draw section backgrounds and titles only
    auto drawSection = [&g](Rectangle<int> area, const String& title, Colour accentColor) {
        area = area.reduced(5, 3);

        // Card background
        g.setColour(Colour(20, 22, 25));
        g.fillRoundedRectangle(area.toFloat(), 6.0f);

        // Top accent bar
        Rectangle<float> accentBar = area.toFloat().removeFromTop(3);
        g.setColour(accentColor);
        g.fillRoundedRectangle(accentBar, 6.0f);

        // Subtle border
        g.setColour(Colour(40, 45, 52).withAlpha(0.6f));
        g.drawRoundedRectangle(area.toFloat(), 6.0f, 1.0f);

        // Section title
        Rectangle<int> titleArea = area.removeFromTop(28);
        g.setColour(accentColor.brighter(0.3f));
        g.setFont(Font("Exo 2", 11.0f, Font::bold));
        g.drawText(title, titleArea, Justification::centred, true);
        };

    // Draw sections (components draw their own labels)
    drawSection(lowCutArea, "LOW CUT", lowCutColor);
    drawSection(band1Area, "BAND 1", band1Color);
    drawSection(band2Area, "BAND 2", band2Color);
    drawSection(band3Area, "BAND 3", band3Color);
    drawSection(highCutArea, "HIGH CUT", highCutColor);
}

void SimpleEQAudioProcessorEditor::resized()
{
    using namespace juce;

    auto bounds = getLocalBounds();

    // Title bar
    bounds.removeFromTop(40);

    // Response curve area
    Rectangle<int> responseArea = bounds.removeFromTop(bounds.getHeight() * ResponseCurveRatio);
    responseArea = responseArea.reduced(15, 10);
    responseCurveComponent.setBounds(responseArea);

    // Spacing
    bounds.removeFromTop(15);

    // Control area
    bounds = bounds.reduced(15, 10);

	constexpr float CutFilterRatio = 0.18f;
    const float cutWidth = bounds.getWidth() * CutFilterRatio;
    const float bandWidth = (bounds.getWidth() - 2 * cutWidth) / 3.0f;

	auto fSetCutFilterSection = [&bounds, &cutWidth](RotarySliderWithLabels& freqSlider, MinimalCombo& slopeCombo)
    {
			static constexpr float GapRatio = 0.2f;
			static constexpr float SliderComboRatio = 0.8f;

            auto area = bounds.removeFromLeft(cutWidth).reduced(5, 3);

            const int gap = area.getHeight() * GapRatio;
            area.removeFromTop(gap);
            area.removeFromBottom(gap);

            const int availableHeight = area.getHeight();

            auto sliderSpace = availableHeight * SliderComboRatio; 
            freqSlider.setBounds(area.removeFromTop(sliderSpace));

            area.removeFromTop(3);

            slopeCombo.setBounds(area);
    };

    auto fSetBandFilterSection = [&bounds, &bandWidth](RotarySliderWithLabels& freq, RotarySliderWithLabels& gain, RotarySliderWithLabels& quality)
        {
            static constexpr float GapRatio = 0.1f;
            static constexpr float FreqRatio = 0.33f;
            static constexpr float GainRatio = 0.5f;

            auto area = bounds.removeFromLeft(bandWidth).reduced(5, 3);
            const int gap = area.getHeight() * GapRatio;
            area.removeFromTop(gap);
            freq.setBounds(area.removeFromTop(area.getHeight() * FreqRatio));
            gain.setBounds(area.removeFromTop(area.getHeight() * GainRatio));
            quality.setBounds(area);
        };

	fSetCutFilterSection(lowCutFreqSlider, lowCutSlopeCombo);

	fSetBandFilterSection(band1FreqSlider, band1GainSlider, band1QualitySlider);
	fSetBandFilterSection(band2FreqSlider, band2GainSlider, band2QualitySlider);
	fSetBandFilterSection(band3FreqSlider, band3GainSlider, band3QualitySlider);

    fSetCutFilterSection(highCutFreqSlider, highCutSlopeCombo);
}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    return {
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