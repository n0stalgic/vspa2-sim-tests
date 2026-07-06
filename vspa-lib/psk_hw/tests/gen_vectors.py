#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""BPSK-via-ld.qam vector generator.

Mirrors la931x_vspa_common/vspa-lib/qam/tests: N is a *line count*, not a
bit count. One line = 32 packed bits in = 32 cfloat16 symbols out. This
generates N_LINES=1 worth of vectors (one input word, 32 output symbols).

Reference follows qam/python/model.py's BPSK oracle: map_table[bit] =
{-1, +1}, then multiplied by BPSK's norm_u32 (0x3F800000 == 1.0f exactly),
so the reference reduces to raw, unscaled bit -> -1/+1, imag=0 -- no
headroom scaling like mod_bpsk's, since this test is only probing whether
ld.qam's constellation mapping itself is correct.

This intentionally matches qamModBpsk's own convention (packed-bit input,
cfloat16 packed output) rather than mod_bpsk's (one float per bit,
half-fixed output) -- ld.qam consumes packed bits and its Vprec here is
half (IEEE fp16), not half-fixed.
"""

from __future__ import annotations

import random
import sys
from pathlib import Path

_TESTS_DIR = Path(__file__).resolve().parent
_COMMON_PY = _TESTS_DIR.parent.parent / 'common' / 'python'
if str(_COMMON_PY) not in sys.path:
    sys.path.insert(0, str(_COMMON_PY))

import numpy as np
from vspa.arith import r_half_flt

N_LINES = 1
N_BITS = N_LINES * 32  # one packed word per line
SEED = 20260706

OUTDIR = _TESTS_DIR / 'vectors'


def pack_cfloat16(real: float, imag: float) -> int:
    """Pack (real, imag) as fp16 into one uint32: (imag << 16) | real."""
    re16 = np.float16(r_half_flt(real))
    im16 = np.float16(r_half_flt(imag))
    re_bits = int(np.frombuffer(re16.tobytes(), dtype='<u2')[0])
    im_bits = int(np.frombuffer(im16.tobytes(), dtype='<u2')[0])
    return (im_bits << 16) | re_bits


def main() -> None:
    rng = random.Random(SEED)
    bits = [rng.randint(0, 1) for _ in range(N_BITS)]

    word = 0
    for i, b in enumerate(bits):
        word |= (b << i)  # LSB-first packing, matching ld.qam's bit order

    OUTDIR.mkdir(parents=True, exist_ok=True)

    with open(OUTDIR / 'input.hex', 'w') as f:
        f.write(f'0x{word:08X},\n')

    with open(OUTDIR / 'ref.hex', 'w') as f:
        for b in bits:
            real = 1.0 if b else -1.0
            f.write(f'0x{pack_cfloat16(real, 0.0):08X},\n')

    print(f'Generated N_LINES={N_LINES} ({N_BITS} bits, 1 packed word) -> vectors/input.hex, vectors/ref.hex')


if __name__ == '__main__':
    main()
