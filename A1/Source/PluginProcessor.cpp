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
	// default mode will be mode 2, (mixed)
	// mode 1 (passthrough): passThrough is true
	// mode 2 (mixed): chordArp is false
	// mode 3 (chord + arp): chordArp is true
	// Chord gen on only one key is always the behaviour
	// arpeggiate on multiple keys pressed is always the behaviour and doesn't require a flag
	passThrough = false;
	chordArp = true;

	// Latch mode begins disabled
	latchEnabled = false;

	// Begins ascending
	isAscending = true;

	// Octave span range is 1-3
	octaveSpan = 1;
	chord = MAJ;

	//keysHeld = 0;
	keysHeld.clear();

	swing = false;
	makeNoteShort = false;

	numKeysPressed = 0;
	keyPressed = false, keyReleased = false;

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
	if (passThrough) return;
 
	// processBlock runs once for a set of samples at once
	// if the buffer is 512 samples, then the function runs once every 512 samples
    auto numSamples = buffer.getNumSamples();                                                       // [7]

    // get note duration
	// -- Must be casted to integer to allow modulus operation later
	// -- right to left:
	// -- 1.0f - arpSpeed inverts the value; 0.0 is now 1.0 and 1.0 is now 0.0
	// ^ this added to 0.1f for a range of 0.1 - 1.1 (prevents a noteDuration of 0)
	// rate * 0.25f is a baseline duration of one quarter of a second
	// this arithmetic results in a maximum duration of rate/4 * 1.1 and minimum duration of rate/4 * 0.1
	// (at 44.1KHz, max duration of 12,128 samples, and min 1,103 samples)
    auto noteDuration = static_cast<int> (std::ceil (rate * 1.0f * (0.1f + (1.0f - (arpSpeed)))));   // [8]
	
	// If "swing" parameter is active, every other note's duration is halved
	if (swing && makeNoteShort) noteDuration *= 0.5;

	// calculate offset value that determines where in the midi buffer messages should be placed
	auto offset = juce::jmax (0, juce::jmin ((int) (noteDuration - time), numSamples - 1));     // [12]
	// -- 'auto' in cpp causes the compiler to infer the data type

	keyPressed = false, keyReleased = false;
	//numKeysPressed = 0;
	
    for (const auto metadata : midi)                                                                // [9]
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
		{
			keyPressed = true;
			notes.add (msg.getNoteNumber());
			keysHeld.add(msg.getNoteNumber());
			numKeysPressed++;
		}
		else if (msg.isNoteOff())
		{
			keyReleased = true;
			notes.remove(msg.getNoteNumber());
			keysHeld.remove(msg.getNoteNumber());
			numKeysPressed--;
		}
    }
	
	// clear the MidiBuffer
    midi.clear();                                                                                  // [10]

	// store number of keys currently held down
	//int numKeysPressed = keysHeld.size();
	
	// if (keyPressed || keyReleased)
	// {
	// 	notes.removeValuesNotIn(keysHeld);
	// }

	int numKeysHeld = keysHeld.size();

	// if no keys are being held down and latch mode isn't on, then stop everything immediately
	if (numKeysHeld == 0 && numKeysPressed == 1)
	{ 
		midi.addEvent(juce::MidiMessage::allNotesOff(1), offset);
		notes.clear();
	}
	
	if (numKeysPressed > 0)
	{
		time = -1.0f;
		currentNote = -1;
	}

	// ===== Chord gen =====
	// If there is only one note, populate notes set with a chord
	if (numKeysPressed == 1 && notes.size() <= 1)
	{
		const auto tonic = notes[0];
		notes.clear();
		for (int i = 0; i < octaveSpan; i++)
		{
			int octave = i * 12;

			if (!isAscending) octave *= -1;
			
			switch(chord)
			{
				case MAJ:
					notes.add(tonic + octave);
					notes.add(tonic + 4 + octave);
					notes.add(tonic + 7 + octave);
					break;
				case MIN:
					notes.add(tonic + octave);
					notes.add(tonic + 3 + octave);
					notes.add(tonic + 7 + octave);
					break;
				default: // case ALT, which is Dom9
					notes.add(tonic + octave);
					notes.add(tonic + 4 + octave);
					notes.add(tonic + 7 + octave);
					notes.add(tonic + 10 + octave);
					notes.add(tonic + 14 + octave);
					break;

			}
		}

		// then play the notes immediately as a chord, if not in chord-arp mode
		if (!chordArp)
		{
			midi.addEvent(juce::MidiMessage::allNotesOff(1), offset);
			for (const auto n : notes)
			{
				midi.addEvent(juce::MidiMessage::noteOn(1, n, (juce::uint8) 127), offset);
			}
		}
	}
	else if (numKeysPressed > 1)
	{
		notes.removeValuesNotIn(keysHeld);
	}

	// If more than one key is held or chordArp is enabled, then midi messages should be sent on a timer for arpeggiation
	// this code runs during the time rollover, processes changes in note data on a fixed timer
	if ((chordArp || numKeysPressed > 1) && ((time + numSamples) >= noteDuration || time < 0))                                                       // [11]
	{

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

			//Iterates forward through notes
			if (isAscending) lastNoteValue = notes[currentNote];
			// Iterates backward through notes
			else lastNoteValue = notes[notes.size()-1 - currentNote];

			// juce::uint8 is  an 8-bit unsigned integer, which is exactly the midi CC range
			midi.addEvent (juce::MidiMessage::noteOn  (1, lastNoteValue, (juce::uint8) 127), offset);
		}


		// if the variant note timing is active, flip timing flag here
		if (swing) makeNoteShort = !makeNoteShort;
	}


	// progress the timer forward at the end of every processBlock call.
	// here time tracks how long a particular note has been playing
	// it rolls over to 0 when crossing the noteDuration
   	time = (time + numSamples) % noteDuration;                                                      // [15]


	// reset midi input flags
	//midiInputReceivedThisBlock = false;
	

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
