#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioFilePlayerEditor final : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    explicit AudioFilePlayerEditor (AudioFilePlayerProcessor&);
    ~AudioFilePlayerEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void chooseFile();
    void updateDisplay();

    AudioFilePlayerProcessor& player;

    juce::TextButton loadButton   { "Load..." };
    juce::TextButton playButton   { "Play" };
    juce::TextButton stopButton   { "Stop" };
    juce::TextButton rewindButton { "Rewind" };
    juce::ToggleButton loopButton { "Loop" };

    juce::Label fileLabel;
    juce::Label timeLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;   // must outlive the async dialog

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFilePlayerEditor)
};
