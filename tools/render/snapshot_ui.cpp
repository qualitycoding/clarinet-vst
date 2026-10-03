// SPDX-License-Identifier: Apache-2.0
// clar_ui_snapshots — renders the G-004 screenshot bundle (plan/GATES.md): one PNG per standard fingering
// (written 52..96), one per overblown-fingering case (concert 69..82) and one of the full editor.
// Usage (Linux headless): xvfb-run -a build/plugin/clar_ui_snapshots [OUTPUT_DIR]   (default gates/G-004)
#include "ClarinetView.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "clar/Fingering.h"
#include "clar/Pitch.h"
#include <cstdio>
#include <juce_gui_basics/juce_gui_basics.h>
#include <string>

namespace {

std::string fileName(int midi) {
    static const char* const names[] = {"C", "Cs", "D", "Eb", "E", "F", "Fs", "G", "Ab", "A", "Bb", "B"};
    return std::string(names[midi % 12]) + std::to_string(midi / 12 - 1);
}

bool writePng(const juce::File& file, const juce::Image& img) {
    file.deleteFile();
    juce::FileOutputStream out(file);
    if (!out.openedOk()) return false;
    juce::PNGImageFormat png;
    return png.writeImageToStream(img, out);
}

std::string two(int n) { return (n < 10 ? "0" : "") + std::to_string(n); }

} // namespace

int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI juceInit;
    const juce::File dir = juce::File::getCurrentWorkingDirectory().getChildFile(argc > 1 ? argv[1] : "gates/G-004");
    if (!dir.createDirectory()) { std::fprintf(stderr, "cannot create %s\n", dir.getFullPathName().toRawUTF8()); return 1; }

    int failures = 0, written = 0;
    for (int w = clar::kLowestWritten; w <= clar::kHighestWritten; ++w) {
        ClarinetView view;
        view.setBounds(0, 0, 640, 720);
        view.setHighlightedKeys(clar::standardFingering(w).keys);
        view.setNote(w - clar::kTransposition, false);
        const auto name = "written-" + two(w) + "-" + fileName(w) + ".png";
        failures += !writePng(dir.getChildFile(name), view.createComponentSnapshot(view.getLocalBounds(), true, 1.0f));
        ++written;
    }
    for (int n = 69; n <= 82; ++n) {
        const auto res = clar::resolveNote(n, true);
        if (!res) { ++failures; continue; }
        ClarinetView view;
        view.setBounds(0, 0, 640, 720);
        view.setHighlightedKeys(res->fingering.keys);
        view.setNote(n, true);
        const auto name = "overblown-concert-" + two(n) + "-" + fileName(n) + ".png";
        failures += !writePng(dir.getChildFile(name), view.createComponentSnapshot(view.getLocalBounds(), true, 1.0f));
        ++written;
    }
    {   // the full editor with a sounding note
        ClarinetAudioProcessor proc;
        proc.prepareToPlay(48000, 256);
        juce::MidiBuffer midi;
        midi.addEvent(juce::MidiMessage::noteOn(1, 72, static_cast<juce::uint8>(90)), 0);
        juce::AudioBuffer<float> buf(2, 256);
        for (int i = 0; i < 40; ++i) { proc.processBlock(buf, midi); midi.clear(); }
        std::unique_ptr<juce::AudioProcessorEditor> ed(proc.createEditor());
        ed->setSize(900, 600);
        if (auto* e = dynamic_cast<ClarinetAudioProcessorEditor*>(ed.get())) e->refreshFromProcessor();
        failures += !writePng(dir.getChildFile("editor.png"), ed->createComponentSnapshot(ed->getLocalBounds(), true, 1.0f));
        ++written;
    }
    std::printf("wrote %d images to %s, failures %d\n", written, dir.getFullPathName().toRawUTF8(), failures);
    return failures == 0 ? 0 : 1;
}
