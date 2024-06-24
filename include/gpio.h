#ifndef _GPIO_H
#define _GPIO_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define DATOUCH_PIN26 20
#define DATOUCH_PIN27 21

typedef struct {
    uint gpio_pin;
    uint8_t steps;
    repeating_timer_t timer;
} touch_data_t;

void init_gpio(void);




#endif