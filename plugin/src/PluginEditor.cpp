// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): controls and layout in S-014.
#include "PluginEditor.h"
ClarinetAudioProcessorEditor::ClarinetAudioProcessorEditor(ClarinetAudioProcessor& p)
    : AudioProcessorEditor(p), proc_(p) { addAndMakeVisible(view_); setSize(900, 600); }
ClarinetAudioProcessorEditor::~ClarinetAudioProcessorEditor() { stopTimer(); }
void ClarinetAudioProcessorEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colours::black); }
void ClarinetAudioProcessorEditor::resized() { view_.setBounds(getLocalBounds()); }
void ClarinetAudioProcessorEditor::refreshFromProcessor() {}
