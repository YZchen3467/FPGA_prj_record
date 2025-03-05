#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "sleep.h"

#define DDR_BASE_ADDR XPAR_DDR4_0_C0_DDR4_MEMORY_MAP_BASEADDR
#define DDR_OFFSET   0x1000

 
// int main()
// {
//     init_platform();
 
// //    Xil_DCacheDisable();
 
//     xil_printf("MB_MIG_TEST\n\r");
//     u64 test_data[8]={0};
 
//     for(int i=0; i < 64; i++){
//         u32 data = (u32)0x00000000;
 
//         u32 addr = DDR_BASE_ADDR + (i * 4);  
 
//         Xil_Out32(addr, data);
//     }
 
//     for (int i = 0; i < 64; i++) {
//         u32 data = (u32)0x5A5A6B6B;
 
//         u32 addr = DDR_BASE_ADDR + (i * 4); 
 
//         Xil_Out32(addr, data);
//         Xil_In32(addr);
//         xil_printf("ADDR: 0x%08X, Data Written: 0x%08X\r\n", addr, data);
//     }
 
//     Xil_DCacheFlush();
//     Xil_DCacheInvalidate();
//     int error_count = 0;
//     for (int i = 0; i < 64; i++) {
//         u32 addr = DDR_BASE_ADDR + (i * 4);
 
//         u32 read_data = Xil_In32(addr);            
 
//         if (read_data != test_data[i]) {
//             xil_printf("ERROR: Address: 0x%08X, Expected: 0x%08X, Read: 0x%08X\r\n",
//                             addr, (u32)(test_data[i]),read_data);
//                  error_count++;
//         } else {
//             xil_printf("SUCCESS: Address: 0x%08X, Data: 0x%08X\r\n",
//                             addr, read_data);
//         }
//     }
// }
 
int main()
{
    init_platform();
 
//    Xil_DCacheDisable();
 
    xil_printf("MB_MIG_TEST\n\r");
    u64 test_data[8] = {0};
    
    volatile uint32_t *ddr_ptr = (volatile uint32_t *)DDR_BASE_ADDR;

    for(int i = 0; i < 64; i++){
        *(ddr_ptr + i) = 0x00000000;
    }

    for (int i = 0; i < 64; i++) {
        *(ddr_ptr + i) = 0x5A5A6B6B;
        xil_printf("ADDR: 0x%08X, Data Written: 0x%08X\r\n", 
                   (uint32_t)(DDR_BASE_ADDR + i * 4), *(ddr_ptr + i));
    }

    Xil_DCacheFlush();
    Xil_DCacheInvalidate();

    int error_count = 0;
    for (int i = 0; i < 64; i++) {
        uint32_t read_data = *(ddr_ptr + i);

        if (read_data != test_data[i]) {
            xil_printf("ERROR: Address: 0x%08X, Expected: 0x%08X, Read: 0x%08X\r\n",
                       (uint32_t)(DDR_BASE_ADDR + i * 4), (uint32_t)(test_data[i]), read_data);
            error_count++;
        } else {
            xil_printf("SUCCESS: Address: 0x%08X, Data: 0x%08X\r\n",
                       (uint32_t)(DDR_BASE_ADDR + i * 4), read_data);
        }
    }

    return 0;
}