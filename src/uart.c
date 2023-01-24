#include <stdio.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include <string.h>
#include "crc16.h"
#include "astruct.h"
#include "pid.h"


int init_uart() {
    int uart0_filestream = -1;

    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);      //Open in non blocking read/write mode
    if (uart0_filestream == -1) {
        printf("Erro - Não foi possível iniciar a UART.\n");
    }
    else {
        printf("UART inicializada!\n");
    }    
    
    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;     //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    printf("Buffers de memória criados!\n");

    return uart0_filestream;
}

void write_uart_code23(int uartfilestream, unsigned char subcode) {
    unsigned char tx_buffer[20] = {0x01, 0x23, subcode, 0x0, 0x4, 0x0, 0x3};
    short crc_code = calcula_CRC(tx_buffer, 7);
    memcpy(&tx_buffer[7], (const void *)&crc_code, 2);

    if (uartfilestream != -1) {
        printf("Escrevendo caracteres na UART ...");
        int count = write(uartfilestream, &tx_buffer[0], 9);
        if (count < 0) {
            printf("UART TX error\n");
        }
        else {
            printf("Escrito.\n");
        } 
    }
}

void write_uart_code16(int uartfilestream, unsigned char subcode, unsigned char* data) {
    if ((subcode == 0xD3) || (subcode == 0xD4) || (subcode == 0xD5)){
        unsigned char tx_buffer[20] = {0x01, 0x16, subcode, 0x0, 0x4, 0x0, 0x3, data[0]};
        short crc_code = calcula_CRC(tx_buffer, 8);
        memcpy(&tx_buffer[8], (const void *)&crc_code, 2);

        if (uartfilestream != -1) {
            printf("Enviando status de funcionamento pra UART ...");
            int count = write(uartfilestream, &tx_buffer[0], 10);
            if (count < 0) {
                printf("UART TX error\n");
            }
            else {
                printf("Escrito.\n");
            } 
        }
    }
}

void send_float_uart(int uartfilestream, unsigned char subcode, float data) {
    if ((subcode == 0xD6) || (subcode == 0xD2)) {
        unsigned char tx_buffer[20] = {0x01, 0x16, subcode, 0x0, 0x4, 0x0, 0x3};
        memcpy(&tx_buffer[7], (const void *)&data, 4);
        
        short crc_code = calcula_CRC(tx_buffer, 11);
        memcpy(&tx_buffer[11], (const void *)&crc_code, 2);

        if (uartfilestream != -1) {
            printf("Enviando float para UART %f...", data);
            int count = write(uartfilestream, &tx_buffer[0], 13);
            if (count < 0) {
                printf("UART TX error\n");
            }
            else {
                printf("Escrito.\n");
            }
        }
    }
}

void send_control_uart(int uartfilestream, unsigned char subcode, int data) {
    if (subcode == 0xD1) {
        unsigned char tx_buffer[20] = {0x01, 0x16, subcode, 0x0, 0x4, 0x0, 0x3};
        memcpy(&tx_buffer[7], (const void *)&data, 4);
        
        short crc_code = calcula_CRC(tx_buffer, 11);
        memcpy(&tx_buffer[11], (const void *)&crc_code, 2);

        if (uartfilestream != -1) {
            int count = write(uartfilestream, &tx_buffer[0], 13);
            if (count < 0) {
                printf("UART TX error\n");
            }
            else {
                printf("Escrito.\n");
            }
        }
    }
}

void modify_oven_status(int command, Oven *ovenItem) {
    unsigned char turnOn[4] = {0x01, 0, 0, 0};
    unsigned char turnOff[4] = {0x00, 0, 0, 0};
    
    if (command == 161) {
        write_uart_code16(ovenItem->uart_filestream, 0xD3, turnOn);
    }
    else if (command == 162) {
        write_uart_code16(ovenItem->uart_filestream, 0xD3, turnOff);
    }
    else if (command == 163) {
        write_uart_code16(ovenItem->uart_filestream, 0xD5, turnOn);
    }
    else if (command == 164) {
        write_uart_code16(ovenItem->uart_filestream, 0xD5, turnOff);
    }
    else if (command == 165) {
        write_uart_code16(ovenItem->uart_filestream, 0xD4, turnOff);
    }
}

void translate_codes(int rx_length, unsigned char* rx_buffer, Oven *ovenItem){
    unsigned char code;

    for (int i = 0; i < rx_length; i++) {
        float temperature;
        int command;
        int state;
        
        code = rx_buffer[i];

        if (i+1 < rx_length) {

            if (code == 0xC1) {
                memcpy(&temperature, &rx_buffer[i+1], 4);
                printf("Temperatura Interna: %f \n", temperature);
                ovenItem->internalTemp = temperature;
            }
            else if (code == 0xC2) {
                memcpy(&temperature, &rx_buffer[i+1], 4);
                printf("Temperatura Referencia: %f \n", temperature);
                ovenItem->referenceTemp = temperature;
                pid_atualiza_referencia(ovenItem->referenceTemp);
            }
            else if (code == 0xC3) {
                memcpy(&command, &rx_buffer[i+1], 4);
                printf("Comando: %d \n", command);
                if (command > 0){
                    modify_oven_status(command, ovenItem);
                }
            }
            else if (code == 0xD3) {
                memcpy(&state, &rx_buffer[i+1], 4);
                printf("Estado Do Forno: %d \n", state);
                ovenItem->ovenStatus = state;
            }
            else if (code == 0xD4) {
                memcpy(&state, &rx_buffer[i+1], 4);
                printf("Dash 0 - Curva - 1: %d \n", state);
                ovenItem->tempControl = state;
            }
            else if (code == 0xD5) {
                memcpy(&state, &rx_buffer[i+1], 4);
                printf("Aquecimento: %d \n", state);
                ovenItem->heatingStatus = state;
            }
        }
    }
}

// int check_crc(unsigned char* rx_buffer, int begin, int totalSize) {
//     unsigned char command;
//     memcpy(&command, &rx_buffer[begin], totalSize);
// }

void listen_uart(Oven *ovenItem) {
    printf("Uart: %d", ovenItem->uart_filestream);
    while (true) {
        if (ovenItem->turnOff == 1){
            break;
        }
        if (ovenItem->uart_filestream != -1) {
            usleep(100000);
            // Read up to 255 characters from the port if they are there
            unsigned char rx_buffer[256];
            int rx_length = read(ovenItem->uart_filestream, (void*)rx_buffer, 255); //Filestream, buffer to store in, number of bytes to read (max)
            if (rx_length < 0) {
                printf("Erro na leitura.\n"); //An error occured (will occur if there are no bytes)
            }
            else if (rx_length > 0) {
                //Bytes received
                printf("%i Bytes lidos\n", rx_length);
                translate_codes(rx_length, rx_buffer, ovenItem);
            }
        }
    }
    printf("Encerrando thread que escuta a UART.\n");
}

void search_for_user_commands(Oven *ovenItem) {
    printf("Uart: %d", ovenItem->uart_filestream);
    while (true) {
        usleep(1000000);
        unsigned char tx_buffer[20] = {0x01, 0x23, 0xC3, 0x0, 0x4, 0x0, 0x3};
        short crc_code = calcula_CRC(tx_buffer, 7);
        memcpy(&tx_buffer[7], (const void *)&crc_code, 2);
        
        if (ovenItem->turnOff == 1){
            break;
        }

        if (ovenItem->uart_filestream != -1) {
            int count = write(ovenItem->uart_filestream, &tx_buffer[0], 9);
            if (count < 0) {
                printf("Não foi possível solicitar comandos de usuário\n");
            }
        }
    }
    printf("Encerrando thread para solicitar comandos.\n");
}