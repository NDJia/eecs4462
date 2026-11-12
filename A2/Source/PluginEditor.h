/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class A2StarterAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Slider::Listener, public juce::Button::Listener
{
public:
    A2StarterAudioProcessorEditor (A2StarterAudioProcessor&);
    ~A2StarterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    bool syncDelayLines;
	  int numDelayLines = 1;

private:
    void sliderValueChanged (juce::Slider* slider) override;
	  void buttonClicked (juce::Button* button) override;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    A2StarterAudioProcessor& audioProcessor;

    juce::Slider drySlider; // [1]

    juce::Slider wetSlider;
    juce::Slider panSlider, fbSlider;
	  juce::Slider timeSlider;

	const int width = 400;
	const int height = 400;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (A2StarterAudioProcessorEditor)
};
