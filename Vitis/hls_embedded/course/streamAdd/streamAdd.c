#include "xparameters.h"
#include "xil_io.h"
#include "xil_printf.h"
#include "xaxidma.h"
#include "sleep.h"
#include "xsmul.h"

/***** Define ******/ 
// Device addr
#define DMA_DEV_ID              XPAR_AXI_DMA_0_DEVICE_ID
#define SMUL_DEV_ID             XPAR_SMUL_0_DEVICE_ID
#define DMA_BASE_ADDR           XPAR_AXI_DMA_0_BASEADDR
#define SMUL_BASE_ADDR          XPAR_SMUL_0_S_AXI_CTRL_BASEADDR

// PS DDR addr
#define MEM_BASE_ADDR XPAR_PSU_DDR_0_S_AXI_BASEADDR
#define INPUT_BUFFER (MEM_BASE_ADDR + 0x00100000)
#define OUTPUT_BUFFER (MEM_BASE_ADDR + 0x00300000)

// HLS IP Register Offsets
// 0x0 : Control signals
//       bit 0  - ap_start (Read/Write/COH)
//       bit 1  - ap_done (Read/COR)
//       bit 2  - ap_idle (Read)
//       bit 3  - ap_ready (Read/COR)
//       bit 7  - auto_restart (Read/Write)
//       bit 9  - interrupt (Read)
//       others - reserved
#define CTRL_ADDR_AP_CTRL (SMUL_BASE_ADDR + 0x0)
#define CTRL_ADDR_GIE     (SMUL_BASE_ADDR + 0x4)
#define CTRL_ADDR_IER     (SMUL_BASE_ADDR + 0x8)
#define CTRL_ADDR_ISR     (SMUL_BASE_ADDR + 0xc)

/***** HLS interactive data size ******/ 
#define DATA_SIZE 20

/***** Function prototype *****/ 
// Vitis hls ip function
void smul_ip_start();         // Start the hls ip and restart
void smul_ip_status();
void smul_ip_stop();

XStatus smul_start(XSmul *SmulInst);

// Axi Dma control
XStatus DmaSetup(XAxiDma *DmaInsPtr);
int DmaTransfer(XAxiDma *DmaInsPtr, u32 *input_buffer, u32 *output_buffer, int data_size);


// void smul_ip_start() {
//     xil_printf("HLS IP Start\r\n");
//     Xil_Out32(CTRL_ADDR_AP_CTRL, 0x81);
    
//     xil_printf("HLS IP has completed the computation and is ready for next operation.\r\n");
// }

// void smul_ip_status(){
//     u32 HLS_STATUS = Xil_In32(CTRL_ADDR_AP_CTRL);

//     uint8_t ap_done  = (HLS_STATUS >> 1) & 0x1;  // bit 1
//     uint8_t ap_idle  = (HLS_STATUS >> 2) & 0x1;  // bit 2
//     uint8_t ap_ready = (HLS_STATUS >> 3) & 0x1;  // bit 3

//     uint8_t combined_status = (ap_done << 2) | (ap_idle << 1) | ap_ready;

//     switch (combined_status) {
//         case 0b000:  // ap_done = 0, ap_idle = 0, ap_ready = 0
//             xil_printf("HLS IP status: RUNNING (not done, not idle, not ready)\r\n");
//             break;
//         case 0b001:  // ap_done = 0, ap_idle = 0, ap_ready = 1
//             xil_printf("HLS IP status: READY but NOT IDLE, NOT DONE\r\n");
//             break;
//         case 0b010:  // ap_done = 0, ap_idle = 1, ap_ready = 0
//             xil_printf("HLS IP status: IDLE but NOT DONE\r\n");
//             break;
//         case 0b011:  // ap_done = 0, ap_idle = 1, ap_ready = 1
//             xil_printf("HLS IP status: READY and IDLE, NOT DONE\r\n");
//             break;
//         case 0b100:  // ap_done = 1, ap_idle = 0, ap_ready = 0
//             xil_printf("HLS IP status: DONE but NOT IDLE, NOT READY\r\n");
//             break;
//         case 0b101:  // ap_done = 1, ap_idle = 0, ap_ready = 1
//             xil_printf("HLS IP status: DONE and READY, NOT IDLE\r\n");
//             break;
//         case 0b110:  // ap_done = 1, ap_idle = 1, ap_ready = 0
//             xil_printf("HLS IP status: DONE and IDLE, NOT READY\r\n");
//             break;
//         case 0b111:  // ap_done = 1, ap_idle = 1, ap_ready = 1
//             xil_printf("HLS IP status: DONE, IDLE, and READY (Fully Complete)\r\n");
//             break;
//         default:
//             xil_printf("Unknown HLS IP status.\r\n");
//             break;
//     }

// }

// void smul_ip_stop(){
//     xil_printf("HLS IP Stop\r\n");

//     Xil_Out32(CTRL_ADDR_AP_CTRL, 0x0);
// }

XStatus smul_start(XSmul *SmulInst){
    XSmul_Config *SmulCfg;
    int Status;

    SmulCfg = XSmul_LookupConfig(SMUL_DEV_ID);
    if(!SmulCfg){
        xil_printf("Smul dev not found!\r\n");
        return XST_FAILURE;
    }

    Status = XSmul_CfgInitialize(SmulInst, SmulCfg);
    if(Status != XST_SUCCESS){
        xil_printf("Smul configuration failed!\r\n");
        return XST_FAILURE;
    }

    XSmul_Start(SmulInst);
    XSmul_EnableAutoRestart(SmulInst);

    return XST_SUCCESS;
};


XStatus DmaSetup(XAxiDma *DmaInsPtr){
    
    XAxiDma_Config *DmaCfg;
    int Status;

    DmaCfg = XAxiDma_LookupConfig(DMA_DEV_ID);
    if(!DmaCfg){
        xil_printf("Dma dev not found!\r\n");
        return XST_FAILURE;
    }

    Status = XAxiDma_CfgInitialize(DmaInsPtr, DmaCfg);
    if(Status != XST_SUCCESS){
        xil_printf("Dma configuration failed!\r\n");
        return XST_FAILURE;
    }

    // Make sure scatter-gather mode is disabled
    if(XAxiDma_HasSg(DmaInsPtr)){
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

// Dma transfer
int DmaTransfer(XAxiDma *DmaInsPtr, u32 *input_buffer, u32 *output_buffer, int data_size){
    int Status;
    int TimeOut = 1000000;  // Time threshold

    Xil_DCacheFlushRange((UINTPTR)input_buffer, data_size * sizeof(u32));
    Xil_DCacheFlushRange((UINTPTR)output_buffer, data_size * sizeof(u32));

    Status = XAxiDma_SimpleTransfer(DmaInsPtr, (UINTPTR)input_buffer, data_size * sizeof(u32), XAXIDMA_DMA_TO_DEVICE);
    if (Status != XST_SUCCESS) {
        xil_printf("DMA transfer to device failed!\r\n");
        return XST_FAILURE;
    }

    Status = XAxiDma_SimpleTransfer(DmaInsPtr, (UINTPTR)output_buffer, data_size * sizeof(u32), XAXIDMA_DEVICE_TO_DMA);
    if (Status != XST_SUCCESS) {
        xil_printf("DMA transfer to device failed!\r\n");
        return XST_FAILURE;
    }

    while (TimeOut) {
        if (!(XAxiDma_Busy(DmaInsPtr, XAXIDMA_DEVICE_TO_DMA)) && !(XAxiDma_Busy(DmaInsPtr, XAXIDMA_DMA_TO_DEVICE))) break; 
        TimeOut--;
        usleep(1U); 
    }

    if (TimeOut == 0) {
        xil_printf("DMA transfer timed out!\r\n");
        return XST_FAILURE; 
    }

    Xil_DCacheInvalidateRange((UINTPTR)output_buffer, data_size * sizeof(u32));

    return XST_SUCCESS;

}

int main(){

    XSmul SmulInst;
    XAxiDma DmaInst;
    u32* input_buffer = (u32*)INPUT_BUFFER;
    u32* output_buffer = (u32*)OUTPUT_BUFFER;

    if(DmaSetup(&DmaInst) != XST_SUCCESS){
        xil_printf("Failed to initialize Dma!\r\n");
        return XST_FAILURE;
    }

    for(int i=0; i<DATA_SIZE; i++){
        input_buffer[i] = i;
    }


    if(smul_start(&SmulInst) != XST_SUCCESS){
        xil_printf("Failed to start smul!\r\n");
        return XST_FAILURE;
    }
    // xil_printf("\r\n");
    // smul_ip_status();
    // xil_printf("\r\n");

    // smul_ip_start();
    
    // xil_printf("\r\n");
    // smul_ip_status();
    // xil_printf("\r\n");

    if(DmaTransfer(&DmaInst, input_buffer, output_buffer, DATA_SIZE) != XST_SUCCESS){
        xil_printf("Dma Transefer failed!\r\n");
        return XST_FAILURE;
    }
    
    for(int i=0; i<DATA_SIZE; ++i){
        xil_printf("Input: %d, Output: %d\r\n", input_buffer[i], output_buffer[i]);
    }

    XSmul_DisableAutoRestart(&SmulInst);

    return XST_SUCCESS;

}
