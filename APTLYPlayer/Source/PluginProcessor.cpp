#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioFilePlayerProcessor::AudioFilePlayerProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // wav, aiff, flac, ogg (if enabled) and, depending on platform / build flags, mp3.
    formatManager.registerBasicFormats();
    readAheadThread.startThread();
}

AudioFilePlayerProcessor::~AudioFilePlayerProcessor()
{
    transport.setSource (nullptr);
    readAheadThread.stopThread (2000);
}

//==============================================================================
void AudioFilePlayerProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    transport.prepareToPlay (samplesPerBlock, sampleRate);
}

void AudioFilePlayerProcessor::releaseResources()
{
    transport.releaseResources();
}

bool AudioFilePlayerProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Source only: no inputs, mono or stereo output.
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled())
        return false;

    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void AudioFilePlayerProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Writes silence when stopped or when no file is loaded.
    juce::AudioSourceChannelInfo info (&buffer, 0, buffer.getNumSamples());
    transport.getNextAudioBlock (info);
}

//==============================================================================
bool AudioFilePlayerProcessor::loadFile (const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));

    if (reader == nullptr)
        return false;

    const double fileSampleRate = reader->sampleRate;

    auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader.release(), true);
    newSource->setLooping (looping.load());

    const juce::ScopedLock sl (fileLock);

    // Detach the old source from the transport before destroying it.
    transport.stop();
    transport.setSource (nullptr);
    readerSource = std::move (newSource);

    // Buffer ahead on a background thread so the audio thread never touches the disk.
    transport.setSource (readerSource.get(), 32768, &readAheadThread, fileSampleRate);
    transport.setPosition (0.0);

    loadedFile = file;
    return true;
}

void AudioFilePlayerProcessor::play()
{
    if (! hasFileLoaded())
        return;

    if (transport.hasStreamFinished())
        transport.setPosition (0.0);

    transport.start();
}

void AudioFilePlayerProcessor::stop()
{
    transport.stop();
}

void AudioFilePlayerProcessor::rewind()
{
    transport.setPosition (0.0);
}

void AudioFilePlayerProcessor::setLooping (bool shouldLoop)
{
    looping = shouldLoop;

    const juce::ScopedLock sl (fileLock);

    if (readerSource != nullptr)
        readerSource->setLooping (shouldLoop);
}

bool AudioFilePlayerProcessor::hasFileLoaded() const
{
    const juce::ScopedLock sl (fileLock);
    return readerSource != nullptr;
}

juce::File AudioFilePlayerProcessor::getLoadedFile() const
{
    const juce::ScopedLock sl (fileLock);
    return loadedFile;
}

//==============================================================================
juce::AudioProcessorEditor* AudioFilePlayerProcessor::createEditor()
{
    return new AudioFilePlayerEditor (*this);
}

//==============================================================================
void AudioFilePlayerProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml ("AudioFilePlayerState");
    xml.setAttribute ("file", getLoadedFile().getFullPathName());
    xml.setAttribute ("looping", looping.load());
    copyXmlToBinary (xml, destData);
}

void AudioFilePlayerProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName ("AudioFilePlayerState"))
        {
            setLooping (xml->getBoolAttribute ("looping", false));

            const juce::File file (xml->getStringAttribute ("file"));

            if (file.existsAsFile())
                loadFile (file);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioFilePlayerProcessor();
}
