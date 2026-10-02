// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-029 (integration, plugin level) — SC-1 parameters/state, SC-2 UI highlighting, SC-3 audible output.
#define CATCH_CONFIG_RUNNER
#include "Parameters.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "clar/Fingering.h"
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <memory>

int main(int argc, char* argv[]) {
    juce::ScopedJuceInitialiser_GUI juce;
    return Catch::Session().run(argc, argv);
}

static void render(ClarinetAudioProcessor& p, juce::MidiBuffer midi, int blocks, double* rms = nullptr) {
    juce::AudioBuffer<float> b(2, 256);
    double acc = 0;
    for (int i = 0; i < blocks; ++i) {
        p.processBlock(b, midi);
        midi.clear();
        acc += b.getRMSLevel(0, 0, 256);
    }
    if (rms) *rms = acc / blocks;
}

TEST_CASE("T-029 all D-013 parameters exist with stable IDs", "[T-029][plugin]") {
    ClarinetAudioProcessor p;
    using namespace clarplug::param;
    for (const char* id : {overblow, harmonic, reedHardness, brightness, breathNoise, vibratoRate,
                           vibratoDepth, portamento, tuning, outputGain})
        CHECK(p.parameters().getParameter(id) != nullptr);
    CHECK(p.getParameters().size() == clarplug::param::count);
}

TEST_CASE("T-029 MIDI note produces sound and the editor highlights its fingering", "[T-029][plugin]") {
    ClarinetAudioProcessor p;
    p.prepareToPlay(48000, 256);
    juce::MidiBuffer m; m.addEvent(juce::MidiMessage::noteOn(1, 61, (juce::uint8) 90), 0);
    double rms = 0;
    render(p, m, 100, &rms);
    CHECK(rms > 1e-3);
    CHECK(p.currentKeys() == clar::standardFingering(70).keys);
    std::unique_ptr<juce::AudioProcessorEditor> ed(p.createEditor());
    auto* se = dynamic_cast<ClarinetAudioProcessorEditor*>(ed.get());
    REQUIRE(se != nullptr);
    se->refreshFromProcessor();
    CHECK(se->clarinetView().highlightedKeys() == clar::standardFingering(70).keys);
}

TEST_CASE("T-029 state survives a save/load round trip", "[T-029][plugin]") {
    ClarinetAudioProcessor a, b;
    a.parameters().getParameter(clarplug::param::overblow)->setValueNotifyingHost(0.75f);
    a.parameters().getParameter(clarplug::param::harmonic)->setValueNotifyingHost(1.0f);
    juce::MemoryBlock mb; a.getStateInformation(mb);
    REQUIRE(mb.getSize() > 0);
    b.setStateInformation(mb.getData(), static_cast<int>(mb.getSize()));
    CHECK(b.parameters().getParameter(clarplug::param::overblow)->getValue() == Catch::Approx(0.75f).margin(1e-6));
    CHECK(b.parameters().getParameter(clarplug::param::harmonic)->getValue() == 1.0f);
    b.setStateInformation("garbage", 7); // must not crash; keeps previous values
    CHECK(b.parameters().getParameter(clarplug::param::overblow)->getValue() == Catch::Approx(0.75f).margin(1e-6));
}
