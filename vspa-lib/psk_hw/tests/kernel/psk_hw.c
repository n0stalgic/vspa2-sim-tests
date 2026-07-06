#include <cstdint>
#include <vspa/intrinsics.h>
#include "vspa.h"
#include "vcpu.h"
#include "iohw.h"

/* BPSK normalization (1.0, see qam.h's QAM_NORMALIZATION_BPSK) pre-scaled by
 * 8 to compensate for HPFS/HPFV=0 (set.creg 255,0 disables the hardware's
 * half-fixed auto-scale-by-2) */
#define QAM_NORM_BPSK  8.0f

#pragma optimization_level 0

void mod_bpsk_hw(uint32_t* bit_in, vspa_complex_float16* bpsk_out, uint32_t N)
{
	iowr(LD_RF_TB_REAL_0, 0x00000002);
	iowr(LD_RF_CONTROL, 1);

	__clr_VRA();
	__set_creg(255, 0);
	__set_prec(single, half_fixed, half_fixed, single, half);
	__set_Smode(S0i1r1i1r1, S1straight, S2zeros);

	__set_VRAptr_rV(_VR0);
	__mv_w_rV(QAM_NORM_BPSK);
	__set_VRAptr_rS0(_VR0);
	__rd_S0();

	__set_VRAptr_rS1(_VR2);

	__rd_S2();

	__set_VRAptr_rSt(0);

	for (uint32_t i = 0; i < N; i++)
	{
		__ld_vec(bit_in + i);
		__ld_Rx(qam, 2);
		__rd_S1();
		__rmad();
		__wr(straight);
		__st_vec((vspa_vector_pair_float16 *) bpsk_out + i);
	}
}

#pragma optimization_level reset
