#include "michaelis_menten.h"
#include <iostream>

int main() {
    hls::stream<AXI_VAL> input_stream;
    hls::stream<AXI_VAL> output_stream;

    float Vmax = 15.0;
    float Km = 5.0;
    float S_array[] = {3.0, 5.0, 10.0, 15.0};
    int array_size = sizeof(S_array) / sizeof(S_array[0]);

    AXI_VAL input_val;
    input_val.data = *((int*)(&Vmax));
    input_val.keep = -1;
    input_val.strb = -1;
    input_val.last = 0;
    input_stream.write(input_val);

    input_val.data = *((int*)(&Km));
    input_val.last = 0;
    input_stream.write(input_val);

    for (int i = 0; i < array_size; i++) {
        input_val.data = *((int*)(&S_array[i]));
        input_val.last = (i == array_size - 1) ? 1 : 0;
        input_stream.write(input_val);
    }

    michaelis_menten(input_stream, output_stream, Vmax, Km);

    for (int i = 0; i < array_size; i++) {
        AXI_VAL output_val = output_stream.read();
        float v = *((float*)(&output_val.data));
        std::cout << "Output v[" << i << "] = " << v << std::endl;
    }

    return 0;
}
