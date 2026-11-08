#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

//==============================================================================
/**
    A source plugin that plays an audio file (wav, aiff, flac, mp3, ...) on its
    output bus, so it can be wired in front of the plugin you are developing in
    AudioPluginHost (or any other host).
*/
class AudioFilePlayerProcessor final : public juce::AudioProcessor
{
public:
    AudioFilePlayerProcessor();
    ~AudioFilePlayerProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                     { return true; }

    //==============================================================================
    const juce::String getName() const override         { return "Audio File Player"; }
    bool acceptsMidi() const override                   { return false; }
    bool producesMidi() const override                  { return false; }
    bool isMidiEffect() const override                  { return false; }
    double getTailLengthSeconds() const override        { return 0.0; }

    int getNumPrograms() override                       { return 1; }
    int getCurrentProgram() override                    { return 0; }
    void setCurrentProgram (int) override               {}
    const juce::String getProgramName (int) override    { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Player API (call from the message thread)
    bool loadFile (const juce::File& file);   // returns false if it couldn't be read
    void play();
    void stop();                              // pauses at the current position
    void rewind();                            // jumps back to the start
    void setLooping (bool shouldLoop);

    bool isLooping() const noexcept           { return looping.load(); }
    bool isPlaying() const                    { return transport.isPlaying(); }
    double getPositionSeconds() const         { return transport.getCurrentPosition(); }
    double getLengthSeconds() const           { return transport.getLengthInSeconds(); }
    bool hasFileLoaded() const;
    juce::File getLoadedFile() const;
    const juce::AudioFormatManager& getFormatManager() const noexcept { return formatManager; }

private:
    //==============================================================================
    // Declaration order matters: members are destroyed in reverse, so the
    // transport goes first, then the reader source, then the read-ahead thread.
    juce::AudioFormatManager formatManager;
    juce::TimeSliceThread readAheadThread { "Audio file read-ahead" };

    mutable juce::CriticalSection fileLock;   // guards readerSource / loadedFile
    juce::File loadedFile;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transport;
    std::atomic<bool> looping { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFilePlayerProcessor)
};
