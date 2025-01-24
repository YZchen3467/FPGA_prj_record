#include "michaelis_menten.h"

// Michaelis-Menten function definition
void michaelis_menten(
    hls::stream<AXI_VAL>& input_stream,
    hls::stream<AXI_VAL>& output_stream,
    float Vmax,
    float Km
) {
    #pragma HLS INTERFACE axis port=input_stream
    #pragma HLS INTERFACE axis port=output_stream
	#pragma HLS INTERFACE s_axilite port=Vmax bundle=control
	#pragma HLS INTERFACE s_axilite port=Km bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    while (!input_stream.empty()) {

        AXI_VAL input_val = input_stream.read();
        float S = *((float*)(&input_val.data));

        float v = (Vmax * S) / (Km + S);

        AXI_VAL output_val;
        output_val.data = *((int*)(&v));
        output_val.keep = input_val.keep;
        output_val.strb = input_val.strb;
        output_val.last = input_val.last;

        output_stream.write(output_val);
    }
}
