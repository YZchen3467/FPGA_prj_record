#include "xparameters.h"
#include "xil_printf.h"
#include "xil_io.h"

#include "xil_exception.h"
#include "xscugic.h"

#define MUL_TEST_BASE_ADDR      XPAR_XMUL_TEST_0_S_AXI_CONTROL_BASEADDR
#define MUL_TEST_INTR_ID        XPAR_FABRIC_MUL_TEST_0_INTERRUPT_INTR

#define CTRL_REG                (MUL_TEST_BASE_ADDR+0x00)  // Control register
#define AP_START                (MUL_TEST_BASE_ADDR+0x01)
#define AP_DONE                 (MUL_TEST_BASE_ADDR+0x02)
#define AP_IDLE                 (MUL_TEST_BASE_ADDR+0x03)

#define GLOBAL_INTR_EN          (MUL_TEST_BASE_ADDR+0x04)  // Global intr enable
#define IP_INTR_EN              (MUL_TEST_BASE_ADDR+0x08)  // IP intr enable

#define INTR_STATUS_REG         (MUL_TEST_BASE_ADDR+0x0C)  // Intr status
#define IN_R_REG                (MUL_TEST_BASE_ADDR+0x20)  // Regitser Input
#define OUT_R_REG               (MUL_TEST_BASE_ADDR+0x10)  // Regitser Output
#define OUT_R_VALID_REG         (MUL_TEST_BASE_ADDR+0x14)  // Output valid data

// Varibale declearation
volatile int IntrProcessed = 0; 
volatile int IntrCount = 0;    
volatile int input_data = 0;        


// Function prototype
void HLS_InterruptHandler(void *CallbackRef);
int  SetupInterruptSystem(XScuGic *GicInstPtr);
void EnableHLSInterrupt();

void HLS_InterruptHandler(void *CallbackRef) {
    // check intr status
    if (Xil_In32(INTR_STATUS_REG) & 0x1) {
        // clear intr status
        Xil_Out32(INTR_STATUS_REG, 0x1);
        
        // Output data read
        int output_data = Xil_In32(OUT_R_REG);

        xil_printf("Interrupt occurred! Count: %d\r\n", IntrCount + 1);
        xil_printf("Input: %d, Output: %d\r\n", input_data, output_data);

        input_data += 1; 
        IntrCount++;

        if(IntrCount < 10){
            Xil_Out32(IN_R_REG, input_data);
            Xil_Out32(CTRL_REG, 0x1);
        }else{
            IntrProcessed = 1;
        }
        
        xil_printf("Hardware accelerator done!\r\n");
    }
}

int SetupInterruptSystem(XScuGic *GicInstPtr) {
    XScuGic_Config *GicConfig;
    int Status;
    
    GicConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
    if (GicConfig == NULL) {
        return XST_FAILURE;
    }
    
    Status = XScuGic_CfgInitialize(GicInstPtr, GicConfig, GicConfig->CpuBaseAddress);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler, GicInstPtr);
    Xil_ExceptionEnable();
    
    Status = XScuGic_Connect(GicInstPtr, MUL_TEST_INTR_ID,
                             (Xil_InterruptHandler)HLS_InterruptHandler, NULL);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    
    XScuGic_Enable(GicInstPtr, MUL_TEST_INTR_ID);
    
    return XST_SUCCESS;
}

void EnableHLSInterrupt() {
    Xil_Out32(GLOBAL_INTR_EN, 0x1);
    
    Xil_Out32(IP_INTR_EN, 0x1);
}

int main(){
    int Status;

    XScuGic IntrCtrl;

    Status = SetupInterruptSystem(&IntrCtrl);
    if(Status != XST_SUCCESS){
        xil_printf("Intr setup failed\r\n");
        return XST_FAILURE;
    }

    // Enable the HLS intr
    EnableHLSInterrupt();

    input_data = 0;

    Xil_Out32(IN_R_REG, input_data);
    Xil_Out32(CTRL_REG, 0x1);

    // Wait the intr occuring 10 times intr
    while (!IntrProcessed);

    xil_printf("Computation completed after 10 interrupts!\r\n");

    return 0;
}
