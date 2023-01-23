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
#include "pid.h"
#include "bme280.h"

Oven ovenItem;
struct bme280_dev dev;

int main(int argc, const char * argv[]) {
    // Opções Terminal
    define_constants(&ovenItem);
    int workingOption = define_working_mode();
    ovenItem.tempControl = workingOption;
    
    // Inicia I2C e BME280
    ovenItem.i2c_stream = initialize_i2c(&dev);
    uint32_t req_delay = bme280_init(&dev);

    // Inicia UART
    ovenItem.uart_filestream = init_uart();
    

    pthread_t user_commands_id;
    pthread_t listen_uart_id;
    
    pthread_create(&user_commands_id, NULL, (void *)search_for_user_commands, (void *)&ovenItem);
    pthread_create(&listen_uart_id, NULL, (void *)listen_uart, (void *)&ovenItem);

    unsigned char data[4] = {0x01, 0, 0, 0};
    write_uart_code16(ovenItem.uart_filestream, 0xD3, data);
    usleep(1000000);
    write_uart_code16(ovenItem.uart_filestream, 0xD5, data);
    usleep(1000000);
    data[0] = 0x00;
    write_uart_code16(ovenItem.uart_filestream, 0xD4, data);
    pid_atualiza_referencia(50.0);
    write_uart_code23(ovenItem.uart_filestream, 0xC2);

    double controlSignal;
    double envTemp;
    
    for(int i = 0; i < 10; i++) {
        write_uart_code23(ovenItem.uart_filestream, 0xC1);
        envTemp = stream_sensor_data_forced_mode(&dev, req_delay);
        usleep(1000000);
        controlSignal = pid_controle((double) ovenItem.internalTemp);
        printf("Sinal De Controle: %lf\n", controlSignal);
        printf("Temperatura Ambiente: %lf\n", envTemp);
    }
    
    pthread_join(user_commands_id, NULL);
    pthread_join(listen_uart_id, NULL);


    data[0] = 0x00;
    write_uart_code16(ovenItem.uart_filestream, 0xD5, data);
    usleep(1000000);
    write_uart_code16(ovenItem.uart_filestream, 0xD3, data);
    
    close(ovenItem.uart_filestream);
    printf("UART Filestream: %d", ovenItem.uart_filestream);
    return 0;
}
