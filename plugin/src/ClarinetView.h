// SPDX-License-Identifier: Apache-2.0
// Draws a Boehm-system B-flat clarinet from clar::clarinetKeyLayout() and highlights pressed levers (D-016).
#pragma once
#include "clar/Keys.h"
#include <juce_gui_basics/juce_gui_basics.h>

class ClarinetView final : public juce::Component {
public:
    ClarinetView();
    void paint(juce::Graphics&) override;
    /// Called by the editor's 60 Hz timer; repaints only if the set changed. Stub: ignores input.
    void setHighlightedKeys(const clar::KeySet& keys);
    clar::KeySet highlightedKeys() const noexcept;
private:
    clar::KeySet keys_;
};
