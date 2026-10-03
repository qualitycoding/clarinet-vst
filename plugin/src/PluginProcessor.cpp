// SPDX-License-Identifier: Apache-2.0
// Clarinet plugin processor (S-013): parameters -> clar::ClarinetVoice, MIDI dispatch per D-011, state per D-013.
#include "PluginProcessor.h"
#include "Parameters.h"
#include "PluginEditor.h"
#include "clar/ResonatorTable.h"
#include "clar/State.h"
#include <algorithm>
#include <string_view>

namespace {
std::shared_ptr<const clar::ResonatorTable> loadTable() {
    return std::make_shared<const clar::ResonatorTable>(clar::ResonatorTable::fromJson(clar::embeddedResonatorJson()));
}
} // namespace

static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout() {
    using namespace clarplug;
    using R = juce::NormalisableRange<float>;
    // Two decimals in every host/editor readout (the default float text shows seven).
    auto flt = [](const char* id, const char* name, R range, float def) {
        const auto attrs = juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float v, int) { return juce::String(v, 2); });
        return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id, 1}, name, range, def, attrs);
    };
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    l.add(flt(param::overblow, "Overblow", R{0.f, 1.f}, 0.f));
    l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{param::overblownFingering, 1}, "Overblown fingering", false));
    l.add(flt(param::reedHardness, "Reed hardness", R{0.f, 1.f}, .5f));
    l.add(flt(param::brightness, "Brightness", R{0.f, 1.f}, .5f));
    l.add(flt(param::breathNoise, "Breath noise", R{0.f, 1.f}, .3f));
    l.add(flt(param::vibratoRate, "Vibrato rate", R{3.f, 8.f}, 5.5f));
    l.add(flt(param::vibratoDepth, "Vibrato depth", R{0.f, 1.f}, 0.f));
    l.add(flt(param::portamento, "Portamento (ms)", R{0.f, 500.f}, 30.f));
    l.add(flt(param::tuning, "Tuning A4 (Hz)", R{415.f, 466.f}, 440.f));
    l.add(flt(param::outputGain, "Output gain (dB)", R{-24.f, 12.f}, 0.f));
    return l;
}

ClarinetAudioProcessor::ClarinetAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "STATE", makeLayout()),
      voice_(loadTable()) {
    using namespace clarplug::param;
    const char* ids[] = {overblow, overblownFingering, reedHardness, brightness, breathNoise,
                         vibratoRate, vibratoDepth, portamento, tuning, outputGain};
    for (std::size_t i = 0; i < 10; ++i) raw_[i] = apvts_.getRawParameterValue(ids[i]);
}
ClarinetAudioProcessor::~ClarinetAudioProcessor() = default;

clar::VoiceParameters ClarinetAudioProcessor::readParameters() const noexcept {
    clar::VoiceParameters p;
    p.overblow = raw_[0]->load();
    p.overblownFingering = raw_[1]->load() >= 0.5f;
    p.reedHardness = raw_[2]->load();
    p.brightness = raw_[3]->load();
    p.breathNoise = raw_[4]->load();
    p.vibratoRateHz = raw_[5]->load();
    p.vibratoDepth = raw_[6]->load();
    p.portamentoMs = raw_[7]->load();
    p.tuningA4Hz = raw_[8]->load();
    p.outputGainDb = raw_[9]->load();
    return p;
}

void ClarinetAudioProcessor::applyParameters(const clar::VoiceParameters& p) {
    using namespace clarplug::param;
    auto set = [this](const char* id, float v) {
        if (auto* prm = apvts_.getParameter(id)) prm->setValueNotifyingHost(prm->convertTo0to1(v));
    };
    set(overblow, p.overblow);
    set(overblownFingering, p.overblownFingering ? 1.0f : 0.0f);
    set(reedHardness, p.reedHardness);
    set(brightness, p.brightness);
    set(breathNoise, p.breathNoise);
    set(vibratoRate, p.vibratoRateHz);
    set(vibratoDepth, p.vibratoDepth);
    set(portamento, p.portamentoMs);
    set(tuning, p.tuningA4Hz);
    set(outputGain, p.outputGainDb);
}

void ClarinetAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    prepared_ = false;
    try {
        maxBlock_ = std::max(1, samplesPerBlock);
        voice_.prepare(sampleRate, maxBlock_); // throws for rates outside 22.05..192 kHz: stay silent instead of crashing the host
        setLatencySamples(voice_.latencySamples());
        prepared_ = true;
    } catch (...) {
        prepared_ = false;
    }
}
void ClarinetAudioProcessor::releaseResources() { if (prepared_) voice_.reset(); }
bool ClarinetAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    const auto out = l.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void ClarinetAudioProcessor::handleMidi(const juce::MidiMessage& m) noexcept {
    if (m.isNoteOn()) {
        voice_.noteOn(m.getNoteNumber(), static_cast<float>(m.getVelocity()) / 127.0f);
    } else if (m.isNoteOff()) {
        voice_.noteOff(m.getNoteNumber());
    } else if (m.isController()) {
        const int cc = m.getControllerNumber();
        const float v = static_cast<float>(m.getControllerValue()) / 127.0f;
        if (cc == 2 || cc == 11) voice_.setBreath(v);                       // breath controller / expression (D-011)
        else if (cc == 120 || cc == 123) voice_.allNotesOff();               // all sound / all notes off
        else if (cc == 121) { voice_.setBreath(-1.0f); voice_.setPitchBend(0.0f); voice_.setAftertouch(0.0f); } // reset controllers
    } else if (m.isChannelPressure()) {
        voice_.setAftertouch(static_cast<float>(m.getChannelPressureValue()) / 127.0f);
    } else if (m.isPitchWheel()) {
        voice_.setPitchBend(static_cast<float>(m.getPitchWheelValue() - 8192) / 8192.0f);
    }
}

void ClarinetAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    buffer.clear();
    if (!prepared_ || buffer.getNumChannels() < 1 || numSamples < 1) return;

    voice_.setParameters(readParameters());
    float* out = buffer.getWritePointer(0);
    int pos = 0;
    auto renderTo = [&](int end) {
        while (pos < end) {
            const int n = std::min(end - pos, maxBlock_);
            voice_.process(out + pos, n);
            pos += n;
        }
    };
    for (const auto meta : midi) {                       // MIDI is delivered sorted by sample position
        renderTo(std::clamp(meta.samplePosition, 0, numSamples));
        handleMidi(meta.getMessage());
    }
    renderTo(numSamples);
    for (int ch = 1; ch < buffer.getNumChannels(); ++ch) buffer.copyFrom(ch, 0, buffer, 0, 0, numSamples);
}

juce::AudioProcessorEditor* ClarinetAudioProcessor::createEditor() { return new ClarinetAudioProcessorEditor(*this); }

void ClarinetAudioProcessor::getStateInformation(juce::MemoryBlock& dest) {
    const std::string text = clar::serializeState(readParameters());
    dest.replaceAll(text.data(), text.size());
}
void ClarinetAudioProcessor::setStateInformation(const void* data, int size) {
    if (data == nullptr || size <= 0) return;
    const auto parsed = clar::deserializeState(std::string_view(static_cast<const char*>(data), static_cast<std::size_t>(size)));
    if (parsed) applyParameters(*parsed); // anything invalid is ignored: the current values stay
}
clar::KeySet ClarinetAudioProcessor::currentKeys() const noexcept { return voice_.currentKeys(); }
int ClarinetAudioProcessor::currentConcertNote() const noexcept { return voice_.currentConcertNote(); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ClarinetAudioProcessor(); }
