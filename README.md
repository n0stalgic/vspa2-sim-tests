This repo currently contains a few VSPA (Vector Signal Processing Accelerator) kernels for the NXP LA9310 Baseband Processor, as well as a test harness for running on the VSPA cycle-accurate simulator

Simulator results:

| Modulation Algorithm  | Cycles  |
|:-:|---|
| BPSK vectorized non-accelerated  | 1084  |
| BPSK vectorized accelerated  | 96  |
| QPSK vectorized accelerated  | 99  |
