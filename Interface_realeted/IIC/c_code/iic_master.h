#ifndef IIC_MASTER_H
#define IIC_MASTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"
#include "xiic.h"
#include "xil_printf.h"

int iic_init(XIic* IicInstPtr, u16 DeviceId);
int iic_write(XIic* IicInstPtr, u8* WriteBuffer, u16 ByteCount);
int iic_read(XIic *IicInstPtr, u8 *WriteBuffer, u8 *BufferPtr, u16 ByteCount);

extern volatile u8 TransmitComplete;
extern volatile u8 ReceiveComplete;

#ifdef __cplusplus
}
#endif

#endif /* IIC_MASTER_H */
