#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""BPSK vector generator: bit_in (single-precision 0.0f/1.0f) -> AMPLITUDE-scaled +-1.

Reference is encoded in VSPA's half-fixed sign-magnitude format (1-bit sign,
15-bit truncated fraction) -- NOT two's complement, so this does not reuse
common/python/utils/precision.py's float_to_q15 (that one is two's complement
and would silently produce a wrong reference for negative values).
"""

from __future__ import annotations

import random
from pathlib import Path

N_BITS = 32  # one full VRA_WORD_CAPACITY chunk -> one kernel loop iteration
SEED = 20260705

# Must match kernel/psk.c's headroom scaling (factor = 2*AMPLITUDE, clamp = -AMPLITUDE).
AMPLITUDE = 0.9

OUTDIR = Path(__file__).resolve().parent / 'vectors'


def half_fixed_bits(x: float) -> int:
    """Encode x in [-1, 1) as VSPA half-fixed: sign(1) | truncated fraction(15)."""
    sign = 1 if x < 0 else 0
    fraction = min(int(abs(x) * 32768), 0x7FFF)  # truncation, matching VSPA hardware
    return (sign << 15) | fraction


def main() -> None:
    rng = random.Random(SEED)
    bits = [rng.randint(0, 1) for _ in range(N_BITS)]

    OUTDIR.mkdir(parents=True, exist_ok=True)

    with open(OUTDIR / 'input.hex', 'w') as f:
        for b in bits:
            f.write(f'{float(b)}f,\n')

    with open(OUTDIR / 'ref.hex', 'w') as f:
        for i in range(0, N_BITS, 2):
            lo = half_fixed_bits(2 * AMPLITUDE * bits[i] - AMPLITUDE)
            hi = half_fixed_bits(2 * AMPLITUDE * bits[i + 1] - AMPLITUDE)
            word = (hi << 16) | lo
            f.write(f'0x{word:08X},\n')

    print(f'Generated {N_BITS} bits -> vectors/input.hex, vectors/ref.hex')


if __name__ == '__main__':
    main()
