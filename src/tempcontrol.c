#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <softPwm.h>

#define PIN_RESISTANCE 4
#define PIN_FAN 5

void init_GPIO() {
    if (wiringPiSetup() == -1) {
        exit(1);
    }
    
    pinMode(PIN_RESISTANCE, OUTPUT);
    pinMode(PIN_FAN, OUTPUT);
    softPwmCreate(PIN_RESISTANCE, 1, 100);
    softPwmCreate(PIN_FAN, 1, 100);
}

void activate_devices(int control) {
    if (control > 0) {
        softPwmWrite(PIN_FAN, 0);
        softPwmWrite(PIN_RESISTANCE, control);
    }
    else if (control <= -40) {
        softPwmWrite(PIN_RESISTANCE, 0);
        softPwmWrite(PIN_FAN, abs(control));
    }
    else {
        softPwmWrite(PIN_FAN, 0);
        softPwmWrite(PIN_RESISTANCE, 0);
    }
}