#include "ResponseCurveComponent.h"


ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor& p) : 
    audioProcessor(p)
    ,leftPathProducer(p.leftChannelFifo)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->addListener(this);
    }

    fftToggleButton.setClickingTogglesState(true);
    fftToggleButton.setToggleState(true, juce::dontSendNotification);
    fftToggleButton.setColour(juce::TextButton::buttonOnColourId, lightBlue);
    fftToggleButton.setColour(juce::TextButton::buttonColourId, darkBlue);
    fftToggleButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    fftToggleButton.setColour(juce::TextButton::textColourOffId, dimGrey);
    fftToggleButton.onClick = [this]() 
        { 
            showFFT = fftToggleButton.getToggleState(); 
        };
    addAndMakeVisible(fftToggleButton);

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
    bool needsRepaint = false;

    if (parametersChanged.compareAndSetBool(false, true)) {
        updateChain();
        needsRepaint = true;
    }

    if (showFFT)
    {
        auto bounds = getAnalysisArea().toFloat();
        leftPathProducer.process(bounds, audioProcessor.getSampleRate());
        needsRepaint = true;
    }
    else
    {
        if (leftPathProducer.getPath().isEmpty() == false)
        {
            leftPathProducer.drain();
            needsRepaint = true;
        }
    }

    if (needsRepaint)
        repaint();
}

void ResponseCurveComponent::updateChain() {
    double sampleRate = audioProcessor.getSampleRate();
    auto chainSettings = audioProcessor.getChainSettings(audioProcessor.treeState);

    for (int i = 0; i < audioProcessor.nBands; i++) {
        auto peakCoefficients = makeBandFilter(chainSettings, audioProcessor.getSampleRate(), i);
        if (i == 0) {
            auto& bandFilterCoefficients = monoChain.get<Band1>().coefficients;
            bandFilterCoefficients = peakCoefficients;
        }
        if (i == 1) {
            auto& bandFilterCoefficients = monoChain.get<Band2>().coefficients;
            bandFilterCoefficients = peakCoefficients;
        }
        if (i == 2) {
            auto& bandFilterCoefficients = monoChain.get<Band3>().coefficients;
            bandFilterCoefficients = peakCoefficients;
        }
        
    }

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

    if (showFFT)
    {
        auto fftPath = leftPathProducer.getPath();
        g.setColour(lightBlue.withAlpha(0.6f));
        g.strokePath(fftPath, juce::PathStrokeType(1.5f));
    }

    auto& lowcut = monoChain.get<LowCut>();
    auto& band1 = monoChain.get<Band1>();
    auto& band2 = monoChain.get<Band2>();
    auto& band3 = monoChain.get<Band3>();
    auto& highcut = monoChain.get<HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> mags(width);

    for (int i = 0; i < width; i++) {
        double mag = 1.f;
        auto freq = juce::mapToLog10(double(i) / double(width), 20.0, 20000.0);

        if (!monoChain.isBypassed<Band1>())
            mag *= band1.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!monoChain.isBypassed<Band2>())
            mag *= band2.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!monoChain.isBypassed<Band3>())
            mag *= band3.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        getMagForFreq(mag, lowcut, 4, freq);
        getMagForFreq(mag, highcut, 4, freq);

        mags[i] = juce::Decibels::gainToDecibels(mag);
    }

    juce::Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();

    auto map = [outputMin, outputMax](double input)
        {
            return juce::jmap(input, -12.0, 12.0, outputMin, outputMax);
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
        -12, -6, -3, 0, 3, 6, 12
    };

    juce::Array<float> y_gain;
    for (auto gDB : gain) {
        auto y = jmap(gDB, -12.f, 12.f, float(bottom), float(top));
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
        r.setCentre(x, 0);
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
        r.setX(left - textWidth - 2);

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);

    }

    auto buttonArea = getLocalBounds().removeFromTop(16).removeFromRight(36);
    fftToggleButton.setBounds(buttonArea);

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

    bounds.reduce(0, 5);

    return bounds;
}