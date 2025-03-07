#ifndef SPI_MASTER_H
#define SPI_MASTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"
#include "xspi.h"
#include "xil_printf.h"

int spi_init(XSpi* SpiInstPtr, u16 DeviceId);
int spi_write(XSpi* SpiInstPtr, u8* WriteBuffer, u16 ByteCount, u8 cs_n);
// int spi_read(XSpi *SpiInstPtr, u8 *WriteBuffer, u8 *BufferPtr, u16 ByteCount);

#ifdef __cplusplus
}
#endif

#endif /* SPI_MASTER_H */
