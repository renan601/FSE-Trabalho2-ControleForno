#ifndef UART_H_
#define UART_H_
#include "astruct.h"

int init_uart();
void listen_uart(Oven *ovenItem);
void translate_codes(int rx_length, unsigned char* rx_buffer, Oven *ovenItem);
void write_uart_code23(int uartfilestream, unsigned char subcode);
void write_uart_code16(int uartfilestream, unsigned char subcode, unsigned char* data);
void search_for_user_commands(Oven *ovenItem);


#endif /* UART_H_ */
