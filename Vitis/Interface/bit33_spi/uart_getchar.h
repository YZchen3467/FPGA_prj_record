#ifndef UART_GETCHAR_H
#define UART_GETCHAR_H

#include "xuartlite.h"
#include "xparameters.h"

#ifdef __cplusplus
extern "C" {
#endif

char uart_getchar();  // Just for sigle input

u32 read_hex_input(const char *prompt, int max_digits);

u8 read_single_bit(const char *prompt, int max_value);

#ifdef __cplusplus
}
#endif

#endif  // UART_GETCHAR_H