// SPDX-License-Identifier: Apache-2.0
// Draws a Boehm-system B-flat clarinet from clar::clarinetKeyLayout() and highlights pressed levers (D-016).
#pragma once
#include "clar/Keys.h"
#include <juce_gui_basics/juce_gui_basics.h>

class ClarinetView final : public juce::Component {
public:
    ClarinetView();
    void paint(juce::Graphics&) override;
    /// Called by the editor's 60 Hz timer; repaints only if the set changed.
    void setHighlightedKeys(const clar::KeySet& keys);
    clar::KeySet highlightedKeys() const noexcept;
    /// Caption data: the sounding concert MIDI note (< 0 = silent) and whether the Overblown-fingering mode is on.
    void setNote(int concertMidi, bool overblownFingeringMode);
    /// "Written Eb4 | Concert Db4 | clarion" style text shown under the instrument; "-" when silent.
    juce::String caption() const;

private:
    clar::KeySet keys_;
    int note_ = -1;
    bool overblownMode_ = false;
};
