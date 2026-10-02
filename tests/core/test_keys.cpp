// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-002 (unit) — lever identifiers used by fixtures and UI; claim C-010.
#include "clar/Keys.h"
#include <catch2/catch_test_macros.hpp>
#include <set>
#include <string>
using namespace clar;

TEST_CASE("T-002 24 levers with unique stable names that round-trip", "[T-002][unit]") {
    STATIC_REQUIRE(kKeyCount == 24);
    const char* expected[] = {"register","thumb","a_key","g_sharp","lh1","lh2","lh_eb_sliver","lh3",
        "lh_c_sharp","lh_e","lh_f","lh_f_sharp","side_1","side_2","side_3","side_4","rh1","rh2",
        "rh_b_sliver","rh3","rh_e","rh_f","rh_f_sharp","rh_g_sharp"};
    std::set<std::string> seen;
    for (std::size_t i = 0; i < kKeyCount; ++i) {
        const auto k = static_cast<KeyId>(i);
        CHECK(keyName(k) == expected[i]);
        seen.insert(std::string(keyName(k)));
        const auto back = keyFromName(expected[i]);
        REQUIRE(back.has_value());
        CHECK(*back == k);
    }
    CHECK(seen.size() == kKeyCount);
    CHECK_FALSE(keyFromName("").has_value());
    CHECK_FALSE(keyFromName("Register").has_value()); // case-sensitive
    CHECK_FALSE(keyFromName("low_eb").has_value());   // bass-clarinet key, not on the soprano
}
