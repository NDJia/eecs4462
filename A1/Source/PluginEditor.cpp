/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

// -- This file contains data for programming the GUI.
// -- done in a similar way to graphics frameworks I know, it looks like

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
A1StarterAudioProcessorEditor::A1StarterAudioProcessorEditor (A1StarterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (200, 400);
    
    // these define the parameters of our slider object
    arpSlider.setSliderStyle (juce::Slider::LinearBarVertical);
    arpSlider.setRange (0.0, 1.0, 0.05);
    arpSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    arpSlider.setPopupDisplayEnabled (true, false, this);
    arpSlider.setTextValueSuffix (" Speed");
    arpSlider.setValue(0.5);
 
    // this function adds the slider to the editor
    addAndMakeVisible (&arpSlider);
    arpSlider.addListener (this);
}

A1StarterAudioProcessorEditor::~A1StarterAudioProcessorEditor()
{
}

//==============================================================================
// "paint" called once per display frame
void A1StarterAudioProcessorEditor::paint (juce::Graphics& g)
{
   // fill the whole window white
    g.fillAll (juce::Colours::white);
 
    // set the current drawing colour to black
    g.setColour (juce::Colours::black);
 
    // set the font size and draw text to the screen
    g.setFont (15.0f);
 
    g.drawFittedText ("Arpeggiator", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
}

void A1StarterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    // sets the position and size of the slider with arguments (x, y, width, height)
    arpSlider.setBounds (40, 30, 20, getHeight() - 60);
}

// -- interactions with GUI elements are processed in these callback functions
void A1StarterAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
	// audioProcessor object is data passed to PluginProcessor.cpp
	// recall that arpSpeed is a variable in the PluginProcessor file.
    audioProcessor.arpSpeed = arpSlider.getValue();
}
