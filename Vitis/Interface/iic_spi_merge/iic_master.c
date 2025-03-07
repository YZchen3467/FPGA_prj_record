#include "iic_master.h"

volatile u8 TransmitComplete;  
volatile u8 ReceiveComplete;  

int iic_init(XIic* IicInstPtr, u16 DeviceId) {
    int Status;
    XIic_Config *ConfigPtr;

    ConfigPtr = XIic_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        xil_printf("IIC LookupConfig failed for Device ID: %d\r\n", DeviceId);
        return XST_FAILURE;
    }

    Status = XIic_CfgInitialize(IicInstPtr, ConfigPtr, ConfigPtr->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("IIC CfgInitialize failed for Device ID: %d\r\n", DeviceId);
        return XST_FAILURE;
    }

    xil_printf("IIC initialized successfully with Device ID: %d\r\n", DeviceId);
    return XST_SUCCESS;
}

int iic_write(XIic* IicInstPtr, u8* WriteBuffer, u16 ByteCount) {
    int Status;

    TransmitComplete = 1;
    IicInstPtr->Stats.TxErrors = 0;

    Status = XIic_Start(IicInstPtr);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    Status = XIic_MasterSend(IicInstPtr, WriteBuffer, ByteCount);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    while ((TransmitComplete) || (XIic_IsIicBusy(IicInstPtr) == TRUE)) {
        if (IicInstPtr->Stats.TxErrors != 0) {
            Status = XIic_Start(IicInstPtr);
            if (Status != XST_SUCCESS) {
                return XST_FAILURE;
            }
            if (!XIic_IsIicBusy(IicInstPtr)) {
                Status = XIic_MasterSend(IicInstPtr, WriteBuffer, ByteCount);
                if (Status == XST_SUCCESS) {
                    IicInstPtr->Stats.TxErrors = 0;
                }
            }
        }
    }

    Status = XIic_Stop(IicInstPtr);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

int iic_read(XIic *IicInstPtr, u8 *WriteBuffer, u8 *BufferPtr, u16 ByteCount) {
    int Status;

    ReceiveComplete = 1;

    Status = iic_write(IicInstPtr, WriteBuffer, 1);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    Status = XIic_Start(IicInstPtr);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    Status = XIic_MasterRecv(IicInstPtr, BufferPtr, ByteCount);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    while ((ReceiveComplete) || (XIic_IsIicBusy(IicInstPtr) == TRUE)) {}

    Status = XIic_Stop(IicInstPtr);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}
