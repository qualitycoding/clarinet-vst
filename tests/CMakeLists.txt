# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
set(CLAR_FIXTURE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")

function(clar_test name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE clar_core Catch2::Catch2WithMain)
  target_include_directories(${name} PRIVATE core)
  target_compile_definitions(${name} PRIVATE CLAR_FIXTURE_DIR="${CLAR_FIXTURE_DIR}")
endfunction()

clar_test(clar_unit_tests
  core/test_pitch.cpp core/test_keys.cpp core/test_fingering.cpp
  core/test_resonator_table.cpp core/test_reed_model.cpp core/test_state.cpp
  core/test_key_layout.cpp core/test_analysis.cpp)
clar_test(clar_integration_tests
  core/test_colinot_reference.cpp core/test_voice_pitch.cpp core/test_voice_registers.cpp
  core/test_midi_behaviour.cpp)
clar_test(clar_operational_tests core/test_voice_robustness.cpp)
clar_test(clar_alloc_tests core/test_alloc.cpp)
clar_test(clar_perf_tests core/test_perf.cpp)

add_test(NAME unit        COMMAND clar_unit_tests)
add_test(NAME integration COMMAND clar_integration_tests)
add_test(NAME operational COMMAND clar_operational_tests)
add_test(NAME alloc       COMMAND clar_alloc_tests)
add_test(NAME perf        COMMAND clar_perf_tests)
set_tests_properties(perf PROPERTIES LABELS "perf")
