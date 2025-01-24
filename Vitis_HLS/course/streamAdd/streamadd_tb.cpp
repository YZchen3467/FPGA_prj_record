#include "streamAdd.h"
#include <iostream>

using namespace std;

int main() {

    hls::stream<trans_pkt> input_stream;
    hls::stream<trans_pkt> output_stream;

    trans_pkt input_data;
    trans_pkt output_data;
    for(int i=0; i<10; ++i){
        input_data.data = i + 1;
        input_data.keep = -1;
        input_data.strb = -1;
        input_data.user = 0;
        input_data.id = 0;
        input_data.dest = 0;

        input_stream.write(input_data);

        smul(input_stream, output_stream);

        if (!output_stream.empty()) {
            output_data = output_stream.read();
            cout << "Input Data: " << input_data.data
                      << ", Output Data: " << output_data.data;
            cout << "\n";
        } else {
           cout << "Output Stream is empty!\n";
        }

    }

    return 0;
}
