/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "math.h"

//==============================================================================
A2StarterAudioProcessor::A2StarterAudioProcessor()
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

A2StarterAudioProcessor::~A2StarterAudioProcessor()
{
}

//==============================================================================
const juce::String A2StarterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool A2StarterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool A2StarterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool A2StarterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double A2StarterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int A2StarterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int A2StarterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void A2StarterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String A2StarterAudioProcessor::getProgramName (int index)
{
    return {};
}

void A2StarterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void A2StarterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    rate = static_cast<float> (sampleRate);
    volumeBoost = 1.0;

    timeIntervalSeconds = 3.0;
    wet = 0.9;
    feedback = 0.75;
    
    // the delay buffer is at most 3 seconds long
    // it should be double the size for zeno, plus a little extra for insurance
    delayBufferLength = ((int) rate * 6) + 512;

    delayBuffer.setSize(2, delayBufferLength);
    delayBuffer.clear();

    // set delayBufferLength to three seconds
    delayBufferLength = (int) (timeIntervalSeconds * rate);
    delayBufferStart = 0;

    time = 0;

}

void A2StarterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool A2StarterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void A2StarterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    int numSamples = buffer.getNumSamples();
    //DBG(1);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        // pointer to input buffer
        float* data = buffer.getWritePointer (channel,0);

        // Get a pointer to write in the delayBuffer
        float* delayData = delayBuffer.getWritePointer(channel);
        
        // Sample audio processing code. Delete for A2 submission
        for (int i = 0; i < numSamples; ++i)
        {
            // volumeBoost = i*numSamples;
            //if (data[i] > 0) DBG(data[i]);
            //data[i] = data[i]*volumeBoost;
            //if (data[i] > 0) DBG(data[i]);

            // get point in delay buffer in the "future" to write to
            int delayFuture = (delayBufferStart + i + ((int) (rate*timeIntervalSeconds) + 1)) % delayBufferLength;
            // Store input and "past" value at future point of delay buffer
            delayData[delayFuture] = data[i]*volumeBoost*feedback;
            data[i] = (data[i]*volumeBoost + delayData[i]*wet)/2.0f;
        }
                
    }

    delayBufferStart = (delayBufferStart + numSamples) % delayBufferLength;

}

//==============================================================================
bool A2StarterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* A2StarterAudioProcessor::createEditor()
{
    return new A2StarterAudioProcessorEditor (*this);
}

//==============================================================================
void A2StarterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void A2StarterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new A2StarterAudioProcessor();
}
