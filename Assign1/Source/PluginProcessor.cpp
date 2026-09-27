/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
A1StarterAudioProcessor::A1StarterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

A1StarterAudioProcessor::~A1StarterAudioProcessor()
{
}

//==============================================================================
const juce::String A1StarterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool A1StarterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool A1StarterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool A1StarterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double A1StarterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int A1StarterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int A1StarterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void A1StarterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String A1StarterAudioProcessor::getProgramName (int index)
{
    return {};
}

void A1StarterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void A1StarterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    notes.clear();                          // [1]
    currentNote = 0;                        // [2]
    lastNoteValue = -1;                     // [3]
    time = 0;                               // [4]
    rate = static_cast<float> (sampleRate); // [5]
    arpSpeed = 0.5;
    std::cout << arpSpeed;
}

void A1StarterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool A1StarterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void A1StarterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
 
    // We use the audio buffer to get timing information
	// -- processBlock runs once for a set of samples at once
	// -- (if the buffer is 512 samples, then the function runs once every 512 samples
    auto numSamples = buffer.getNumSamples();                                                       // [7]

    // get note duration
	// -- Must be casted to integer to allow modulus operation later
	// -- right to left:
	// -- 1.0f - arpSpeed inverts the value; 0.0 is now 1.0 and 1.0 is now 0.0
	// ^ this added to 0.1f for a range of 0.1 - 1.1 (prevents a noteDuration of 0)
	// rate * 0.25f is a baseline duration of one quarter of a second
	// this arithmetic results in a maximum duration of rate/4 * 1.1 and minimum duration of rate/4 * 0.1
	// (at 44.1KHz, max duration of 12,128 samples, and min 1,103 samples)
    auto noteDuration = static_cast<int> (std::ceil (rate * 0.25f * (0.1f + (1.0f - (arpSpeed)))));   // [8]

	// -- 'auto' in cpp causes the compiler to infer the data type
	// here 'metadata' is type juce::MidiBuffer (see function parameter 'midi' defining a list of MidiBuffers)
    for (const auto metadata : midi)                                                                // [9]
    {
        const auto msg = metadata.getMessage();

        if      (msg.isNoteOn())  notes.add (msg.getNoteNumber());
        else if (msg.isNoteOff()) notes.removeValue (msg.getNoteNumber());
    }

	// remove note from buffer once processed
	// this function can modify the buffer directly (pointers in parameters), so we don't need to track and return a modified buffer
	// considering we're making an arpeggiator we need to remove the midi buffer messages and send our own (see addEvent calls below)
	// we're only tracking noteOn / noteOff events to track the pitch (noteNumber)
    midi.clear();                                                                                   // [10]


	// this code runs during the time rollover, processes changes in note data on a fixed timer
    if ((time + numSamples) >= noteDuration)                                                        // [11]
    {
        auto offset = juce::jmax (0, juce::jmin ((int) (noteDuration - time), numSamples - 1));     // [12]

		// -- if there is a note playing, send a noteOff event to cause the note to stop playing
		// -- addEvent's second arg determines where in the buffer the event will be positioned
		// -- the calc for offset guarantees the buffer value is between 0 - (numSamples-1)
		// -- there is some variance by single samples which is trivial
        if (lastNoteValue > 0)                                                                      // [13]
        {
            midi.addEvent (juce::MidiMessage::noteOff (1, lastNoteValue), offset);
            lastNoteValue = -1;
        }

		// if there is another note to play, start playing that one
		// MidiBuffer lists are automatically sorting by ascending pitch
        if (notes.size() > 0)                                                                       // [14]
        {
            currentNote = (currentNote + 1) % notes.size();
            lastNoteValue = notes[currentNote];
			// juce::uint8 is an 8-bit unsigned integer, which is exactly the midi CC range (0-127)
            midi.addEvent (juce::MidiMessage::noteOn  (1, lastNoteValue, (juce::uint8) 127), offset);
        }

    }


	// progress the timer forward at the end of every processBlock call.
	// here time tracks how long a particular note has been playing
	// it rolls over to 0 when crossing the noteDuration
	// ***** this is part of the arpeggiation bug we need to fix,
	// ***** which causes arpeggiation to occur on a global timer and ignores key presses
	// ***** get time information somehow when processing the noteOn event, and add that value to the left operand here?
    time = (time + numSamples) % noteDuration;                                                      // [15]
    }

//==============================================================================
bool A1StarterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* A1StarterAudioProcessor::createEditor()
{
    return new A1StarterAudioProcessorEditor (*this);
}

//==============================================================================
void A1StarterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void A1StarterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new A1StarterAudioProcessor();
}
