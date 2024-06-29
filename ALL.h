/**
 * @file ALL.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-02-14
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef ALL_H
#define ALL_H


#include "pico/stdlib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gpio.h"

#ifdef PICO_BOARD
#define LED_PIN 25
#endif

#ifdef PICO_W_BOARD
#define LED_PIN 0
#endif

#ifdef PICO_ZERO_BOARD
#define LED_PIN 15
#endif

// 流程狀態
typedef enum {
    STATE_IDLE,
    STATE_READING,
    STATE_PROCESSING,
} State;



// 指定裝置
typedef enum {
    DEVICE_TYPE_ALL = 0x00,     // all devices
    DEVICE_TYPE_1 = 0x01,       // device No.1
    DEVICE_TYPE_2 = 0x02,       // device No.2
    DEVICE_TYPE_3 = 0x03,       // device No.3       
} DeviceType;

// MSG 種類
typedef enum {
    MSGaddress_TYPE_0 = 0x00,   // Information
    MSGaddress_TYPE_1 = 0x01,   // Brightness
    MSGaddress_TYPE_2 = 0x02,   // Power Mode
    MSGaddress_TYPE_3 = 0x03,   // Pattern
    MSGaddress_TYPE_4 = 0x04,   // Diagnostic
    MSGaddress_TYPE_5 = 0x05,   // Current
    MSGaddress_TYPE_6 = 0x06,   // Touch by electrical finger
    MSGaddress_TYPE_7 = 0x07,   // Touch by copper or finger

} MSGaddress;

typedef struct
{
    uint8_t Buffer_Len;
    char * UHC_Buffer;
} UHC_type_CMD;




void gpio_callback(uint gpio, uint32_t events) ;


#endif