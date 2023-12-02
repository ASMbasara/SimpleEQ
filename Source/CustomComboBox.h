#pragma once
#include <JuceHeader.h>
#include "Looks.h"
#include "PluginProcessor.h"

class CustomComboBox : public juce::ComboBox
{
public:
	CustomComboBox(juce::String& componentName, juce::String& suffix) :
		juce::ComboBox(componentName), label(suffix) {
	}
	~CustomComboBox() {
	}

private:
	juce::String label;
};
