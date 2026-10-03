// SPDX-License-Identifier: Apache-2.0
#include "PluginEditor.h"
#include "Parameters.h"
#include <algorithm>

namespace {
struct KnobSpec { const char* id; const char* text; };
constexpr KnobSpec kSpecs[9] = {
    {clarplug::param::overblow, "Overblow"},
    {clarplug::param::reedHardness, "Reed"},
    {clarplug::param::brightness, "Brightness"},
    {clarplug::param::breathNoise, "Breath noise"},
    {clarplug::param::vibratoRate, "Vib. rate"},
    {clarplug::param::vibratoDepth, "Vib. depth"},
    {clarplug::param::portamento, "Portamento"},
    {clarplug::param::tuning, "Tuning A4"},
    {clarplug::param::outputGain, "Output"},
};
} // namespace

ClarinetAudioProcessorEditor::ClarinetAudioProcessorEditor(ClarinetAudioProcessor& p)
    : AudioProcessorEditor(p), proc_(p) {
    addAndMakeVisible(view_);

    title_.setText("Clarinet", juce::dontSendNotification);
    title_.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    title_.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(title_);

    for (std::size_t i = 0; i < knobs_.size(); ++i) {
        auto& k = knobs_[i];
        k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
        k.slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff2b134));
        addAndMakeVisible(k.slider);
        k.label.setText(kSpecs[i].text, juce::dontSendNotification);
        k.label.setJustificationType(juce::Justification::centred);
        k.label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
        addAndMakeVisible(k.label);
        k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters(), kSpecs[i].id, k.slider);
    }
    overblownToggle_.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible(overblownToggle_);
    overblownAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        p.parameters(), clarplug::param::overblownFingering, overblownToggle_);

    setResizeLimits(675, 450, 1800, 1200);
    getConstrainer()->setFixedAspectRatio(1.5);
    setResizable(true, true);
    setSize(900, 600);
    startTimerHz(60);
}

ClarinetAudioProcessorEditor::~ClarinetAudioProcessorEditor() { stopTimer(); }

void ClarinetAudioProcessorEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colour(0xff1b1e23)); }

void ClarinetAudioProcessorEditor::resized() {
    auto area = getLocalBounds();
    view_.setBounds(area.removeFromLeft(static_cast<int>(area.getWidth() * 0.6)));
    area.reduce(12, 12);
    title_.setBounds(area.removeFromTop(36));
    overblownToggle_.setBounds(area.removeFromTop(34));
    area.removeFromTop(8);
    const int cellW = area.getWidth() / 3, cellH = area.getHeight() / 3;
    for (std::size_t i = 0; i < knobs_.size(); ++i) {
        const int col = static_cast<int>(i % 3), row = static_cast<int>(i / 3);
        auto cell = juce::Rectangle<int>(area.getX() + col * cellW, area.getY() + row * cellH, cellW, cellH).reduced(4);
        knobs_[i].label.setBounds(cell.removeFromTop(18));
        knobs_[i].slider.setBounds(cell);
    }
}

void ClarinetAudioProcessorEditor::refreshFromProcessor() {
    view_.setHighlightedKeys(proc_.currentKeys());
    const bool overblown = proc_.parameters().getRawParameterValue(clarplug::param::overblownFingering)->load() >= 0.5f;
    view_.setNote(proc_.currentConcertNote(), overblown);
}
