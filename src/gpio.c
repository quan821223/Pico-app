#include "gpio.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


touch_data_t touch26_data = { DATOUCH_PIN26, 0, {0} };
touch_data_t touch27_data = { DATOUCH_PIN27, 0, {0} };


void init_gpio(void){
    gpio_init(DATOUCH_PIN26);
    gpio_set_dir(DATOUCH_PIN26, GPIO_OUT);
    gpio_put(DATOUCH_PIN26, 0);
    gpio_init(DATOUCH_PIN27);
    gpio_set_dir(DATOUCH_PIN27, GPIO_OUT);
    gpio_put(DATOUCH_PIN27, 0);

}




bool touch_callback(struct repeating_timer *t) {
    touch_data_t *data = (touch_data_t *)t->user_data;
    gpio_put(data->gpio_pin, 0); // 设置 Pin 为低电平
    cancel_repeating_timer(t); // 取消定时器
    return false; // 不再重复调用
}


void simulate_touch(uint gpio_pin, uint8_t steps) {
    gpio_put(gpio_pin, 1); // 设置 Pin 为高电平
    touch_data_t *data = gpio_pin == DATOUCH_PIN26 ? &touch26_data : &touch27_data;
    data->steps = steps;
    data->gpio_pin = gpio_pin;
    add_repeating_timer_ms(steps * 100, touch_callback, data, &data->timer);
}
