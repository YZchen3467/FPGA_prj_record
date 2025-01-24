#ifndef SMUL_H
#define SMUL_H

#include "ap_axi_sdata.h"
#include "hls_stream.h"

// Define AXI Stream Data format
typedef ap_axiu<32, 0, 0, 0> trans_pkt;

// Function declare
void smul(hls::stream<trans_pkt> &INPUT, hls::stream<trans_pkt> &OUTPUT);

#endif // SMUL_H
