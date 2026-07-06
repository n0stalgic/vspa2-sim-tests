#include <cstdint>
#include <vspa/intrinsics.h>
#include "vspa.h"
#include "vcpu.h"

#define VRA_LEN			8
#define VRA_WORD_CAPACITY	32	/* 32 32-bit words */
#define DMEM_LINE_SIZE_BYTES    128	/* 1024 bits */
#define MEM_LINE_SIZE		(512 / 32)

#define TX_AMPLITUDE 0.9f

#pragma optimization_level 0

void mod_bpsk(uint32_t* bit_in, vspa_complex_fixed16* bpsk_out, uint32_t N)
{
	float clamp[VRA_WORD_CAPACITY];
	const uint32_t iter_count = N / VRA_WORD_CAPACITY;
	const float factor = 2 * TX_AMPLITUDE;
	const float *factor_ptr = &factor;
	float *clamp_ptr = clamp;
	
	for (uint32_t i = 0; i < VRA_WORD_CAPACITY; i++)
		clamp[i] = -TX_AMPLITUDE;

	__clr_VRA();
	__set_prec(single, single, single, single, half_fixed);
	__set_Smode(S0hword, S1straight, S2straight);
	__set_VRAptr_rV(_VR5);
	__set_VRAptr_rSt(5);
	
	__ld_Rx_mem_unaligned(0, factor_ptr);
	__set_VRAptr_rS0(_VR0);
	__rd_S0();

	__ld_Rx_mem_unaligned(2, clamp_ptr);
	__set_VRAptr_rS2(_VR2);
	__rd_S2();
	
	for (uint32_t i = 0; i < iter_count; i++)
	{
		uint32_t* bit_in_ptr = bit_in + i * VRA_WORD_CAPACITY;
		__ld_Rx_mem_unaligned(1, bit_in_ptr);
		__set_VRAptr_rS1(_VR1);
		__rd_S1();
		__rmad();
		__wr(straight);
		__st_vec((vspa_vector_pair_fixed16 *) bpsk_out + i);
	}
}

#pragma optimization_level reset
