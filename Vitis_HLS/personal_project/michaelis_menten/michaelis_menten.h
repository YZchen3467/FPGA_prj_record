#ifndef MICHAELIS_MENTEN_H
#define MICHAELIS_MENTEN_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>


typedef ap_axiu<32, 1, 1, 1> AXI_VAL;

void michaelis_menten(
    hls::stream<AXI_VAL>& input_stream,
    hls::stream<AXI_VAL>& output_stream,
    float Vmax,
    float Km
);

#endif // MICHAELIS_MENTEN_H
