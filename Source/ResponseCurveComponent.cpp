#include "ResponseCurveComponent.h"


ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor& p) :
    audioProcessor(p)
    , leftPathProducer(p.leftChannelFifo)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->addListener(this);
    }

    // Modern FFT toggle button styling
    fftToggleButton.setClickingTogglesState(true);
    fftToggleButton.setToggleState(true, juce::dontSendNotification);

    // Minimal modern colors
    fftToggleButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(70, 150, 255).withAlpha(0.15f));
    fftToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colour(25, 27, 32));
    fftToggleButton.setColour(juce::TextButton::textColourOnId, juce::Colour(70, 150, 255));
    fftToggleButton.setColour(juce::TextButton::textColourOffId, juce::Colour(80, 85, 95));
    fftToggleButton.setColour(juce::ComboBox::outlineColourId, juce::Colour(50, 55, 65));

    fftToggleButton.onClick = [this]()
        {
            bShowFFT = fftToggleButton.getToggleState();
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

    if (bShowFFT)
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
    using namespace juce;

    // Clean dark background matching the main UI
    g.fillAll(Colour(20, 22, 25));

    // Draw grid/background
    g.drawImage(background, getLocalBounds().toFloat());

    auto responseArea = getAnalysisArea();
    auto width = responseArea.getWidth();

    // Draw FFT spectrum with modern gradient
    if (bShowFFT)
    {
        auto fftPath = leftPathProducer.getPath();

        // Subtle gradient fill under FFT
        ColourGradient fftGradient(
            Colour(70, 150, 255).withAlpha(0.15f),
            responseArea.toFloat().getBottomLeft(),
            Colour(70, 150, 255).withAlpha(0.0f),
            responseArea.toFloat().getTopLeft(),
            false
        );
        g.setGradientFill(fftGradient);
        g.fillPath(fftPath);

        // Bright FFT line
        g.setColour(Colour(70, 150, 255).withAlpha(0.7f));
        g.strokePath(fftPath, PathStrokeType(1.8f));
    }

    auto& lowcut = monoChain.get<LowCut>();
    auto& band1 = monoChain.get<Band1>();
    auto& band2 = monoChain.get<Band2>();
    auto& band3 = monoChain.get<Band3>();
    auto& highcut = monoChain.get<HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();
    auto chainSettings = audioProcessor.getChainSettings(audioProcessor.treeState);

    std::vector<double> mags(width);

    for (int i = 0; i < width; i++) {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(width), 20.0, 20000.0);

        // Check bypass state for bands
        if (!chainSettings.band1Bypass)
            mag *= band1.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!chainSettings.band2Bypass)
            mag *= band2.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!chainSettings.band3Bypass)
            mag *= band3.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        // Check bypass state for cut filters
        if (!chainSettings.lowCutBypass)
            getMagForFreq(mag, lowcut, 4, freq);
        if (!chainSettings.highCutBypass)
            getMagForFreq(mag, highcut, 4, freq);

        mags[i] = Decibels::gainToDecibels(mag);
    }

    Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();

    auto map = [outputMin, outputMax](double input)
        {
            return jmap(input, -12.0, 12.0, outputMin, outputMax);
        };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 0; i < mags.size(); i++) {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    // Gradient fill under response curve
    ColourGradient curveGradient(
        Colour(100, 220, 255).withAlpha(0.2f),
        responseArea.toFloat().getBottomLeft(),
        Colour(100, 220, 255).withAlpha(0.0f),
        responseArea.toFloat().getTopLeft(),
        false
    );

    Path fillPath = responseCurve;
    fillPath.lineTo(responseArea.getRight(), responseArea.getBottom());
    fillPath.lineTo(responseArea.getX(), responseArea.getBottom());
    fillPath.closeSubPath();

    g.setGradientFill(curveGradient);
    g.fillPath(fillPath);

    // Modern response curve line with glow effect
    g.setColour(Colour(100, 220, 255).withAlpha(0.3f));
    g.strokePath(responseCurve, PathStrokeType(3.5f)); // Outer glow

    g.setColour(Colour(150, 240, 255));
    g.strokePath(responseCurve, PathStrokeType(2.2f)); // Main line

    // --- Filter frequency indicators on the response curve ---
    struct FilterIndicator {
        float freq;
        Colour colour;
        bool bypassed;
    };

    FilterIndicator indicators[] = {
        { chainSettings.lowCutFreq,  Colour(255, 80, 80),   chainSettings.lowCutBypass },
        { chainSettings.band1Freq,   Colour(255, 160, 70),  chainSettings.band1Bypass },
        { chainSettings.band2Freq,   Colour(100, 220, 120), chainSettings.band2Bypass },
        { chainSettings.band3Freq,   Colour(80, 180, 255),  chainSettings.band3Bypass },
        { chainSettings.highCutFreq, Colour(180, 100, 255), chainSettings.highCutBypass },
    };

    for (auto& ind : indicators)
    {
        if (ind.bypassed)
            continue;

        // Map frequency to x-pixel within the response area
        auto normX = mapFromLog10(static_cast<double>(ind.freq), 20.0, 20000.0);
        int pixelIndex = static_cast<int>(normX * width);
        pixelIndex = jlimit(0, (int)mags.size() - 1, pixelIndex);

        float x = static_cast<float>(responseArea.getX() + pixelIndex);
        float y = static_cast<float>(map(mags[pixelIndex]));

        constexpr float outerRadius = 6.0f;
        constexpr float innerRadius = 4.0f;

        // Outer glow
        g.setColour(ind.colour.withAlpha(0.2f));
        g.fillEllipse(x - outerRadius, y - outerRadius, outerRadius * 2, outerRadius * 2);

        // Solid inner dot
        g.setColour(ind.colour);
        g.fillEllipse(x - innerRadius, y - innerRadius, innerRadius * 2, innerRadius * 2);

        // Bright center highlight
        g.setColour(ind.colour.brighter(0.5f));
        g.fillEllipse(x - 1.5f, y - 1.5f, 3.0f, 3.0f);
    }
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

    auto analysisArea = getAnalysisArea();
    auto left = analysisArea.getX();
    auto right = analysisArea.getRight();
    auto top = analysisArea.getY();
    auto bottom = analysisArea.getBottom();
    auto width = analysisArea.getWidth();
    auto height = analysisArea.getHeight();

    // Minimal grid lines
    g.setColour(Colour(35, 38, 45).withAlpha(0.6f));

    Array<float> x_freq;
    for (auto f : freqs) {
        auto normX = mapFromLog10(f, 20.f, 20000.f);
        auto xs = left + width * normX;
        x_freq.add(xs);

        // Make important frequencies (100Hz, 1kHz, 10kHz) slightly more visible
        if (f == 100.f || f == 1000.f || f == 10000.f) {
            g.setColour(Colour(45, 50, 58).withAlpha(0.8f));
        }
        else {
            g.setColour(Colour(35, 38, 45).withAlpha(0.5f));
        }
        g.drawVerticalLine(xs, top, bottom);
    }

    Array<float> gain{
        -12, -6, -3, 0, 3, 6, 12
    };

    Array<float> y_gain;
    for (auto gDB : gain) {
        auto y = jmap(gDB, -12.f, 12.f, float(bottom), float(top));
        y_gain.add(y);

        // Highlight 0dB line
        if (gDB == 0.f) {
            g.setColour(Colour(60, 70, 85).withAlpha(0.8f));
        }
        else {
            g.setColour(Colour(35, 38, 45).withAlpha(0.5f));
        }
        g.drawHorizontalLine(y, left, right);
    }

    // Modern font and labels
    g.setFont(Font("Exo 2", 9.5f, Font::plain));
    g.setColour(Colour(140, 150, 165));

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
        r.setY(2);

        g.drawFittedText(str, r.toNearestInt(), Justification::centred, 1);
    }

    for (int i = 0; i < gain.size(); i++) {
        auto y = gain[i];
        auto x = y_gain[i];
        String str;

        if (y > 0)
            str << "+" << String(y);
        else
            str = String(y);

        str << "dB";

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<float> r;
        r.setSize(textWidth + 2, g.getCurrentFont().getHeight());
        r.setCentre(0, x);
        r.setX(left - textWidth - 4);

        g.drawFittedText(str, r.toNearestInt(), Justification::centredRight, 1);
    }

    // Modern FFT button placement
    auto buttonArea = analysisArea.removeFromTop(18).removeFromRight(40).reduced(1);
    fftToggleButton.setBounds(buttonArea);
}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();

    bounds.removeFromTop(16);
    bounds.removeFromLeft(32);
    bounds.removeFromRight(14);
    bounds.removeFromBottom(2);

    return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
    auto bounds = getRenderArea();
    bounds.reduce(0, 4);
    return bounds;
}