#ifndef ASTRUCT_H_
#define ASTRUCT_H_
#include <stdbool.h>

typedef struct oven {
    bool ovenStatus;
    bool heatingStatus;
    short int tempControl;

    float referenceTemp;
    float internalTemp;
    float externalTemp;

    float Kp;
    float Ki;
    float Kd;

    int uart_filestream;
    int i2c_stream;
} Oven;

#endif /* ASTRUCT_H_ */