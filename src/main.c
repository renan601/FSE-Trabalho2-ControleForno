#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include <string.h>
#include <stdbool.h> 
#include <pthread.h>
#include <signal.h>
#include "crc16.h"
#include "menu.h"
#include "uart.h"
#include "astruct.h"
#include "bme280.h"
#include "i2c.h"
#include "tempcontrol.h"
#include "pid.h"

Oven ovenItem;
struct bme280_dev dev;
struct identifier id;
int reflowMatrix[100][2];

double time_spent;
clock_t begin;
clock_t end;

void deactivate() {
    ovenItem.ovenStatus = 0;
    ovenItem.heatingStatus = 0;
    ovenItem.turnOff = 1;

    unsigned char turnOff[4] = {0x00, 0, 0, 0};
    write_uart_code16(ovenItem.uart_filestream, 0xD3, turnOff);
    write_uart_code16(ovenItem.uart_filestream, 0xD5, turnOff);

    usleep(500000);
    activate_devices(0);

    close(id.fd);

    close(ovenItem.uart_filestream);
    printf("Encerrando conexões\n");

    close(ovenItem.uart_filestream);
    exit(0);
}

// void curve_control_temperature() {
//     printf("Time: %f\n", (float) time_spent);

//     for (int i = 1; i < 100; i++){
//         if ( ((int) time_spent > reflowMatrix[i][0]) && ((int) time_spent < reflowMatrix[i+1][0]) ){
//             printf("Referencia:%d - %f", i, (float) reflowMatrix[i][1]);
//             send_float_uart(ovenItem.uart_filestream, 0xD2, (float) reflowMatrix[i][1]);
//             break;
//         }
//     }

//     write_uart_code23(ovenItem.uart_filestream, 0xC1);
//     usleep(500000);

//     double envTemp = le_temp_bme280_i2c(&dev);
//     ovenItem.externalTemp = (float) envTemp;
    
//     double controlSignal = pid_controle((double) ovenItem.internalTemp);
//     if ((controlSignal > -40) && (controlSignal < 0)) {
//         controlSignal = -40;
//     }
//     activate_devices((int) controlSignal);
//     usleep(500000);
//     send_control_uart(ovenItem.uart_filestream, 0xD1, (int) controlSignal);
    
//     send_float_uart(ovenItem.uart_filestream, 0xD6, (float) envTemp);

//     printf("Temperatura Ambiente: %lf\n", envTemp);
// }

// void remove_spaces(char* s) {
//     char* d = s;
//     do {
//         while (*d == ' ') {
//             ++d;
//         }
//     } while (*s++ = *d++);
// }

// void process_csv(){
//     FILE *fp;
//     char row[100];
//     char *token;

//     int countRows = 0;
//     int countColumns = 0;

//     memset(reflowMatrix, 0, sizeof(reflowMatrix));
    
//     fp = fopen("../curva_reflow.csv","r");


//     while (feof(fp) != true) {
//         fgets(row, 100, fp);

//         token = strtok(row, ",");

//         countColumns = 0;
//         while(token != NULL) {
//             remove_spaces(token);
//             reflowMatrix[countRows][countColumns] = atoi (token);
//             token = strtok(NULL, ",");
//             countColumns++;
//         }

//         countRows++;
//     }

// }

int main(int argc, const char * argv[]) {
    signal(SIGINT, deactivate);

    // Variável que indica que o programa será encerrado
    ovenItem.turnOff = 0;

    // Opções Terminal
    define_constants(&ovenItem);
    int workingOption = define_working_mode();
    ovenItem.tempControl = 0;
    float usertemp = 0;
    if (workingOption == 2) {
        printf("Insira a temperatura de referencia desejada ");
        scanf("%f",&usertemp);
    }
    
    // Inicia I2C e BME280
    monta_i2c(&dev, &id);
    abre_i2c(&dev, &id);
    inicializa_bme280_i2c(&dev);
    configura_bme280_i2c(&dev);

    // Inicia GPIO
    init_GPIO();

    // Inicia UART
    ovenItem.uart_filestream = init_uart();
    
    pthread_t user_commands_id;
    pthread_t listen_uart_id;
    
    pthread_create(&user_commands_id, NULL, (void *)search_for_user_commands, (void *)&ovenItem);
    pthread_create(&listen_uart_id, NULL, (void *)listen_uart, (void *)&ovenItem);

    unsigned char turnOff[4] = {0x00, 0, 0, 0};
    write_uart_code16(ovenItem.uart_filestream, 0xD3, turnOff);
    write_uart_code16(ovenItem.uart_filestream, 0xD5, turnOff);
    
    if (usertemp != 0) {
        send_float_uart(ovenItem.uart_filestream, 0xD2, usertemp);
    }
    usleep(1000000);
    write_uart_code23(ovenItem.uart_filestream, 0xC2);
    usleep(1000000);
    
    while (true) {
        // Desligar imediatamente
        if (ovenItem.turnOff == 1){
            break;
        }
        
        // Forno está ligado e aquecendo/esfriando?
        if ((ovenItem.ovenStatus == 1) && (ovenItem.heatingStatus == 1)) {
            if (ovenItem.tempControl == 0) {
                write_uart_code23(ovenItem.uart_filestream, 0xC1);
                usleep(500000);
                write_uart_code23(ovenItem.uart_filestream, 0xC2);
                double envTemp = le_temp_bme280_i2c(&dev);
                ovenItem.externalTemp = (float) envTemp;
                
                double controlSignal = pid_controle((double) ovenItem.internalTemp);
                if ((controlSignal > -40) && (controlSignal < 0)) {
                    controlSignal = -40;
                }
                activate_devices((int) controlSignal);
                usleep(500000);
                send_control_uart(ovenItem.uart_filestream, 0xD1, (int) controlSignal);
                
                send_float_uart(ovenItem.uart_filestream, 0xD6, (float) envTemp);

                printf("Temperatura Ambiente: %lf\n", envTemp);
            }
            // else if (ovenItem.tempControl == 1) {
            //     curve_control_temperature();
            // }
        }
        else {
            activate_devices(0);
            usleep(1000000);
        }
        //end = clock();
        //time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    }
    
    printf("Encerrando programa");
    
    pthread_join(user_commands_id, NULL);
    pthread_join(listen_uart_id, NULL);
    
    return 0;
}