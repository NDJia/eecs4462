/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
A3AudioProcessorEditor::A3AudioProcessorEditor (A3AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 200);
    
    filterMenu.setJustificationType(juce::Justification::centred);
    filterMenu.addItem("Filter: Low Pass", 1);
    filterMenu.addItem("Filter: Band Pass", 2);
    filterMenu.addItem("Filter: High Pass", 3);
    filterMenu.addItem("Filter: Off", 4);
    addAndMakeVisible(&filterMenu);

    phaserMenu.setJustificationType(juce::Justification::centred);
    phaserMenu.addItem("Phaser: On", 1);
    phaserMenu.addItem("Phaser: Off", 2);
    addAndMakeVisible(&phaserMenu);
    
    cutOffSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    cutOffSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    cutOffSlider.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(&cutOffSlider);
    
    rateSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    rateSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rateSlider.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(&rateSlider);
    
    depthSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    depthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    depthSlider.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(&depthSlider);
    
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(&gainSlider);
    
    cutOffValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "CUTOFF", cutOffSlider);
    rateValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "PHASERRATE", rateSlider);
    depthValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "PHASERDEPTH", depthSlider);
    gainValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "GAIN", gainSlider);
    filterMenuValue = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "FILTERMENU", filterMenu);
    phaserMenuValue = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "PHASERMENU", phaserMenu);
    
}

A3AudioProcessorEditor::~A3AudioProcessorEditor()
{
}

//==============================================================================
void A3AudioProcessorEditor::paint (juce::Graphics& g)
{
    getLookAndFeel().setColour (juce::Slider::thumbColourId, juce::Colours::greenyellow);
    juce::Rectangle<int> area = getLocalBounds().reduced(5);
    juce::Rectangle<int> titleArea = area.removeFromTop (20);
    //int width = area.getWidth();
    g.fillAll (juce::Colours::grey);
    g.setColour(juce::Colours::white);
    g.drawText("A3 Starter", titleArea, juce::Justification::centredTop);
    
    juce::Rectangle <int> innerArea = getLocalBounds().reduced(25);//(25, 25, 350, 150);
    g.setColour(juce::Colours::yellowgreen);
    g.drawRect(innerArea, 1.0f);
    
    int innerWidth = innerArea.getWidth();
    g.drawText("Cutoff", innerArea.getX(), 80, innerWidth / 4, 25, juce::Justification::centred);
    g.drawText("Rate", innerArea.getX() + innerWidth / 4, 80, innerWidth / 4, 25, juce::Justification::centred);
    g.drawText("Depth", innerArea.getX() + innerWidth / 2, 80, innerWidth / 4, 25, juce::Justification::centred);
    g.drawText("Gain", innerArea.getX() + 3 * innerWidth / 4, 80, innerWidth / 4, 25, juce::Justification::centred);
    
}

void A3AudioProcessorEditor::resized()
{
    juce::Rectangle<int> area = getLocalBounds().reduced(40);
    juce::Rectangle<int> menus = area.removeFromTop(20);
    int width = menus.getWidth();
    filterMenu.setBounds(menus.removeFromLeft(width / 2 - 5));
    phaserMenu.setBounds(menus.removeFromRight(width / 2 - 5));
    cutOffSlider.setBounds (35 , 90, width / 4 - 10, area.getHeight() - 10);
    rateSlider.setBounds (120, 90, width / 4 - 10, area.getHeight() - 10);
    depthSlider.setBounds (209, 90, width / 4 - 10, area.getHeight() - 10);
    gainSlider.setBounds (295, 90, width / 4 - 10, area.getHeight() - 10);
}
