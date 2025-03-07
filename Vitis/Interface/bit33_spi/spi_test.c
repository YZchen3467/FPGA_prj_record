#include <stdio.h>
#include "xparameters.h"
#include "spi_mst.h"
#include "xil_printf.h"
#include "xuartlite.h"

#include "uart_getchar.h"
#include "xil_io.h"
#include "sleep.h"

#define SPI_MST_BASEADDR    XPAR_SPI_MST_0_S00_AXI_BASEADDR
#define START (SPI_MST_BASEADDR + 0x0)
#define HIGH_1_DATA (SPI_MST_BASEADDR + 0x4)
#define LOW_32_DATA (SPI_MST_BASEADDR + 0x8)
#define READ_SPI    (SPI_MST_BASEADDR + 0xC)

/* Function prototype */
void single_transfer();

/* Function implementation */
// Transfer
void single_transfer(){
    Xil_Out32(START, 0x1);
    // usleep(1);
    Xil_Out32(START, 0x0);
}

int main(){

    xil_printf("Hello, SPI!\r\n");

    u64 spi_data_full;
    u32 spi_data_low;
    u8  spi_data_high;
    u32 spi_read_data;

    while (1){
        u8 rw = read_single_bit("Please enter a value from 0 to 1, then press enter(0:Write or 1:Read): ", 1);
        u8 ch = read_hex_input("Please enter channel from 0 to 3, then press enter(0:CH0; 1:CH1; 2:CH2; 3:CH3): ", 1) & 0x3;
        u8 addr = read_hex_input("Please enter address from 0x00 to 0xFF, then press enter: ", 2) & 0xFF;
        u32 cmd = read_hex_input("Please enter command from 000000 to 3FFFFF, then press enter: ", 6) & 0x3FFFFF;

        spi_data_full = ((u64)rw << 32) | ((u64)ch << 30) | ((u64)addr << 22) | cmd;
        spi_data_high = (spi_data_full >> 32) & 0x1;     
        spi_data_low = spi_data_full & 0xFFFFFFFF;

        Xil_Out32(HIGH_1_DATA, spi_data_high);
        Xil_Out32(LOW_32_DATA, spi_data_low);

        single_transfer();

        spi_read_data = Xil_In32(READ_SPI);

        switch (spi_read_data) {
            case 0x06d53e:
                xil_printf("Ver: 1.0, YY: 2024, MM: 10, DD: 31\r\n\n");
                break;
            case 0x26d53e:
                xil_printf("Status 0\r\n\n");
                break;
            case 0x16d53e:
                xil_printf("Status 1\r\n\n");
                break;
            case 0x36d53e:
                xil_printf("Status 2\r\n\n");
                break;
            case 0x17d53e:
                xil_printf("Status 3\r\n\n");
                break;
            case 0x16d53f:
                xil_printf("Status 4\r\n\n");
                break;
            case 0x06d54e:
                xil_printf("Status 5\r\n\n");
                break;
            case 0x06d55e:
                xil_printf("Status 6\r\n\n");
                break;
            case 0x06d56e:
                xil_printf("Status 7\r\n\n");
                break;
            case 0x06d73e:
                xil_printf("Status 8\r\n\n");
                break;
            default:
                xil_printf("Write operation\r\n\n");
                break;
        }

        usleep(1000000);
    };  
    return 0;

}
