// SPDX-License-Identifier: Apache-2.0
// clar_render — offline renderer used by the realism comparison (D-015) and the G-003 bundle.
// Usage: clar_render --note N --velocity V --seconds S --fs F --out FILE.wav
//                   [--overblow O] [--overblown-fingering 0|1] [--release-at T]
// Writes mono 32-bit float WAV. Exit codes: 0 ok, 1 bad arguments, 2 not implemented.
// STUB (S-000): prints "not implemented" and exits 2.
#include <cstdio>
int main(int, char**) {
    std::fprintf(stderr, "clar_render: not implemented\n");
    return 2;
}
