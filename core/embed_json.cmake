# SPDX-License-Identifier: Apache-2.0
# Generates a C++ source exposing clar::embeddedResonatorJson() from data/clarinet_resonators.json (D-009).
# The JSON is stored as <= 12 kB raw-string chunks and joined once at run time (MSVC string-literal limits).
file(READ "${IN}" content)
string(LENGTH "${content}" len)
set(chunks "")
set(pos 0)
while(pos LESS len)
  string(SUBSTRING "${content}" ${pos} 12000 piece)
  string(APPEND chunks "    R\"clarjson(${piece})clarjson\",\n")
  math(EXPR pos "${pos} + 12000")
endwhile()
file(WRITE "${OUT}" "// Generated from data/clarinet_resonators.json by core/embed_json.cmake. Do not edit.
#include \"clar/ResonatorTable.h\"
#include <string>
namespace clar {
std::string_view embeddedResonatorJson() {
  static const char* const kChunks[] = {
${chunks}  };
  static const std::string joined = [] {
    std::string s;
    for (const char* c : kChunks) s += c;
    return s;
  }();
  return joined;
}
}
")
