// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "clar/ClarinetVoice.h"
#include "clar/Keys.h"
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

class ClarinetAudioProcessor final : public juce::AudioProcessor {
public:
    ClarinetAudioProcessor();
    ~ClarinetAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock; // keep the double-precision overload visible (-Woverloaded-virtual)

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

    /// Lock-free snapshots for the editor (D-010): levers of the sounding fingering, and the concert note (-1 = silent).
    clar::KeySet currentKeys() const noexcept;
    int currentConcertNote() const noexcept;
    juce::AudioProcessorValueTreeState& parameters() noexcept { return apvts_; }

private:
    clar::VoiceParameters readParameters() const noexcept;
    void applyParameters(const clar::VoiceParameters&);
    void handleMidi(const juce::MidiMessage&) noexcept;

    juce::AudioProcessorValueTreeState apvts_;
    std::atomic<float>* raw_[10] = {};
    clar::ClarinetVoice voice_;
    int maxBlock_ = 0;
    bool prepared_ = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClarinetAudioProcessor)
};
