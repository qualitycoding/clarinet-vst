// SPDX-License-Identifier: Apache-2.0
// Programmatic drawing of the clarinet (no bitmaps, A-020) and lever highlighting (D-016).
#include "ClarinetView.h"
#include "clar/Fingering.h"
#include "clar/KeyLayout.h"
#include "clar/Pitch.h"
#include <algorithm>
#include <cmath>

namespace {

const juce::Colour kBackground{0xff14161a};
const juce::Colour kBody{0xff241f1c};
const juce::Colour kBodyEdge{0xff3d342e};
const juce::Colour kMetal{0xff8a8f98};
const juce::Colour kKeyIdle{0xff30343a};
const juce::Colour kKeyHole{0xff0b0b0c};
const juce::Colour kPressed{0xfff2b134};

juce::String noteName(int midi) {
    static const char* const names[] = {"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"};
    return juce::String(names[midi % 12]) + juce::String(midi / 12 - 1);
}

} // namespace

ClarinetView::ClarinetView() { setOpaque(true); }

void ClarinetView::setHighlightedKeys(const clar::KeySet& keys) {
    if (keys == keys_) return;
    keys_ = keys;
    repaint();
}
clar::KeySet ClarinetView::highlightedKeys() const noexcept { return keys_; }

void ClarinetView::setNote(int concertMidi, bool overblownFingeringMode) {
    if (concertMidi == note_ && overblownFingeringMode == overblownMode_) return;
    note_ = concertMidi;
    overblownMode_ = overblownFingeringMode;
    repaint();
}

juce::String ClarinetView::caption() const {
    const auto res = clar::resolveNote(note_, overblownMode_);
    if (!res) return "-";
    const int written = res->fingering.writtenMidi;
    const int sounding = res->concertMidi;
    juce::String reg;
    if (res->overblownFingering) reg = "overblown twelfth";
    else if (res->fingering.targetRegister == 2) reg = "clarion";
    else if (res->fingering.targetRegister == 3) reg = "altissimo";
    else reg = (res->fingering.writtenMidi >= 67) ? "throat" : "chalumeau";
    // In overblown-fingering mode the fingering shown is the chalumeau note a twelfth below the sounding one.
    return "Fingering " + noteName(written) + "  |  Sounds " + noteName(sounding) + " (concert)  |  " + reg;
}

void ClarinetView::paint(juce::Graphics& g) {
    g.fillAll(kBackground);
    const auto area = getLocalBounds().toFloat().reduced(8.0f);
    const float captionH = 34.0f;
    const auto drawArea = area.withTrimmedBottom(captionH);
    const float side = std::min(drawArea.getWidth(), drawArea.getHeight());
    if (side < 40.0f) return;
    const auto box = juce::Rectangle<float>(side, side).withCentre(drawArea.getCentre());
    auto X = [&](float nx) { return box.getX() + nx * side; };
    auto Y = [&](float ny) { return box.getY() + ny * side; };
    auto S = [&](float n) { return n * side; };

    // ---- instrument body (upright: mouthpiece at the top, bell at the bottom) ----
    const float cx = 0.45f;
    auto tube = [&](float y0, float y1, float halfW) {
        g.setColour(kBody);
        g.fillRect(X(cx - halfW), Y(y0), S(2 * halfW), S(y1 - y0));
        g.setColour(kBodyEdge);
        g.drawRect(X(cx - halfW), Y(y0), S(2 * halfW), S(y1 - y0), 1.0f);
    };
    // mouthpiece (tapered), barrel, upper joint, lower joint, bell
    juce::Path mouth;
    mouth.addTriangle(X(cx - 0.020f), Y(0.075f), X(cx + 0.020f), Y(0.075f), X(cx), Y(0.018f));
    g.setColour(juce::Colour(0xff17120f));
    g.fillPath(mouth);
    g.setColour(kBodyEdge);
    g.strokePath(mouth, juce::PathStrokeType(1.0f));
    g.setColour(kMetal.withAlpha(0.7f));                       // ligature
    g.fillRect(X(cx - 0.030f), Y(0.060f), S(0.060f), S(0.010f));
    tube(0.075f, 0.135f, 0.040f);                              // barrel (slightly wider)
    tube(0.135f, 0.485f, 0.034f);                              // upper joint
    tube(0.485f, 0.805f, 0.034f);                              // lower joint
    juce::Path bell;                                           // flared bell
    bell.startNewSubPath(X(cx - 0.034f), Y(0.805f));
    bell.lineTo(X(cx + 0.034f), Y(0.805f));
    bell.quadraticTo(X(cx + 0.040f), Y(0.900f), X(cx + 0.095f), Y(0.960f));
    bell.lineTo(X(cx - 0.095f), Y(0.960f));
    bell.quadraticTo(X(cx - 0.040f), Y(0.900f), X(cx - 0.034f), Y(0.805f));
    bell.closeSubPath();
    g.setColour(kBody);
    g.fillPath(bell);
    g.setColour(kBodyEdge);
    g.strokePath(bell, juce::PathStrokeType(1.0f));
    g.setColour(kMetal.withAlpha(0.8f));                       // joint rings
    for (float y : {0.135f, 0.485f, 0.805f}) g.fillRect(X(cx - 0.040f), Y(y) - 1.5f, S(0.080f), 3.0f);

    // ---- rear-view inset for the thumb hole and register key ----
    const float ix = 0.055f, iy = 0.135f, iw = 0.165f, ih = 0.290f;
    g.setColour(juce::Colour(0xff1d2026));
    g.fillRoundedRectangle(X(ix), Y(iy), S(iw), S(ih), 6.0f);
    g.setColour(kMetal.withAlpha(0.5f));
    g.drawRoundedRectangle(X(ix), Y(iy), S(iw), S(ih), 6.0f, 1.0f);
    g.setFont(juce::FontOptions(std::max(9.0f, S(0.026f))));
    g.drawText("rear view", juce::Rectangle<float>(X(ix), Y(iy + ih - 0.045f), S(iw), S(0.04f)), juce::Justification::centred);
    const float dash[] = {3.0f, 3.0f};
    g.drawDashedLine(juce::Line<float>(X(ix + iw), Y(0.305f), X(cx - 0.034f), Y(0.305f)), dash, 2, 1.0f);

    // ---- levers: pressed = amber with glow, idle = dark metal ----
    const auto& layout = clar::clarinetKeyLayout();
    for (const auto& k : layout) {
        const bool pressed = keys_.test(clar::index(k.key));
        const auto r = juce::Rectangle<float>(X(k.x), Y(k.y), S(k.w), S(k.h));
        const bool isHole = (k.kind == clar::KeyShapeKind::RingHole || k.kind == clar::KeyShapeKind::PlainHole);
        if (pressed) {
            g.setColour(kPressed.withAlpha(0.25f));
            if (isHole) g.fillEllipse(r.expanded(S(0.014f)));
            else g.fillRoundedRectangle(r.expanded(S(0.010f)), S(0.012f));
        }
        if (isHole) {
            if (k.kind == clar::KeyShapeKind::RingHole) {               // outer ring around the hole
                g.setColour(kMetal);
                g.drawEllipse(r.expanded(S(0.006f)), 1.5f);
            }
            g.setColour(pressed ? kPressed : kKeyHole);
            g.fillEllipse(r);
            g.setColour(pressed ? kPressed.brighter(0.4f) : kMetal.withAlpha(0.6f));
            g.drawEllipse(r, 1.0f);
        } else {
            const float corner = (k.kind == clar::KeyShapeKind::Pill) ? r.getHeight() * 0.5f : r.getHeight() * 0.3f;
            juce::Path p;
            p.addRoundedRectangle(r, corner);
            if (std::abs(k.rotationDeg) > 1e-6f)
                p.applyTransform(juce::AffineTransform::rotation(juce::degreesToRadians(k.rotationDeg), r.getCentreX(), r.getCentreY()));
            g.setColour(pressed ? kPressed : kKeyIdle);
            g.fillPath(p);
            g.setColour(pressed ? kPressed.brighter(0.4f) : kMetal);
            g.strokePath(p, juce::PathStrokeType(1.2f));
        }
    }

    // ---- caption ----
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.setFont(juce::FontOptions(std::clamp(side * 0.034f, 12.0f, 20.0f)));
    g.drawText(caption(), area.withTrimmedTop(area.getHeight() - captionH), juce::Justification::centred, true);
}
