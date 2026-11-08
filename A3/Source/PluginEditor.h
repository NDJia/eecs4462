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
class A3AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    A3AudioProcessorEditor (A3AudioProcessor&);
    ~A3AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    A3AudioProcessor& audioProcessor;
    
    juce::Slider cutOffSlider;
    juce::Slider rateSlider;
    juce::Slider depthSlider;
    juce::Slider gainSlider;
    juce::ComboBox filterMenu;
    juce::ComboBox phaserMenu;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutOffValue;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateValue;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthValue;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainValue;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterMenuValue;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> phaserMenuValue;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (A3AudioProcessorEditor)
};
