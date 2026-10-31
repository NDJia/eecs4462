/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class A1StarterAudioProcessor  : public juce::AudioProcessor
{
public:
     float arpSpeed;
    //==============================================================================
    A1StarterAudioProcessor();
    ~A1StarterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Values which are modified by the GUI should be public
    // Determines if passthrough mode should be active
    // This is mode 1 of the assignment)
    bool passThrough; 
    
                      

    // Determines which of modes 2 or 3 in assignment should be active
    // A bool for multi-key arpeggiation behaviour is unnecessary;
    // the behaviour is identical in both modes
    bool chordArp;    
    
    // Ascending/descending toggle
    bool isAscending;
    
    // tracking of octave span
    int octaveSpan;
    
    // Latch mode toggle
    bool latchEnabled;
    
    // enum for chord type
    // MAJ: 0, 4, 7 (Major chord)
    // MIN: 0, 3, 7 (Minor chord)
    // ALT: 0, 3, 4, 6, 8 (Major 9th suspended 4th)
    // (generic term "ALT" used to avoid refactoring as I kept changing my mind on the custom chord)
    enum chordTypes { MAJ, MIN, ALT } chord;

    // "Swing" toggle used for the note duration variation
    bool swing;

private:
    //==============================================================================
    int currentNote, lastNoteValue;
    int time;
    float rate;
    juce::SortedSet<int> notes;

    // tracks which keys are being held down by the user.
    // as there is frequently a distinction between what keys are being held and what keys are being playing,
    // this is used in order to determine the intended chordgen/arpeggio behaviour.
    // int keysHeld;
    juce::SortedSet<int> keysHeld;

    bool makeNoteShort;
    bool keyPressed, keyReleased;
    int numKeysPressed;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (A1StarterAudioProcessor)
    
};
