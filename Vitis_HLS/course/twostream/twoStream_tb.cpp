#include "ap_axi_sdata.h"
#include "hls_stream.h"
#include <iostream>
#include <cmath>
#include <cassert>

#define LEN 10

typedef ap_axis<32, 2, 5, 6> transPkt;

union fp_int {
    int i;
    float fp;
};

// Prototype of the function to test
void example(hls::stream<transPkt> &A, hls::stream<transPkt> &B, hls::stream<transPkt> &C);

int main() {
    // Declare HLS streams for A, B, and C
    hls::stream<transPkt> A, B, C;

    // Initialize input data
    fp_int Adata, Bdata;
    for (int i = 0; i < LEN; i++) {
        transPkt pktA, pktB;

        // Set float values
        Adata.fp = i * 1.5; // Example: A[i] = 1.5 * i
        Bdata.fp = i * 2.0; // Example: B[i] = 2.0 * i

        // Convert to integer for packet data
        pktA.data = Adata.i;
        pktB.data = Bdata.i;

        pktA.keep = pktB.keep = -1; // Assume valid keep signal
        pktA.strb = pktB.strb = -1; // Assume valid strb signal
        pktA.last = pktB.last = (i == LEN - 1); // Mark the last packet

        // Push packets into streams
        A.write(pktA);
        B.write(pktB);
    }

    // Call the function
    example(A, B, C);

    // Verify output data
    for (int i = 0; i < LEN; i++) {
        transPkt pktC;
        fp_int Cdata;

        // Read output packet
        if (!C.empty()) {
            pktC = C.read();
            Cdata.i = pktC.data;

            // Expected result
            float expected = (i * 1.5) + (i * 2.0);

            // Check output
            if (std::fabs(Cdata.fp - expected) > 1e-6) {
                std::cerr << "Test failed at index " << i << ": expected " << expected
                          << ", got " << Cdata.fp << std::endl;
                return 1;
            }
        }
    }

    std::cout << "Test passed successfully!" << std::endl;
    return 0;
}
