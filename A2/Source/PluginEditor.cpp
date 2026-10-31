/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
A2StarterAudioProcessorEditor::A2StarterAudioProcessorEditor (A2StarterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (200, 400);
    
    // these define the parameters of our slider object
    volumeSlider.setSliderStyle (juce::Slider::LinearBarVertical);
    volumeSlider.setRange (0.0, 2.0, 0.05);
    volumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    volumeSlider.setPopupDisplayEnabled (true, false, this);
    volumeSlider.setTextValueSuffix (" Volume");
    volumeSlider.setValue(1.0);
 
    // this function adds the slider to the editor
    addAndMakeVisible (&volumeSlider);
    volumeSlider.addListener (this);
}

A2StarterAudioProcessorEditor::~A2StarterAudioProcessorEditor()
{
}

//==============================================================================
void A2StarterAudioProcessorEditor::paint (juce::Graphics& g)
{
   // fill the whole window white
    g.fillAll (juce::Colours::white);
 
    // set the current drawing colour to black
    g.setColour (juce::Colours::black);
 
    // set the font size and draw text to the screen
    g.setFont (15.0f);
 
    g.drawFittedText ("A2 Delay", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
}

void A2StarterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    // sets the position and size of the slider with arguments (x, y, width, height)
    volumeSlider.setBounds (40, 30, 20, getHeight() - 60);
}

void A2StarterAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    audioProcessor.volumeBoost = volumeSlider.getValue();
}
