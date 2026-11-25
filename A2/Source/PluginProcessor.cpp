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
    dry = 1.0;

    timeIntervalSeconds = 1.0;
    wet = 1.0;
    feedback = 0.5;
    
    // the delay buffer is at most 6 seconds long
    // it should be double the size for zeno, plus a little extra for insurance

    delayBuffer.setSize(2, ((int) rate * 6) + 512);
    delayBuffer.clear();

    // set delayBufferLength to the starting time
    delayBufferLength = (int) (timeIntervalSeconds * rate);
    delayBlockStart = 0;

    panLeft = 0.5;

    time = 0;
    pingpongEnabled = false;
    zenoEnabled = false;

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

    if (!zenoEnabled)
    {
        // Standard delay, recursively adds data to delayBuffer based on input
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            // pointer to input buffer
            float* data = buffer.getWritePointer (channel,0);

            // Get a pointer to write in the delayBuffer
            float* delayData = delayBuffer.getWritePointer(channel,delayBlockStart);
            
            for (int i = 0; i < numSamples; ++i)
          {
                // get point in delay buffer in the "future" to write to
                int futureIndex = ((int) ceil(rate*timeIntervalSeconds) + i) % (delayBufferLength);

                // output is sum of input and current delay buffer sample
                float output = data[i]*dry + delayData[i]*wet;
                float pan;
                if (channel % 2 == 0) pan = panLeft;
                else pan = (1 - panLeft);
                              
                
                // Store future delay buffer sample
                delayData[futureIndex] = data[i] + delayData[i]*feedback;

                // clear current delay buffer value
                //delayData[i] = 0.0f;

                // data[i] = output * pan;
                data[i] = output * pan;
                
            }
                    
        }
        // update starting position in delayBuffer for next block
        delayBlockStart = (delayBlockStart + numSamples) % delayBufferLength;
    }
    else
    {
        // Zeno delay, adds delay repetition at once based on fixed input
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            // pointer to input buffer
            float* data = buffer.getWritePointer (channel,0);

            // Get a pointer to write in the delayBuffer
            float* delayData = delayBuffer.getWritePointer(channel,delayBlockStart);
            
            for (int i = 0; i < numSamples; ++i)
            {
                // Save current input data
                float input = data[i];

                // get point in delay buffer in the "future" to write to
                int futureIndex = ((int) ceil(rate*timeIntervalSeconds) + i) % (delayBufferLength*2);

                // output is sum of input and curren
                //float pan;
                //if (channel % 2 == 0) pan = panLeft;
                //else pan = (1 - panLeft);
                
                // Compute output value and modify buffer
                data[i] = input*dry + delayData[i]*wet;
                
                //delayData[futureIndex] = input*dry + delayData[i]*zenoFeedback;


                // Place additional repetitions in the buffer at decreasing intervals
                for (int j = 0; j < 4; j++)
                {
                    float nextTimeInterval = timeIntervalSeconds + timeIntervalSeconds*(0.5*j);
                    int nextIndex = ((int) ceil(rate*nextTimeInterval) + i) % (delayBufferLength*2);

                    delayData[nextIndex] = input*dry + delayData[futureIndex]*0.75;
                    futureIndex = nextIndex;

                }
               
            }
        }

        // update starting position in delayBuffer for next block
        // Note that the delayBufferLength is doubled in zeno mode.
        delayBlockStart = (delayBlockStart + numSamples) % (delayBufferLength*2);
    }

    // update panning value when pingpong is enabled
    if (pingpongEnabled)
    {
        time += numSamples / rate;
        panLeft = (sin(time) + 1.0f) / 2.0f;
    }

}


void A2StarterAudioProcessor::clearDelayBuffer()
{
    delayBuffer.clear();
    //delayBlockStart = 0;
}

void A2StarterAudioProcessor::changeBufferTime(float newTime)
{
    timeIntervalSeconds = newTime;
    delayBufferLength = (int) (timeIntervalSeconds * rate);
    delayBuffer.clear();
    delayBlockStart = 0;

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
