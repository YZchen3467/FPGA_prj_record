#include "xparameters.h"
#include "xil_printf.h"
#include "xuartlite.h"
#include "sleep.h"

char uart_getchar();
u32 read_hex_input(const char *prompt, int max_digits);
u8 read_single_bit(const char *prompt, int max_value);

char uart_getchar(){
    return inbyte();
}

u32 read_hex_input(const char *prompt, int max_digits){
    char input_buffer[9] = {0};
    int i = 0;
    u32 result = 0;

    while (1){
        i = 0;
        xil_printf("%s", prompt);

        // Input value constraint
        while (i < max_digits) { 
            char c = uart_getchar();

            // Backspace process
            if(c == '\b'){                    
                if(i>0) {
                    i--;
                    input_buffer[i] = '\0';
                    xil_printf("\b \b");
                }
                continue;
            }

            // Valid input process
            if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')){
                xil_printf("%c", c);
                input_buffer[i++] = c;
            } else {
                xil_printf("Invalid input, please input a valid hex digit (0-9, A-F).\r\n");
                i = 0;
                break;
            }
        }

        char next_char = uart_getchar();
        if(next_char == '\r' || next_char == '\n'){
            xil_printf("\r\n");
            break;
        } else {
            xil_printf("Invalid input. Please press Enter after entering the hex digits.\r\n");
        }
    }

    // Symbol transfer
    result = 0;        
    for(int j=0; j<i; j++){
        result <<= 4;
        if(input_buffer[j] >= '0' && input_buffer[j] <= '9')
            result += input_buffer[j] - '0';
        else if(input_buffer[j] >= 'A' && input_buffer[j] <= 'F')
            result += input_buffer[j] - 'A' + 10;
        else if(input_buffer[j] >= 'a' && input_buffer[j] <= 'f')
            result += input_buffer[j] - 'a' + 10;
     }
    return result;
}

u8 read_single_bit(const char *prompt, int max_value){
    u8 result = 0;
    while (1) {
        xil_printf("%s", prompt);
        char c = uart_getchar();
        xil_printf("%c", c);

        if(c == '\r' || c == '\n') {
            xil_printf("Invalid input. Please enter a value from 0 to 1, then press enter\r\n");
            continue;
        }
        result = c - '0';

        if (result <= max_value) {
            char next_char = uart_getchar();
            if(next_char == '\r' || next_char == '\n'){
                xil_printf("\r\n");
                break;
            } else xil_printf("Invalid input. Please press Enter after entering a value.\r\n");
        } else xil_printf("Invalid input. Please enter a value from 0 to 1.\r\n");
        
    }
    return result;
}