/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h> /* �????��?��????��?? */
#include <time.h>   /* ????????��????��?? */

//extern uint16_t last_rand;
uint16_t last_rand = 0xACE1U;

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#include "include/ALL.h"
#include "include/uart_bridge.h"

void Clear_CMD_Assert()
{
    UART_Sendor_CMD.UART_RX_flag = Disable;
    memset(UART_Sendor_CMD.UART_RxBuffer, 0, sizeof(UART_Sendor_CMD.UART_RxBuffer));
}

void initialize_params(void)
{
    IsWakeup = 1;
    IsEnableACK = No;
    FaultInjection =1;

    hundredth_min = 0;
    hundredth_max = 100;
    thousandth_min = 0;
    thousandth_max = 1000;
    threshold_voltage = 80;
    threshold_brightness = 99;    
    min_n_current = 0;
    max_n_current = 20;
}

void led_blink()
{
    gpio_put(led_pin, true);
    sleep_ms(500);
    gpio_put(led_pin, false);
    sleep_ms(500);
}



int main()
{

    initialize_params();
   
    // Initialize LED pin
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);

    // Initialize chosen serial port
    stdio_init_all();

    // Set up our UART with a basic baud rate.
    uart_init(UART_ID, 2400);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, UART_RX_INTB);

    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    // OK, all set up.
    // Lets send a basic string out, and then run a loop and wait for RX interrupts
    // The handler will count them, but also reflect the incoming data back with a slight change!
    uart_puts(UART_ID, "\nHelo world!\r\n");
    UART_Sendor_CMD.UART_RX_flag == false;
    // uart_write_blocking(UART_ID, Host_ver, 3);
    len_chars = 0;

    memset(UART_Sendor_CMD.UART_RxBuffer, 0, sizeof(UART_Sendor_CMD.UART_RxBuffer));
    UART_Sendor_CMD.UART_ReceiveLength = 0;



    while (1)
    {
        
        // generating a random number based on real time data calculations.
        last_rand = rand(); 
        // modified the value based on the random number 
        //modified_status();
        // reponse flow to TX
        Execute_Simulator_CMD();
        sleep_ms(1);
    }
}
