/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    g.setColour(lightBlue);
    g.fillEllipse(bounds);

    g.setColour(darkBlue);
    g.drawEllipse(bounds, 5.f);


    if (RotarySliderWithLabels* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)) {
        auto center = bounds.getCentre();

        Path p;

        
        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY() + bounds.getWidth()*0.05);
        r.setBottom(center.getY() - rswl->getTextHeight() * 2.5);

        p.addRoundedRectangle(r.toFloat(), 4.f, 1.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPos, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.setColour(Colour(255u, 255u, 255u));
        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        g.setColour(Colour(255u, 255u, 255u));
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);

    }

    


}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    float startAng = degreesToRadians(180.f + 45.f); 
    float endAng = startAng - MathConstants<double>::halfPi + MathConstants<double>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    /*g.setColour(Colour(0u, 19u, 17u));
    g.drawRect(getLocalBounds());
    g.setColour(Colour(41u, 23u, 19u));
    g.drawRect(getSliderBounds());*/

    getLookAndFeel().drawRotarySlider(g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        param->convertTo0to1(getValue()),
        startAng,
    endAng,
        *this);
       
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

    g.setColour(lightBlue);
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; i++) {
        auto pos = labels[i].pos;
        bool above = pos > 0.25f && pos < 0.75f;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);

        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

        auto c = center.getPointOnCircumference(radius + getTextHeight() * 1, ang);

        Rectangle<float> r;
        auto text = labels[i].label;
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        r.setSize(strWidth + 4, getTextHeight() + 2);
        r.setCentre(c);
        above ? r.setY(r.getY() - getTextHeight() * 0.33) : r.setY(r.getY() + getTextHeight() * 0.33);


        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (juce::AudioParameterChoice* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param)) {
        juce::String val = choiceParam->getCurrentChoiceName();
        if (choiceParam->getParameterID().containsWholeWord("Slope")) {
            val.append(" ", 1);
            val.append(suffix, suffix.length());
        }
        return val;
    }

    juce::String str;
    bool addK = false;

    
    if (juce::AudioParameterFloat* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param)) {
        

        float value = getValue();

        if ( value >= 1000.f) {
            value /= 1000.f;
            addK = true;
        }

        if (getName().contains("Slope"))
            value = 12 + value * 12;
        str = juce::String(value, (addK ? 2 : 0));
    }
    else {
        jassertfalse;
    }

    if (suffix.isNotEmpty()) {
        str << " ";
        if (addK)
            str << "k";
        str << suffix;
    }

    return str;

}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds()
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), bounds.getCentreY());
    r.setY(2);

    return r;
}

ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->addListener(this);
    }

    startTimerHz(60);

    updateChain();
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true)) {
        //update monochain
        
        updateChain();
        //signal a repaint
        repaint();
    }
}

void ResponseCurveComponent::updateChain() {
    double sampleRate = audioProcessor.getSampleRate();
    auto chainSettings = audioProcessor.getChainSettings(audioProcessor.treeState);

    auto peakCoefficients = makeBandFilter(chainSettings, audioProcessor.getSampleRate());
    auto& bandFilterCoefficients = monoChain.get<Band>().coefficients;
    bandFilterCoefficients = peakCoefficients;

    auto newLowCutCoefficients = makeLowCutFilter(chainSettings, sampleRate);
    audioProcessor.updateCutCoefficients(monoChain.get<LowCut>(), newLowCutCoefficients, chainSettings.lowCutSlope);

    auto newHighCutCoefficients = makeHighCutFilter(chainSettings, sampleRate);
    audioProcessor.updateCutCoefficients(monoChain.get<HighCut>(), newHighCutCoefficients, chainSettings.highCutSlope);
}


void ResponseCurveComponent::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::black);

    g.drawImage(background, getLocalBounds().toFloat());

    auto responseArea = getAnalysisArea();
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

    g.setColour(darkBlue);
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);

    g.setColour(juce::Colours::white);
    g.strokePath(responseCurve, juce::PathStrokeType(2.f));
}

void ResponseCurveComponent::resized()
{
    using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);

    Graphics g(background);

    Array<float> freqs{
        20, 30, 50, 100,
        200, 300, 500, 1000,
        2000, 3000, 5000, 10000,
        20000
    };


    g.setColour(dimGrey);
    //

    auto analysisArea = getAnalysisArea();
    auto left = analysisArea.getX();
    auto right = analysisArea.getRight();
    auto top = analysisArea.getY();
    auto bottom = analysisArea.getBottom();
    auto width = analysisArea.getWidth();
    auto height = analysisArea.getHeight();

    juce::Array<float> x_freq;
    for (auto f : freqs) {
        auto normX = mapFromLog10(f, 20.f, 20000.f);
        auto xs = left + width * normX;
        x_freq.add(xs);
        g.drawVerticalLine(xs, top, bottom);
    }

    Array<float> gain{
        -24, -12, -6, -3, 0, 3, 6, 12, 24
    };

    juce::Array<float> y_gain;
    for (auto gDB : gain) {
        auto y = jmap(gDB, -24.f, 24.f, float(bottom), float(top));
        //g.setColour(gDB == 0 ? Colour(0u, 19u, 17u) : Colour(66u, 66u, 66u));
        y_gain.add(y);
        g.drawHorizontalLine(y, left, right);
    }

    g.setFont(10);
    g.setColour(lightBlue);
    for (int i = 0; i < freqs.size(); i++) {
        auto f = freqs[i];
        auto x = x_freq[i];

        bool addK = false;
        String str;

        if (f >= 1000.f) {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if (addK)
            str << "k";
        str << "Hz";

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<float> r;
        r.setSize(textWidth, g.getCurrentFont().getHeight());
        r.setCentre(x,0);
        r.setY(1);

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);

    }

    for (int i = 0; i < gain.size(); i++) {
        auto y = gain[i];
        auto x = y_gain[i];
        String str;
        
        if (y > 0)
            str << "+" << String(y);
        else
            str = String(y);

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<float> r;
        r.setSize(textWidth, g.getCurrentFont().getHeight());
        r.setCentre(0, x);
        r.setX(left - textWidth - 2 );

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);

    }



}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();

    bounds.removeFromTop(12);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(2);


    return bounds;
}
juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
    auto bounds = getRenderArea();

    bounds.reduce(0,5);

    return bounds;
}

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
    lowCutSlopeSlider(*audioProcessor.treeState.getParameter("LowCut Slope"), "dB/Oct"),
    highCutSlopeSlider(*audioProcessor.treeState.getParameter("HighCut Slope"), "dB/Oct"),
    responseCurveComponent(audioProcessor),
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

    float slopePos = 0;
    juce::Array<juce::String> slopeStr = { "12", "24", "36", "48" };
    for (int i = 0; i < 4; i++) {
        slopePos = 0 + 0.333333 * i;
        lowCutSlopeSlider.labels.add({ slopePos, slopeStr[i] });
        highCutSlopeSlider.labels.add({ slopePos, slopeStr[i] });
    }

    
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
        &highCutSlopeSlider,
        &responseCurveComponent
    };   

}

