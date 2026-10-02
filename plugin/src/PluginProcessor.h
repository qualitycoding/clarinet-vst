// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "clar/Keys.h"
#include <juce_audio_processors/juce_audio_processors.h>

class ClarinetAudioProcessor final : public juce::AudioProcessor {
public:
    ClarinetAudioProcessor();
    ~ClarinetAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Clarinet"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    /// Lock-free snapshot for the editor (D-010). Stub: empty set.
    clar::KeySet currentKeys() const noexcept;
    juce::AudioProcessorValueTreeState& parameters() noexcept { return apvts_; }

private:
    juce::AudioProcessorValueTreeState apvts_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClarinetAudioProcessor)
};
