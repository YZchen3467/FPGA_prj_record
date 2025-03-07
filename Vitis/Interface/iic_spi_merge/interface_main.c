#include "xparameters.h"
#include "xiic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xintc.h"
#include "iic_master.h"
#include "spi_master.h"

/***** Device ID *****/
#define IIC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID
#define SPI_DEVICE_ID	XPAR_SPI_0_DEVICE_ID

#define INTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
#define IIC_INTR_ID		XPAR_INTC_0_IIC_0_VEC_ID
#define SPI_INTR_ID		XPAR_INTC_0_SPI_0_VEC_ID

/***** IIC control Device Addr *****/
#define IIC_MUX_ADDRESS 		0x74
#define IIC_EEPROM_CHANNEL		0x01

#define EEPROM_ADDRESS 			0x54	

/***** Maximum of data can receive by eeprom per  *****/
#define PAGE_SIZE   16

/***** Start address of eeprom *****/
#define EEPROM_DATA_START_ADDR   128

static int SetupInterruptSystem(XIic *IicInstPtr, XIntc *IntcInstPtr);
static void SendHandler(XIic *InstancePtr);
static void ReceiveHandler(XIic *InstancePtr);
static void StatusHandler(XIic *InstancePtr, int Event);


int main(void)
{
	int Status;

	XIntc IntcInstPtr;
	XIic IicInstPtr;
	XSpi SpiInstPtr;

	u8 WriteBuffer[1 + PAGE_SIZE]; 
	u8 ReadBuffer[PAGE_SIZE]; 
	u8 SpiWriteBuffer[PAGE_SIZE] = {0xA1, 0xB9, 0x36, 0x78, 0x05, 0x06, 0x07, 
									0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 
									0x0F, 0x10};


	/*
	 * Initialize the IIC and SPI driver so that it is ready to use.
	 */
	Status = iic_init(&IicInstPtr, IIC_DEVICE_ID);
	if(Status != XST_SUCCESS){
		xil_printf("IIC init failed\r\n");
		return XST_FAILURE;
	}
	Status = spi_init(&SpiInstPtr, SPI_DEVICE_ID);
	if(Status != XST_SUCCESS){
		xil_printf("SPI init failed\r\n");
		return XST_FAILURE;
	}

	// Intr session
	/*
	 * Setup the Interrupt System.
	 */
	Status = SetupInterruptSystem(&IicInstPtr, &IntcInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Set the Handlers for transmit and reception.
	 */
	XIic_SetSendHandler(&IicInstPtr, &IicInstPtr,
				(XIic_Handler) SendHandler);
	XIic_SetRecvHandler(&IicInstPtr, &IicInstPtr,
				(XIic_Handler) ReceiveHandler);
	XIic_SetStatusHandler(&IicInstPtr, &IicInstPtr,
				  (XIic_StatusHandler) StatusHandler);


	// Set mux active the eeprom channel
	Status = XIic_SetAddress(&IicInstPtr, XII_ADDR_TO_SEND_TYPE,
				 IIC_MUX_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	WriteBuffer[0] = IIC_EEPROM_CHANNEL;
	Status = iic_write(&IicInstPtr, WriteBuffer, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Set EEPROM device and write/read opration
	Status = XIic_SetAddress(&IicInstPtr, XII_ADDR_TO_SEND_TYPE, EEPROM_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	// Write data
	for(int Page=0; Page < 1; Page++){
		WriteBuffer[0] = EEPROM_DATA_START_ADDR + (Page * PAGE_SIZE);
		for (int Index = 0; Index < PAGE_SIZE; Index++) {
			WriteBuffer[1 + Index] = Index + Page * PAGE_SIZE;
			ReadBuffer[Index] = 0;
		}
		Status = iic_write(&IicInstPtr, WriteBuffer, 1 + PAGE_SIZE);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}	
	}

	// Read data
	Status = XIic_SetAddress(&IicInstPtr, XII_ADDR_TO_SEND_TYPE, EEPROM_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	for(int Page=0; Page < 1; Page++){
		WriteBuffer[0] = EEPROM_DATA_START_ADDR + (Page * PAGE_SIZE);
        Status = iic_read(&IicInstPtr, WriteBuffer, ReadBuffer, PAGE_SIZE);
        if (Status != XST_SUCCESS) {
            xil_printf("EEPROM Read Failed at Page %d\r\n", Page);
            return XST_FAILURE;
        }
        for (int Index = 0; Index < PAGE_SIZE; Index++) {
            xil_printf("ReadBuffer[%d] = 0x%02X\r\n", Page * PAGE_SIZE + Index, ReadBuffer[Index]);
            if (ReadBuffer[Index] != Index + (Page * PAGE_SIZE)) {
                xil_printf("Data mismatch at Index %d\r\n", Page * PAGE_SIZE + Index);
                return XST_FAILURE;
            }
        }
		xil_printf("Page %d read successfully.\r\n", Page);
	}

	xil_printf("Successfully ran IIC eeprom Example\r\n");

	Status = spi_write(&SpiInstPtr, SpiWriteBuffer, PAGE_SIZE, 0x04);
	if(Status != XST_SUCCESS){
		xil_printf("SPI write failed\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
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

