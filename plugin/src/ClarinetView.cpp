// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): full drawing in S-014.
#include "ClarinetView.h"
ClarinetView::ClarinetView() = default;
void ClarinetView::paint(juce::Graphics& g) { g.fillAll(juce::Colours::black); }
void ClarinetView::setHighlightedKeys(const clar::KeySet&) {}
clar::KeySet ClarinetView::highlightedKeys() const noexcept { return keys_; }
