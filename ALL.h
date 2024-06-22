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


#define DATOUCH_PIN26 20
#define DATOUCH_PIN27 21

typedef struct {
    uint gpio_pin;
    uint8_t steps;
    repeating_timer_t timer;
} touch_data_t;



// 流程狀態
typedef enum {
    STATE_IDLE,
    STATE_READING,
    STATE_PROCESSING,
} State;

typedef enum {
    Header_TYPE_FA = 0xFA,
    Header_TYPE_DA = 0xDA,
} HeaderType;

// 指令屬性
typedef enum {
    MSG_TYPE_READ = 0x52,
    MSG_TYPE_WRITE = 0x57,
} MessageType;

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

typedef enum {
    CMD_TYPE_READ = 0x01,
    CMD_TYPE_WRITE = 0x02,
    CMD_TYPE_RESET = 0x03, // 新指令
    // 添加其他命令類型
} command_type_t;

void gpio_callback(uint gpio, uint32_t events) ;


#endif