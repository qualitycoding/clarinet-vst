# SPDX-License-Identifier: Apache-2.0
set(CLAR_PLUGIN_FORMATS VST3 Standalone)
if(APPLE)
  list(APPEND CLAR_PLUGIN_FORMATS AU)
endif()

juce_add_plugin(ClarinetVST
  COMPANY_NAME "qualitycoding"
  PLUGIN_MANUFACTURER_CODE Qcod
  PLUGIN_CODE Clra
  FORMATS ${CLAR_PLUGIN_FORMATS}
  PRODUCT_NAME "Clarinet"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  VST3_CATEGORIES Instrument Synth)

target_sources(ClarinetVST PRIVATE
  src/PluginProcessor.cpp src/PluginEditor.cpp src/ClarinetView.cpp)
target_include_directories(ClarinetVST PUBLIC src)
target_compile_definitions(ClarinetVST PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(ClarinetVST
  PUBLIC clar_core juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_basics
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

if(CLAR_BUILD_TESTS)
  add_executable(clar_plugin_tests ${CMAKE_SOURCE_DIR}/tests/plugin/test_plugin.cpp)
  target_link_libraries(clar_plugin_tests PRIVATE ClarinetVST Catch2::Catch2)
  add_test(NAME plugin COMMAND clar_plugin_tests)
endif()

# G-004 screenshot bundle generator (S-014): renders every fingering with the real view.
add_executable(clar_ui_snapshots ${CMAKE_SOURCE_DIR}/tools/render/snapshot_ui.cpp)
target_link_libraries(clar_ui_snapshots PRIVATE ClarinetVST)
