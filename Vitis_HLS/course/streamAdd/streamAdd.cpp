#include "ap_axi_sdata.h"
#include "hls_stream.h"
#include "streamAdd.h"

void smul(hls::stream< trans_pkt > &INPUT, hls::stream< trans_pkt > &OUTPUT)
{
#pragma HLS INTERFACE s_axilite port = return bundle = CTRL

                #pragma HLS INTERFACE axis port=INPUT
                #pragma HLS INTERFACE axis port=OUTPUT
                trans_pkt data_p;

                INPUT.read(data_p);
                data_p.data *= 2;
                OUTPUT.write(data_p);
}
