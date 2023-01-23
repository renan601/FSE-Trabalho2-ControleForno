#include <stdio.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include <string.h>
#include <stdbool.h> 
#include <pthread.h>
#include "crc16.h"
#include "menu.h"
#include "uart.h"
#include "astruct.h"

Oven ovenItem;

int main(int argc, const char * argv[]) {
    define_constants(&ovenItem);

    int workingOption = define_working_mode();
    ovenItem.tempControl = workingOption;
    
    pthread_t menu_id;
    pthread_create(&menu_id, NULL, menu_control_oven, (void *)&ovenItem);
    pthread_join(menu_id, NULL);

    // printf("%d\n", ovenItem.tempControl);
        
    // printf("%f", ovenItem.Kp);
    // printf("%f", ovenItem.Ki);
    // printf("%f", ovenItem.Kd);

    // int uart0_filestream = init_uart();
    
    // for(int i = 0; i < 10; i++ ) {
    //     unsigned char tx_buffer[20] = {0x01, 0x23, 0xC1, 0x0, 0x4, 0x0, 0x3};
    //     short crc_temp = calcula_CRC(tx_buffer, 7);
    //     memcpy(&tx_buffer[7], (const void *)&crc_temp, 2);
    //     usleep(200000);

    //     if (uart0_filestream != -1)
    //     {
    //         printf("Escrevendo caracteres na UART ...");
    //         int count = write(uart0_filestream, &tx_buffer[0], 9);
    //         if (count < 0)
    //         {
    //             printf("UART TX error\n");
    //         }
    //         else
    //         {
    //             printf("escrito.\n");
    //         }
    //     }

    //     usleep(200000);

    //     //----- CHECK FOR ANY RX BYTES -----
    //     if (uart0_filestream != -1)
    //     {
    //         // Read up to 255 characters from the port if they are there
    //         unsigned char rx_buffer[256];
    //         int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);      //Filestream, buffer to store in, number of bytes to read (max)
    //         if (rx_length < 0)
    //         {
    //             printf("Erro na leitura.\n"); //An error occured (will occur if there are no bytes)
    //         }
    //         else if (rx_length == 0)
    //         {
    //             printf("Nenhum dado disponível.\n"); //No data waiting
    //         }
    //         else
    //         {
    //             //Bytes received
    //             //rx_buffer[rx_length] = '\0';
    //             printf("%i Bytes lidos : %s\n", rx_length, rx_buffer);
    //             for(int c = 0; c < rx_length - 1; c++ ) {
    //                 printf(" %c ", rx_buffer[c]);
    //             }
    //         }
    //     }
    // }

    
    // close(uart0_filestream);
    return 0;
}
