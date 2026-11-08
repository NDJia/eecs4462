#include "PluginEditor.h"

//==============================================================================
static juce::String formatTime (double seconds)
{
    seconds = juce::jmax (0.0, seconds);
    const int minutes = (int) (seconds / 60.0);
    return juce::String::formatted ("%d:%04.1f", minutes, seconds - minutes * 60.0);
}

//==============================================================================
AudioFilePlayerEditor::AudioFilePlayerEditor (AudioFilePlayerProcessor& p)
    : AudioProcessorEditor (&p), player (p)
{
    for (auto* c : { static_cast<juce::Component*> (&loadButton),
                     static_cast<juce::Component*> (&playButton),
                     static_cast<juce::Component*> (&stopButton),
                     static_cast<juce::Component*> (&rewindButton),
                     static_cast<juce::Component*> (&loopButton),
                     static_cast<juce::Component*> (&fileLabel),
                     static_cast<juce::Component*> (&timeLabel) })
        addAndMakeVisible (c);

    loadButton.onClick   = [this] { chooseFile(); };
    playButton.onClick   = [this] { player.play();   updateDisplay(); };
    stopButton.onClick   = [this] { player.stop();   updateDisplay(); };
    rewindButton.onClick = [this] { player.rewind(); updateDisplay(); };

    loopButton.setToggleState (player.isLooping(), juce::dontSendNotification);
    loopButton.onClick = [this] { player.setLooping (loopButton.getToggleState()); };

    fileLabel.setJustificationType (juce::Justification::centredLeft);
    timeLabel.setJustificationType (juce::Justification::centredRight);

    setSize (440, 130);
    updateDisplay();
    startTimerHz (20);
}

AudioFilePlayerEditor::~AudioFilePlayerEditor()
{
    stopTimer();
}

//==============================================================================
void AudioFilePlayerEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AudioFilePlayerEditor::resized()
{
    auto area = getLocalBounds().reduced (12);

    auto top = area.removeFromTop (28);
    loadButton.setBounds (top.removeFromLeft (80));
    top.removeFromLeft (8);
    fileLabel.setBounds (top);

    area.removeFromTop (8);
    timeLabel.setBounds (area.removeFromTop (24));

    area.removeFromTop (8);
    auto row = area.removeFromTop (28);
    const int w = (row.getWidth() - 3 * 8) / 4;

    playButton.setBounds   (row.removeFromLeft (w));  row.removeFromLeft (8);
    stopButton.setBounds   (row.removeFromLeft (w));  row.removeFromLeft (8);
    rewindButton.setBounds (row.removeFromLeft (w));  row.removeFromLeft (8);
    loopButton.setBounds   (row);
}

//==============================================================================
void AudioFilePlayerEditor::timerCallback()
{
    updateDisplay();
}

void AudioFilePlayerEditor::updateDisplay()
{
    const bool hasFile  = player.hasFileLoaded();
    const bool playing  = player.isPlaying();

    const auto file = player.getLoadedFile();
    fileLabel.setText (hasFile ? file.getFileName() : "No file loaded", juce::dontSendNotification);

    timeLabel.setText (formatTime (player.getPositionSeconds()) + " / " + formatTime (player.getLengthSeconds()),
                       juce::dontSendNotification);

    playButton.setEnabled (hasFile && ! playing);
    stopButton.setEnabled (playing);
    rewindButton.setEnabled (hasFile);
}

void AudioFilePlayerEditor::chooseFile()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Select an audio file...",
                                                       player.getLoadedFile().getParentDirectory(),
                                                       player.getFormatManager().getWildcardForAllFormats());

    const auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    juce::Component::SafePointer<AudioFilePlayerEditor> safeThis (this);

    fileChooser->launchAsync (flags, [safeThis] (const juce::FileChooser& fc)
    {
        if (safeThis == nullptr)
            return;

        const auto file = fc.getResult();

        if (! file.existsAsFile())
            return;   // cancelled

        if (! safeThis->player.loadFile (file))
            juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon,
                                                    "Couldn't open file",
                                                    "JUCE couldn't decode:\n" + file.getFullPathName());

        safeThis->updateDisplay();
    });
}
