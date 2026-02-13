#pragma once
#include <JuceHeader.h>

namespace Theme
{
    // Backgrounds
    inline const auto Background = juce::Colour(15, 15, 17);
    inline const auto Card = juce::Colour(20, 22, 25);
    inline const auto CardBorder = juce::Colour(40, 45, 52);

    // Accent colours
	inline const auto GenericAccent = juce::Colour(100, 180, 255);
    inline const auto LowCutAccent = juce::Colour(255, 80, 80);
    inline const auto Band1Accent = juce::Colour(255, 160, 70);
    inline const auto Band2Accent = juce::Colour(100, 220, 120);
    inline const auto Band3Accent = juce::Colour(80, 180, 255);
    inline const auto HighCutAccent = juce::Colour(180, 100, 255);

    // Text
    inline const auto TitleText = juce::Colour(220, 225, 235);
    inline const auto SubtleText = juce::Colour(120, 130, 145);
    inline const auto LabelText = juce::Colour(140, 150, 165);

    inline const juce::String FontFamily = "Exo 2";
}