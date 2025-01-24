#include "xil_printf.h"
#include "xparameters.h"
#include "xiic.h"
#include "xil_exception.h"
#include "xintc.h"
#include "xil_cache.h"
#include <stdint.h>
#include "iic_master.h"
#include "platform.h"
#include "sleep.h"

/***** Device ID *****/
#define IIC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID
#define INTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
#define IIC_INTR_ID		XPAR_INTC_0_IIC_0_VEC_ID

/***** IIC control Device Addr *****/
#define IIC_MUX_ADDRESS 		0x74
#define IIC_EEPROM_CHANNEL		0x01
#define IIC_DDR_CHANNEL		    0x80

#define EEPROM_ADDRESS 			0x54
#define DDR_DEVICE_ADDRESS 		0x51	

/***** Maximum of data can receive by eeprom per  *****/
#define PAGE_SIZE               16

/***** Start address of eeprom *****/
#define EEPROM_DATA_START_ADDR   128

/***** Start address of DDR *****/
#define DDR_BASE_ADDR            XPAR_DDR4_0_C0_DDR4_MEMORY_MAP_BASEADDR
#define BUFFER_OFFSET            0x00000000                 // 4KB offset
// #define SIZE                  (9 * 1024 / 4)
#define SIZE                     4
#define CACHE_LINE_SIZE          16   

static int  SetupInterruptSystem(XIic *IicInstPtr, XIntc *IntcInstPtr);
static void SendHandler(XIic *InstancePtr);
static void ReceiveHandler(XIic *InstancePtr);
static void StatusHandler(XIic *InstancePtr, int Event);

int main(void){

    init_platform();
    xil_printf("Hello MIG exdes\n\r");

	XIntc IntcInstPtr;
	XIic IicInstPtr;
	int Status;
    u64 i;                   // Test data index
   	volatile u64* ddr_addr;

    // Enable the cache
    Xil_DCacheEnable();

    Status = iic_init(&IicInstPtr, IIC_DEVICE_ID);
    if(Status != XST_SUCCESS){
		xil_printf("IIC init failed\r\n");
		return XST_FAILURE;
	}

    Status = SetupInterruptSystem(&IicInstPtr, &IntcInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
    XIic_SetSendHandler(&IicInstPtr, &IicInstPtr,
				(XIic_Handler) SendHandler);
	XIic_SetRecvHandler(&IicInstPtr, &IicInstPtr,
				(XIic_Handler) ReceiveHandler);
	XIic_SetStatusHandler(&IicInstPtr, &IicInstPtr,
				  (XIic_StatusHandler) StatusHandler);

    /* Data Write */
    ddr_addr = (u64 *)(DDR_BASE_ADDR + BUFFER_OFFSET);
    
	// u32 buffer[8] = {0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777};

    for(i = 0; i < SIZE; i=i+2){
         *(ddr_addr+i) = 0x0000000000000000 + i*0x1111111111111111;
//		*(ddr_addr+i) = ((u64)buffer[i]<<32) | buffer[i+1];
    }
//     Xil_DCacheFlushRange((UINTPTR)ddr_addr, sizeof(u64) * SIZE);

//     Xil_DCacheInvalidateRange((UINTPTR)ddr_addr, sizeof(u64) * SIZE);
    
    volatile u64* read_ptr = (u64 *)(DDR_BASE_ADDR + BUFFER_OFFSET);
    for (i = 0; i < 10; i++) {
		xil_printf("Data[%d] = %llx\r\n", i, read_ptr[i]);
		// if(read_ptr[i] != i*2){
		//     xil_printf("ERROR at Index %d: Expected %d, Got %d\r\n", i, i*2, read_ptr[i]);
		//     // return XST_FAILURE;
		// }
	}

    Xil_DCacheDisable();
    cleanup_platform();
    return 0;
}

static int SetupInterruptSystem(XIic *IicInstPtr, XIntc *IntcInstPtr)
{
	int Status;

	// Initialize the interrupt controller driver so that it is ready to use.
	Status = XIntc_Initialize(IntcInstPtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("AXI Intc initialization failed\r\n");
		return XST_FAILURE;
	}

	// Connect a device driver handler that will be called when an interrupt 
	// for the device occurs, the device driver handler performs 
	// the specific interrupt processing for the device.
	Status = XIntc_Connect(IntcInstPtr, 
							IIC_INTR_ID,
				   			(XInterruptHandler) XIic_InterruptHandler,
				   			IicInstPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("IIC Intc connect failed\r\n");
		return XST_FAILURE;
	}

	// Start the interrupt controller such that interrupts are enabled 
	// for all devices that cause interrupts.
	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Enable the interrupt for the IIC device.
	XIntc_Enable(IntcInstPtr, IIC_INTR_ID);

	// Initialize the exception table.
	Xil_ExceptionInit();

	// Register the interrupt controller handler with the exception table.
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 					(Xil_ExceptionHandler)XIntc_InterruptHandler, 
								IntcInstPtr);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

static void SendHandler(XIic *InstancePtr)
{
	TransmitComplete = 0;
}

static void ReceiveHandler(XIic *InstancePtr)
{
	ReceiveComplete = 0;
}

static void StatusHandler(XIic *InstancePtr, int Event)
{

}
