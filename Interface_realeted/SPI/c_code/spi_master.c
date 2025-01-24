#include "spi_master.h"

int spi_init(XSpi* SpiInstPtr, u16 DeviceId) {
    int Status;
    XSpi_Config *ConfigPtr;

    ConfigPtr = XSpi_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        xil_printf("SPI LookupConfig failed for Device ID: %d\r\n", DeviceId);
        return XST_FAILURE;
    }

    Status = XSpi_CfgInitialize(SpiInstPtr, ConfigPtr, ConfigPtr->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("SPI CfgInitialize failed for Device ID: %d\r\n", DeviceId);
        return XST_FAILURE;
    }
    
    Status = XSpi_SelfTest(SpiInstPtr);
    if (Status != XST_SUCCESS) {
        xil_printf("SPI SelfTest failed for Device ID: %d\r\n", DeviceId);
        return XST_FAILURE;
    }

    xil_printf("SPI initialized successfully with Device ID: %d\r\n", DeviceId);
    return XST_SUCCESS;
}

int spi_write(XSpi* SpiInstPtr, u8* WriteBuffer, u16 ByteCount, u8 cs_n) {
    
    int Status;

    /*
	 * Run loopback test only in case of standard SPI mode.
	 */
    if(SpiInstPtr->SpiMode != XSP_STANDARD_MODE) {
        xil_printf("SPI is not in standard mode. Loopback test is not possible.\r\n");
        return XST_FAILURE;
    }

	Status = XSpi_SetOptions(SpiInstPtr, XSP_MASTER_OPTION |
 					XSP_LOOPBACK_OPTION);
	if (Status != XST_SUCCESS) return XST_FAILURE;

    Status = XSpi_SetSlaveSelect(SpiInstPtr, cs_n);
    if (Status != XST_SUCCESS) return XST_FAILURE;
    
    Status = XSpi_Start(SpiInstPtr);
    if (Status != XST_SUCCESS) return XST_FAILURE;

    XSpi_IntrGlobalDisable(SpiInstPtr);

    Status = XSpi_Transfer(SpiInstPtr, WriteBuffer, NULL, ByteCount);
    if (Status != XST_SUCCESS) return XST_FAILURE;

    Status = XSpi_Stop(SpiInstPtr);
    if (Status != XST_SUCCESS) return XST_FAILURE;

    return XST_SUCCESS;
}

// int spi_write(XSpi* SpiInstPtr, u8* WriteBuffer, u16 ByteCount, u8 cs_n) {
    
//     int Status;

//     /*
// 	 * Run loopback test only in case of standard SPI mode.
// 	 */
//     if(SpiInstPtr->SpiMode != XSP_STANDARD_MODE) {
//         xil_printf("SPI is not in standard mode. Loopback test is not possible.\r\n");
//         return XST_FAILURE;
//     }

// 	Status = XSpi_SetOptions(SpiInstPtr, XSP_MASTER_OPTION |
//  					XSP_LOOPBACK_OPTION);
// 	if (Status != XST_SUCCESS) return XST_FAILURE;

//     Status = XSpi_SetSlaveSelect(SpiInstPtr, cs_n);
//     if (Status != XST_SUCCESS) return XST_FAILURE;
    
//     Status = XSpi_Start(SpiInstPtr);
//     if (Status != XST_SUCCESS) return XST_FAILURE;

//     XSpi_IntrGlobalDisable(SpiInstPtr);

//     Status = XSpi_Transfer(SpiInstPtr, WriteBuffer, NULL, ByteCount);
//     if (Status != XST_SUCCESS) return XST_FAILURE;

//     Status = XSpi_Stop(SpiInstPtr);
//     if (Status != XST_SUCCESS) return XST_FAILURE;

//     return XST_SUCCESS;
// }
