// SPDX-License-Identifier: Apache-2.0
// Editor (D-016): clarinet picture with lever highlighting on the left, controls on the right.
#pragma once
#include "PluginProcessor.h"
#include "ClarinetView.h"
#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

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

    struct Knob {                       // member order matters: the attachment must be destroyed first
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    ClarinetAudioProcessor& proc_;
    ClarinetView view_;
    std::array<Knob, 9> knobs_;
    juce::ToggleButton overblownToggle_{"Overblown fingering"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> overblownAttachment_;
    juce::Label title_;
};
