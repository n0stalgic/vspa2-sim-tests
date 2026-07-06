// SPDX-License-Identifier: BSD-3-Clause
// bpsk_hw kernel test harness -- validates the ld.qam hardware-accelerator
// path (see kernel/psk_hw.c) against the same packed-bit / cfloat16
// convention as NXP's own qamModBpsk
// (la931x_vspa_common/vspa-lib/qam/tests/test_qam.c): N is a *line count*,
// not a bit count -- each line packs 32 bits into one input word and
// produces 32 cfloat16 output symbols. This test uses N=1 (one packed
// word in, 32 symbols out) since it's only probing whether ld.qam's
// constellation mapping itself is correct, not throughput.
//
// Reference is raw, unscaled bit -> -1/+1 (imag=0), packed as cfloat16
// ((imag_fp16<<16) | real_fp16) -- see gen_vectors.py.

#include <stdint.h>
#include <stdio.h>
#include <vspa/intrinsics.h>
#include "test_utils.h"

extern void mod_bpsk_hw(unsigned int *bit_in, vspa_complex_float16 *bpsk_out, unsigned int N);

#define N_LINES        1
#define N_INPUT_WORDS  1
#define N_OUT_SYMBOLS  32

// Compile-time-baked vectors from gen_vectors.py.
static const unsigned int INPUT_DATA[N_INPUT_WORDS] = {
#include "vectors/input.hex"
};

static const unsigned int REF_DATA[N_OUT_SYMBOLS] = {
#include "vectors/ref.hex"
};

// cwproj-aligned BSS buffers -- vector-aligned, zero on entry to main().
_VSPA_VECTOR_ALIGN static unsigned int bitIn[N_INPUT_WORDS];
_VSPA_VECTOR_ALIGN static unsigned int bpskOut[N_OUT_SYMBOLS];

int main(void)
{
    int i;

    for (i = 0; i < N_INPUT_WORDS; i++)
        bitIn[i] = INPUT_DATA[i];

    KCYC_INIT();
    KCYC_START();
    mod_bpsk_hw(bitIn, (vspa_complex_float16 *)bpskOut, (unsigned int)N_LINES);
    KCYC_STOP_PRINT();

    return vspa_array_cmp(bpskOut, REF_DATA, N_OUT_SYMBOLS);
}
