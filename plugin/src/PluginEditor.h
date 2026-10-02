// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "PluginProcessor.h"
#include "ClarinetView.h"
#include <juce_audio_processors/juce_audio_processors.h>

class ClarinetAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit ClarinetAudioProcessorEditor(ClarinetAudioProcessor&);
    ~ClarinetAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    /// Pulls currentKeys() from the processor into the view (also what the timer does).
    void refreshFromProcessor();
    ClarinetView& clarinetView() noexcept { return view_; }
private:
    void timerCallback() override { refreshFromProcessor(); }
    ClarinetAudioProcessor& proc_;
    ClarinetView view_;
};
