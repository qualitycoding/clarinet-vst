# SPDX-License-Identifier: Apache-2.0
add_executable(clar_calibrate calibrate_tuning.cpp)
target_link_libraries(clar_calibrate PRIVATE clar_core)
if(MSVC)
  target_compile_options(clar_calibrate PRIVATE /W4 /permissive-)
else()
  target_compile_options(clar_calibrate PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()
