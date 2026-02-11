#pragma once
#include <JuceHeader.h>

constexpr int analyzerFFTOrder = 11;
constexpr int analyzerFFTSize = 1 << analyzerFFTOrder; // 2048

//==============================================================================
// Lock-free FIFO for passing samples from the audio thread to the GUI thread.
class SampleFifo
{
public:
    SampleFifo() = default;

    void push(const float* data, int numSamples)
    {
        const auto scope = abstractFifo.write(numSamples);
        if (scope.blockSize1 > 0)
            std::copy(data, data + scope.blockSize1, buffer.data() + scope.startIndex1);
        if (scope.blockSize2 > 0)
            std::copy(data + scope.blockSize1,
                data + scope.blockSize1 + scope.blockSize2,
                buffer.data() + scope.startIndex2);
    }

    int pull(float* dest, int maxSamples)
    {
        const auto scope = abstractFifo.read(maxSamples);
        int total = scope.blockSize1 + scope.blockSize2;
        if (scope.blockSize1 > 0)
            std::copy(buffer.data() + scope.startIndex1,
                buffer.data() + scope.startIndex1 + scope.blockSize1,
                dest);
        if (scope.blockSize2 > 0)
            std::copy(buffer.data() + scope.startIndex2,
                buffer.data() + scope.startIndex2 + scope.blockSize2,
                dest + scope.blockSize1);
        return total;
    }

    int getNumReady() const { return abstractFifo.getNumReady(); }

private:
    static constexpr int capacity = analyzerFFTSize * 4;
    juce::AbstractFifo abstractFifo{ capacity };
    std::array<float, capacity> buffer{};
};

//==============================================================================
// Pulls samples from a SampleFifo, computes windowed FFT, and produces a
// smoothed juce::Path suitable for drawing a spectrum analyser overlay.
class FFTPathProducer
{
public:
    FFTPathProducer(SampleFifo& fifoToUse);

    void process(juce::Rectangle<float> bounds, double sampleRate);
    void drain();
    juce::Path getPath() const { return latestPath; }

private:
    SampleFifo& sampleFifo;

    juce::dsp::FFT forwardFFT{ analyzerFFTOrder };
    juce::dsp::WindowingFunction<float> window{
        static_cast<size_t>(analyzerFFTSize),
        juce::dsp::WindowingFunction<float>::hann
    };

    std::array<float, analyzerFFTSize>     inputBuffer{};
    int inputWriteIndex = 0;

    std::array<float, analyzerFFTSize * 2> fftData{};
    std::array<float, analyzerFFTSize / 2> smoothedMagnitudes{};
    bool hasMagnitudes = false;

    juce::Path latestPath;

    void generatePath(juce::Rectangle<float> bounds, double sampleRate);
};