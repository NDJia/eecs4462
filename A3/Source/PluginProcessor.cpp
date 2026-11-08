/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
A3AudioProcessor::A3AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "PARAMETERS", createParameterLayout()) // initialize parameter layout object
																					   // an undo manager object also exists
#endif
{
}

A3AudioProcessor::~A3AudioProcessor()
{
}

//==============================================================================
const juce::String A3AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool A3AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool A3AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool A3AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double A3AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int A3AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int A3AudioProcessor::getCurrentProgram()
{
    return 0;
}

void A3AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String A3AudioProcessor::getProgramName (int index)
{
    return {};
}

void A3AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void A3AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getMainBusNumOutputChannels();
    
    stateVariableFilter.reset();
    fxChain.reset();
    stateVariableFilter.prepare(spec);
    fxChain.prepare(spec);
    updateFX();
}

// updateFX is called in prepareToPlay above, sets all AudioProcessorValueTreeState items
// Makes GUI layout easier, and exposes the parameters to VST host software.
// Refer to createParameterLayout at the bottom of the file
void A3AudioProcessor::updateFX()
{
    int filterChoice = *apvts.getRawParameterValue("FILTERMENU");
    int phaserChoice = *apvts.getRawParameterValue("PHASERMENU");
    float cutoff = *apvts.getRawParameterValue("CUTOFF");
    float phaserRate = *apvts.getRawParameterValue("PHASERRATE");
    float phaserDepth = *apvts.getRawParameterValue("PHASERDEPTH");
    float gain = *apvts.getRawParameterValue("GAIN");
    
    bypassFilter = false;
    if (filterChoice == 1) stateVariableFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    if (filterChoice == 2) stateVariableFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    if (filterChoice == 3) stateVariableFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    if (filterChoice == 4) bypassFilter = true;
    stateVariableFilter.setCutoffFrequency(cutoff);
    
    if (phaserChoice == 1) bypassPhaser = false;
    if (phaserChoice == 2) bypassPhaser = true;

    auto& gainProcessor = fxChain.template get<gainIndex>();
    gainProcessor.setGainLinear (gain);
     
    auto& phaserProcessor = fxChain.template get<phaserIndex>();
    phaserProcessor.setRate (phaserRate);
    phaserProcessor.setDepth (phaserDepth);
}

void A3AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool A3AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void A3AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateFX();
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
   
	// the filter is processed separate from the rest of the FX chain (phaser)
    if (!bypassFilter) stateVariableFilter.process(context);
    if (!bypassPhaser) fxChain.process(context);

}

//==============================================================================
bool A3AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* A3AudioProcessor::createEditor()
{
    return new A3AudioProcessorEditor (*this);
}

//==============================================================================
void A3AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void A3AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new A3AudioProcessor();
}

// Parameters are set and controlled here!
juce::AudioProcessorValueTreeState::ParameterLayout A3AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
 
    layout.add (std::make_unique<juce::AudioParameterFloat> ("CUTOFF", "Cutoff", 20.0f, 20000.0f, 600.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("PHASERRATE", "Rate", 0.0f, 2.0f, 1.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("PHASERDEPTH", "Depth", 0.0f, 1.0f, 0.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("GAIN", "Gain", 0.0f, 2.0f, 1.0f));
    layout.add (std::make_unique<juce::AudioParameterInt> ("FILTERMENU", "Filter Menu", 1, 4, 4));
    layout.add (std::make_unique<juce::AudioParameterInt> ("PHASERMENU", "Phaser Menu", 1, 2, 2));
 
    return layout;
}
