# SPDX-License-Identifier: Apache-2.0
add_library(clar_core STATIC
  src/Pitch.cpp src/Keys.cpp src/Fingering.cpp src/KeyLayout.cpp
  src/ResonatorTable.cpp src/ReedModel.cpp src/ColinotReference.cpp
  src/ClarinetVoice.cpp src/Analysis.cpp src/State.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/embedded_resonators.cpp)
set(CLAR_RESONATOR_JSON ${PROJECT_SOURCE_DIR}/data/clarinet_resonators.json)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/embedded_resonators.cpp
  COMMAND ${CMAKE_COMMAND} -DIN=${CLAR_RESONATOR_JSON} -DOUT=${CMAKE_CURRENT_BINARY_DIR}/embedded_resonators.cpp
          -P ${CMAKE_CURRENT_SOURCE_DIR}/embed_json.cmake
  DEPENDS ${CLAR_RESONATOR_JSON} ${CMAKE_CURRENT_SOURCE_DIR}/embed_json.cmake
  COMMENT "Embedding data/clarinet_resonators.json")
target_include_directories(clar_core PUBLIC include)
target_compile_features(clar_core PUBLIC cxx_std_20)
target_link_libraries(clar_core PRIVATE nlohmann_json::nlohmann_json)
set_target_properties(clar_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(clar_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(clar_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()
