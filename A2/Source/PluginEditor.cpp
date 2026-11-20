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
    setSize (width, height);
    
    // these define the parameters of our slider object
    // drySlider represents the global dry parameter.
    drySlider.setSliderStyle (juce::Slider::LinearBarVertical);
    drySlider.setRange (0.0, 2.0, 0.05);
    drySlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    drySlider.setPopupDisplayEnabled (true, false, this);
    drySlider.setTextValueSuffix (" Volume");
    drySlider.setValue(1.0);
 
    // this function adds the slider to the editor
    addAndMakeVisible (&drySlider);
    drySlider.addListener (this);


    // setup slider for wet parameter
    wetSlider.setSliderStyle (juce::Slider::LinearBarVertical);
    wetSlider.setRange (0.0, 1.0, 0.05);
    wetSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    wetSlider.setPopupDisplayEnabled (true, false, this);
    wetSlider.setTextValueSuffix (" Wet");
    wetSlider.setValue(1.0);
    // add wet slider to editor
    addAndMakeVisible (&wetSlider);
    wetSlider.addListener (this);

    // setup slider for feedback parameter
    fbSlider.setSliderStyle (juce::Slider::LinearBarVertical);
    fbSlider.setRange (0.0, 0.99, 0.05);
    fbSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    fbSlider.setPopupDisplayEnabled (true, false, this);
    fbSlider.setTextValueSuffix (" Feedback");
    fbSlider.setValue(0.5);
    // add feedback slider to editor
    addAndMakeVisible (&fbSlider);
    fbSlider.addListener (this);

    // setup slider for panning parameter
    panSlider.setSliderStyle (juce::Slider::LinearBarVertical);
    panSlider.setRange (0.0, 1.0, 0.05);
    panSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    panSlider.setPopupDisplayEnabled (true, false, this);
    panSlider.setTextValueSuffix (" Pan");
    panSlider.setValue(0.5);
    // add panning slider to editor
    addAndMakeVisible (&panSlider);
    panSlider.addListener (this);

    // delay time slider
    timeSlider.setSliderStyle (juce::Slider::LinearBarVertical);
    timeSlider.setRange (0.1, 3.0, 0.1);
    timeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    timeSlider.setPopupDisplayEnabled (true, false, this);
    timeSlider.setTextValueSuffix (" Interval");
    timeSlider.setValue(1.5);
    // add delay slider to editor
    addAndMakeVisible (&timeSlider);
    timeSlider.addListener (this);
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
    drySlider.setBounds (40, 30, 20, getHeight() - 60);
    wetSlider.setBounds (100, 30, 20, getHeight() - 60);
    fbSlider.setBounds (160, 30, 20, getHeight() - 60);
    panSlider.setBounds (220, 30, 20, getHeight() - 60);
    timeSlider.setBounds (280, 30, 20, getHeight() - 60);
}

void A2StarterAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &drySlider)
        audioProcessor.dry = drySlider.getValue();
    else if (slider == &wetSlider)
        audioProcessor.wet = wetSlider.getValue();
    else if (slider == &fbSlider)
        audioProcessor.feedback = fbSlider.getValue();
    else if (slider == &panSlider)
        audioProcessor.panLeft = panSlider.getValue();
    else if (slider == &timeSlider)
    {
        audioProcessor.clearDelayBuffer();
        audioProcessor.timeIntervalSeconds = timeSlider.getValue();
    }
}

void A2StarterAudioProcessorEditor::buttonClicked (juce::Button* button)
{

}

