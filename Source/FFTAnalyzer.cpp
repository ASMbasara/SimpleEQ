#include "FFTAnalyzer.h"

FFTPathProducer::FFTPathProducer(SampleFifo& fifoToUse)
    : sampleFifo(fifoToUse)
{
    smoothedMagnitudes.fill(0.0f);
}

void FFTPathProducer::process(juce::Rectangle<float> bounds, double sampleRate)
{
    if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0 || sampleRate <= 0)
    {
        drain();
        return;
    }

    // Pull every available sample from the lock-free FIFO
    std::array<float, analyzerFFTSize * 2> tempBuffer{};
    int numPulled = sampleFifo.pull(tempBuffer.data(),
        static_cast<int>(tempBuffer.size()));

    if (numPulled == 0 && !hasMagnitudes)
        return;

    bool newFFTReady = false;

    for (int i = 0; i < numPulled; ++i)
    {
        inputBuffer[static_cast<size_t>(inputWriteIndex++)] = tempBuffer[static_cast<size_t>(i)];

        if (inputWriteIndex >= analyzerFFTSize)
        {
            // Copy accumulated block into fftData, zero-pad the second half
            std::copy(inputBuffer.begin(), inputBuffer.end(), fftData.begin());
            std::fill(fftData.begin() + analyzerFFTSize, fftData.end(), 0.0f);

            window.multiplyWithWindowingTable(fftData.data(),
                static_cast<size_t>(analyzerFFTSize));
            forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

            // Exponential smoothing on the magnitude bins
            constexpr float smoothing = 0.9f;
            constexpr int   numBins = analyzerFFTSize / 2;

            for (int bin = 0; bin < numBins; ++bin)
            {
                // Correct normalisation: ×2 for one-sided spectrum,
                // ×2 for Hann-window coherent-gain compensation = ×4/N
                float normalized = fftData[static_cast<size_t>(bin)]
                    * 4.0f / static_cast<float>(analyzerFFTSize);

                if (hasMagnitudes)
                    smoothedMagnitudes[static_cast<size_t>(bin)] =
                    smoothedMagnitudes[static_cast<size_t>(bin)] * smoothing
                    + normalized * (1.0f - smoothing);
                else
                    smoothedMagnitudes[static_cast<size_t>(bin)] = normalized;
            }

            hasMagnitudes = true;
            newFFTReady = true;
            inputWriteIndex = 0;
        }
    }

    if (newFFTReady)
        generatePath(bounds, sampleRate);
}

void FFTPathProducer::drain()
{
    std::array<float, analyzerFFTSize * 2> temp{};
    while (sampleFifo.getNumReady() > 0)
        sampleFifo.pull(temp.data(), static_cast<int>(temp.size()));
    inputWriteIndex = 0;
}

void FFTPathProducer::generatePath(juce::Rectangle<float> bounds, double sampleRate)
{
    const int widthInt = static_cast<int>(bounds.getWidth());
    if (widthInt <= 0) return;

    constexpr int   numBins = analyzerFFTSize / 2;
    const float     binWidth = static_cast<float>(sampleRate)
        / static_cast<float>(analyzerFFTSize);

    constexpr float minFreq = 20.0f;
    constexpr float maxFreq = 20000.0f;
    constexpr float negInfDB = -96.0f;   // true silence floor for gainToDecibels
    constexpr float minDB = -48.0f;    // display bottom (wide range for analyzer)
    constexpr float maxDB = 6.0f;      // display top
    constexpr float visualBoostDB = 12.0f;     // brings broadband music into visible range

    // --- Pass 1: bin interpolation → per-pixel dB values with visual boost ---
    std::vector<float> dbValues(static_cast<size_t>(widthInt));

    for (int x = 0; x < widthInt; ++x)
    {
        float normX = static_cast<float>(x) / bounds.getWidth();
        float freq = juce::mapToLog10(normX, minFreq, maxFreq);

        float fractBin = freq / binWidth;
        int   bin0 = juce::jlimit(0, numBins - 1,
            static_cast<int>(std::floor(fractBin)));
        int   bin1 = juce::jlimit(0, numBins - 1, bin0 + 1);
        float frac = fractBin - std::floor(fractBin);

        float magnitude = smoothedMagnitudes[static_cast<size_t>(bin0)] * (1.0f - frac)
            + smoothedMagnitudes[static_cast<size_t>(bin1)] * frac;

        float dB = juce::Decibels::gainToDecibels(magnitude, negInfDB) + visualBoostDB;
        dbValues[static_cast<size_t>(x)] = juce::jlimit(minDB, maxDB, dB);
    }

    // --- Pass 2 & 3: two moving-average passes (triangular kernel) ---
    constexpr int smoothHalfWidth = 4;

    for (int pass = 0; pass < 2; ++pass)
    {
        std::vector<float> smoothed(static_cast<size_t>(widthInt));
        for (int x = 0; x < widthInt; ++x)
        {
            float sum = 0.0f;
            int   lo = juce::jmax(0, x - smoothHalfWidth);
            int   hi = juce::jmin(widthInt - 1, x + smoothHalfWidth);
            for (int k = lo; k <= hi; ++k)
                sum += dbValues[static_cast<size_t>(k)];
            smoothed[static_cast<size_t>(x)] = sum / static_cast<float>(hi - lo + 1);
        }
        dbValues = std::move(smoothed);
    }

    // --- Pass 3: build the path ---
    latestPath.clear();

    for (int x = 0; x < widthInt; ++x)
    {
        float y = juce::jmap(dbValues[static_cast<size_t>(x)], minDB, maxDB,
            bounds.getBottom(), bounds.getY());
        float px = bounds.getX() + static_cast<float>(x);

        if (x == 0)
            latestPath.startNewSubPath(px, y);
        else
            latestPath.lineTo(px, y);
    }
}