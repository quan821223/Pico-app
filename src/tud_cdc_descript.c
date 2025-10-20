/**
 * @file tud_cdc_descript.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-02-14
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "ALL.h"
#include "tud_cdc_descript.h"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RX_BUFFER_SIZE 64
#define SuperDeviceType 0x2
#define Latencytime 50



char rx_buffer[RX_BUFFER_SIZE];
uint16_t last_rand = 0xACE1U;
UHC_type_CMD tud_cdc_buffer;
State current_state = STATE_IDLE;
uint8_t rx_buffer_index = 0;
bool tud_cdc_SendRequest;

bool tud_cdc_ReturnRequest()
{
    return tud_cdc_SendRequest? true: false;
}
void reset_usb_parser_state()
{
    tud_cdc_SendRequest = false;
    current_state = STATE_IDLE;
}

void receive_data(uint8_t *data, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        uint8_t byte = data[i];

        switch (current_state) {
            case STATE_IDLE:
                if (byte == 0xFA || byte == 0xDA) {
                    rx_buffer_index = 0;
                    rx_buffer[rx_buffer_index++] = byte;
                    current_state = STATE_READING;
                }
                break;

            case STATE_READING:
                if (byte == 0xFA || byte == 0xDA) {
                    // 新封包起始符號，重置緩衝
                    rx_buffer_index = 0;
                    rx_buffer[rx_buffer_index++] = byte;
                    current_state = STATE_READING;
                } else {
                    if (rx_buffer_index < RX_BUFFER_SIZE) {
                        rx_buffer[rx_buffer_index++] = byte;
                    }

                    if (rx_buffer_index == 5) { // 若固定長度為 5
                        process_message(rx_buffer, 5);
                        current_state = STATE_IDLE;
                        rx_buffer_index = 0;
                    }
                }
                break;

            default:
                current_state = STATE_IDLE;
                rx_buffer_index = 0;
                break;
        }
    }
}

void ResponseCMD(uint8_t *data,uint8_t len)
{
    // usb_send(data, len);
    DelayResponse.trigger_time = make_timeout_time_ms(Latencytime);
    memcpy(DelayResponse.buffer, data, len);
    DelayResponse.length = len;
    DelayResponse.valid = true;
}

int limited_rand( int min, int max) {
    last_rand = (18000U * (last_rand & 0xFFFFU)) + (last_rand >> 16);
    return (last_rand % (max - min + 1)) + min;
}

int prob_based_rand(int current_value, int change_value, int probability) {
    int rand_val = limited_rand(1, 100); // 1???100?????��?????
    if(rand_val <= probability) {
        return change_value;
    } else {
        return current_value;
    }
}
uint8_t IdentifyDeviceType[4] = {0xFA, 0x00, 0x0D, 0x0A};
uint8_t SCContentFunc[11] = {0xFA, 0x00, 0x08, 0x01, 0x02, 0x01, 0x02, 0x00, 0x02, 0x0D, 0x0A};
uint8_t version_buf[8] = {0xFA, 0x01, 0x05, 0x00, 0x00, 0x00, 0x0D, 0x0A};
uint8_t current_buf[7] = {0xFA, 0x01, 0x04, 0x01, 0x23, 0x0D, 0x0A};
uint8_t brightnessbacklight_buf[6] = {0xFA, 0x01, 0x03, 0x64, 0x0D, 0x0A};
uint8_t diag_buf[9] = {0xFA, 0x01, 0x06, 0x00, 0x02, 0x03, 0x04, 0x0D, 0x0A};
uint8_t Voltage_buf[7] = {0xFA, 0x01, 0x04, 0x00, 0x8A, 0x0D, 0x0A};
uint8_t FIDMtemp[8] = {0xFA, 0x01, 0x05, 0x4A, 0x4B, 0x4C, 0x0D, 0x0A};
uint8_t ChamberTemp[6] = {0xFA, 0x01, 0x03, 0x4B, 0x0D, 0x0A};
uint8_t ADC[7] = {0xFA, 0x01, 0x04, 0x01, 0x12, 0x0D, 0x0A};
uint8_t Touchx06[66] = {0xFA, 0x01, 0x3F, 0x0A, 0x00, 0x11, 0x01, 0x12, 0x00, 0xF2,
                                                0x00, 0x21, 0x02, 0x12, 0x01, 0x11,
                                                0x00, 0x31, 0x03, 0x12, 0x01, 0xA0,
                                                0x00, 0x41, 0x04, 0x12, 0x02, 0x00,
                                                0x00, 0x51, 0x05, 0x12, 0x02, 0x14,
                                                0x00, 0x61, 0x06, 0x12, 0x02, 0x58,
                                                0x00, 0x71, 0x03, 0xF4, 0x02, 0x65,
                                                0x00, 0x81, 0x08, 0x00, 0x02, 0x74,
                                                0x00, 0x91, 0x02, 0xF4, 0x02, 0x84,
                                                0x00, 0xA1, 0x01, 0xF4, 0x03, 0x84, 0x0D, 0x0A};
uint8_t AmbientTemp [6] = {0xFA, 0x02, 0x03, 0x41, 0x0D, 0x0A}; 
uint8_t AmbientHumidity [6] = {0xFA, 0x02, 0x03, 0x55, 0x0D, 0x0A}; 
uint8_t Write_ACK[3] = {0xC3, 0x0D, 0x0A}; 
uint8_t PARTnum[13] = {0xFA, 0x01, 0x0A, 0x46, 0x49, 0x44, 0x4D, 0x53, 0x57, 0x23, 0x30, 0x0D, 0x0A}; 
uint8_t FIDMSW[13] = {0xFA, 0x01, 0x0A, 0x46, 0x49, 0x44, 0x4D, 0x53, 0x57, 0x23, 0x31, 0x0D, 0x0A}; 
uint8_t FIDMHW[13] = {0xFA, 0x01, 0x0A, 0x46, 0x49, 0x44, 0x4D, 0x48, 0x57, 0x23, 0x31, 0x0D, 0x0A}; 
uint8_t TCONVER[13] = {0xFA, 0x01, 0x0A, 0x54, 0x43, 0x4F, 0x4E, 0x58, 0x58, 0x23, 0x31, 0x0D, 0x0A}; 
uint8_t TOUCHVER[13] = {0xFA, 0x01, 0x0A, 0x54, 0x4F, 0x55, 0x43, 0x48, 0x58, 0x23, 0x31, 0x0D, 0x0A}; 
uint8_t LCMVER[13] = {0xFA, 0x01, 0x0A, 0x4C, 0x43, 0x4D, 0x58, 0x58, 0x58, 0x23, 0x31, 0x0D, 0x0A}; 
uint8_t FIDMBL[13] = {0xFA, 0x01, 0x0A, 0x46, 0x49, 0x44, 0x4D, 0x42, 0x4C, 0x23, 0x31, 0x0D, 0x0A};
uint8_t DAdeviceStatus[11] = {0xDA, 0x00, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0D, 0x0A};
uint8_t DAx02deviceStatus[8] = {0xDA, 0x02, 0x05, 0x01, 0x01, 0x01, 0x0D, 0x0A};
uint8_t DAx0CCurrent[9] = {0xDA, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x0D, 0x0A};
uint8_t DAx0D01Voltage[7] = {0xDA, 0x05, 0x04, 0x01, 0x01, 0x0D, 0x0A};
uint8_t DAx0D02Voltage[7] = {0xDA, 0x07, 0x04, 0x01, 0x01, 0x0D, 0x0A};
// void gpio_callback(uint gpio, uint32_t events) {
//     // 根據觸發中斷的 GPIO Pin 處理相應的操作
//     if (gpio == DATOUCH_PIN26) {
//         // 處理Pin 26的操作
//         printf("Pin 26 pressed!\n");
//         gpio_put(DATOUCH_PIN27, 1); // 設置 Pin 27 為高電平
//         sleep_ms(PUSH_TIME_STEPS * 100);  // 模擬觸控信號保持 100ms
//         gpio_put(DATOUCH_PIN27, 0); // 設置 Pin 27 為低電平
//     }
//     if (gpio == DATOUCH_PIN27) {
//         // 處理Pin 27的操作
//         printf("Pin 27 pressed!\n");
//         gpio_put(DATOUCH_PIN26, 1); // 設置 Pin 26 為高電平
//         sleep_ms(PUSH_TIME_STEPS * 100);  // 模擬觸控信號保持 100ms
//         gpio_put(DATOUCH_PIN26, 0); // 設置 Pin 26 為低電平
//     }
// }
touch_data_t touch26_data = { DATOUCH_PIN26, 0, {0} };
touch_data_t touch27_data = { DATOUCH_PIN27, 0, {0} };


bool touch_callback(struct repeating_timer *t) {
    touch_data_t *data = (touch_data_t *)t->user_data;
    gpio_put(data->gpio_pin, 0); // 设置 Pin 为低电平
    cancel_repeating_timer(t); // 取消定时器
    return false; // 不再重复调用
}

void chamber_set_state(chamber_state_t state) {
    // 注意：我們 switch 的是「邏輯狀態」，而不是 uint8_t level
    switch (state) {
        case CHAMBER_STATE_OFF: // 0x00
            gpio_put(GPIO_PIN2, 0);
            gpio_put(GPIO_PIN3, 0);
            break;
        case CHAMBER_STATE_A: // 0x01
            gpio_put(GPIO_PIN2, 1);
            gpio_put(GPIO_PIN3, 0);
            break;
        case CHAMBER_STATE_B: // 0x02
            gpio_put(GPIO_PIN2, 0);
            gpio_put(GPIO_PIN3, 1);
            break;
        case CHAMBER_STATE_ON: // 0x03
        default:
            gpio_put(GPIO_PIN2, 1);
            gpio_put(GPIO_PIN3, 1);
            break;
    }
}

void simulate_touch(uint gpio_pin, uint8_t steps) {
    gpio_put(gpio_pin, 1); // 设置 Pin 为高电平
    touch_data_t *data = gpio_pin == DATOUCH_PIN26 ? &touch26_data : &touch27_data;
    data->steps = steps;
    data->gpio_pin = gpio_pin;
    add_repeating_timer_ms(steps * 100, touch_callback, data, &data->timer);
}



void process_message(uint8_t *message, uint8_t length) {
    uint8_t temp_len = 0;

    MessageType Header = message[0];    
    MessageType msg_type = message[1];  
    DeviceType device_type = message[2];
    MSGaddress category = message[3];
    uint8_t MSGparam = message[4];
    
    uint8_t * ptr;
    uint8_t * return_buf = NULL; // ← 加這行，安全起見
    if (msg_type == MSG_TYPE_READ) {
        if(Header ==Header_TYPE_FA)
        {
            switch (category)
            {
                case MSGaddress_TYPE_1A:
                    return_buf = (uint8_t*)malloc(4); 
                    memcpy(return_buf, IdentifyDeviceType, sizeof(IdentifyDeviceType));
                    return_buf[1] = SuperDeviceType;                  
                    ResponseCMD(return_buf, 4);               
       
                    break;
                case MSGaddress_TYPE_1B:
                    return_buf = (uint8_t*)malloc(11); 
                    memcpy(return_buf, SCContentFunc, sizeof(SCContentFunc));
                    return_buf[1] = 0x00;                  
                    ResponseCMD(return_buf, 11);  

                    break;
                case MSGaddress_TYPE_0:             
                    return_buf = (uint8_t*)malloc(13);                     
                    if (MSGparam == 0x00) { memcpy(return_buf, PARTnum, sizeof(PARTnum));}     
                    else if (MSGparam == 0x01) { memcpy(return_buf, FIDMHW, sizeof(FIDMHW));}               
                    else if (MSGparam == 0x02) { memcpy(return_buf, FIDMSW, sizeof(FIDMSW)); }
                    else if (MSGparam == 0x03) { memcpy(return_buf, FIDMBL, sizeof(FIDMBL)); }
                    else if (MSGparam == 0x04) { memcpy(return_buf, TCONVER, sizeof(TCONVER)); }   
                    else if (MSGparam == 0x05) { memcpy(return_buf, TOUCHVER, sizeof(TOUCHVER)); }   
                    else if (MSGparam == 0x06) { memcpy(return_buf, LCMVER, sizeof(LCMVER)); }   
                    return_buf[1] = device_type;
                    return_buf[10] = 0x30 + MSGparam;
                    ResponseCMD(return_buf, 13);    

                    break;
                case MSGaddress_TYPE_1:
                    return_buf = (uint8_t*)malloc(6); 
                    memcpy(return_buf, brightnessbacklight_buf, sizeof(brightnessbacklight_buf));         
                    return_buf[1] = device_type;
                    ResponseCMD(return_buf, 6 );    
                    break;
                case MSGaddress_TYPE_2:
                    break;
                case MSGaddress_TYPE_3:
                    break;   
                case MSGaddress_TYPE_4:
                    if (MSGparam == 0x01) {    
                        return_buf = (uint8_t*)malloc(9);                    
                        memcpy(return_buf, diag_buf, sizeof(diag_buf)); 
                    
                        return_buf[1] = device_type;
                        return_buf[3] = 0x00; // 0x01 : pass ; 0x00 : fail
                        return_buf[4] = 0x02; 
                        return_buf[5] = 0x03; 
                        return_buf[6] = 0x04; 
                        ResponseCMD(return_buf, 9); 
                    }               
                    else if (MSGparam == 0x02) {
                        return_buf = (uint8_t*)malloc(7); 
                        memcpy(return_buf, Voltage_buf, sizeof(Voltage_buf)); 
                        return_buf[1] = device_type;
                        return_buf[3] = 0x00;
                        return_buf[4] = 0x8A;                    
                        ResponseCMD(return_buf, 7); 
                    }
                    else if (MSGparam == 0x03) {
                        return_buf = (uint8_t*)malloc(8); 
                        memcpy(return_buf, FIDMtemp, sizeof(FIDMtemp));   
                        return_buf[1] = device_type;
                        ResponseCMD(return_buf, 8); 
                    }
                    else if (MSGparam == 0x04) { 
                        return_buf = (uint8_t*)malloc(6); 
                        memcpy(return_buf, ChamberTemp, sizeof(ChamberTemp));  
                        return_buf[1] = device_type; 
                        ResponseCMD(return_buf, 6); 
                    }   
                    else if (MSGparam == 0x05) { 
                        return_buf = (uint8_t*)malloc(7); 
                        memcpy(return_buf, ADC, sizeof(ADC)); 
                        return_buf[1] = device_type;  
                        ResponseCMD(return_buf, 7); 
                    }   
                    break;  
                case MSGaddress_TYPE_5:
                    return_buf = (uint8_t*)malloc(7); 
                    memcpy(return_buf, current_buf, sizeof(current_buf)); 
                    return_buf[1] = device_type;  
                    ResponseCMD(return_buf, 7); 
                    break;  
                case MSGaddress_TYPE_6:
                    return_buf = (uint8_t*)malloc(66); 
                    memcpy(return_buf, Touchx06, sizeof(Touchx06)); 
                    return_buf[1] = device_type;  
                    ResponseCMD(return_buf, 66); 
                    break;
                case 0x0A:
                    break;  
                default:
                    break;
            }
        // ...
        }
        else if (Header == Header_TYPE_DA)
        {
            switch (device_type)
            {
                case MSGaddress_TYPE_2:
                    return_buf = (uint8_t*)malloc(8); 
                    memcpy(return_buf, DAx02deviceStatus, sizeof(DAx02deviceStatus));  
                    return_buf[1] = device_type; 
                    ResponseCMD(return_buf, 8); 
                    break;
                case MSGaddress_TYPE_3:
                    return_buf = (uint8_t*)malloc(11); 
                    memcpy(return_buf, DAdeviceStatus, sizeof(DAdeviceStatus));  
                    return_buf[1] = device_type; 
                    ResponseCMD(return_buf, 11); 
                    break;
                case MSGaddress_TYPE_4:
                    return_buf = (uint8_t*)malloc(11); 
                    memcpy(return_buf, DAdeviceStatus, sizeof(DAdeviceStatus));  
                    return_buf[1] = device_type; 
                    ResponseCMD(return_buf, 11); 
                    break;
                case MSGaddress_TYPE_5:
                    return_buf = (uint8_t*)malloc(11); 
                    memcpy(return_buf, DAdeviceStatus, sizeof(DAdeviceStatus));  
                    return_buf[1] = device_type; 
                    ResponseCMD(return_buf, 11); 
                    break;
                case MSGaddress_TYPE_6:
                    return_buf = (uint8_t*)malloc(11); 
                    memcpy(return_buf, DAdeviceStatus, sizeof(DAdeviceStatus));  
                    return_buf[1] = device_type; 
                    ResponseCMD(return_buf, 11); 
                    break;
                case MSGaddress_TYPE_7:
                    return_buf = (uint8_t*)malloc(11); 
                    memcpy(return_buf, DAdeviceStatus, sizeof(DAdeviceStatus));  
                    return_buf[1] = device_type; 
                    ResponseCMD(return_buf, 11); 
                    break;
                case MSGaddress_TYPE_9:
                    return_buf = (uint8_t*)malloc(11); 
                    memcpy(return_buf, DAdeviceStatus, sizeof(DAdeviceStatus));  
                    return_buf[1] = device_type; 
                    ResponseCMD(return_buf, 11); 
                    break;
                case MSGaddress_TYPE_10:
                    return_buf = (uint8_t*)malloc(11); 
                    memcpy(return_buf, DAdeviceStatus, sizeof(DAdeviceStatus));  
                    return_buf[1] = device_type; 
                    ResponseCMD(return_buf, 11); 
                    break;
                case MSGaddress_TYPE_0C:
                    return_buf = (uint8_t*)malloc(9); 
                    memcpy(return_buf, DAx0CCurrent, sizeof(DAx0CCurrent));  
                    return_buf[3] = MSGparam; 
                    return_buf[4] = MSGparam; 
                    return_buf[5] = MSGparam; 
                    return_buf[6] = MSGparam; 
                    ResponseCMD(return_buf, 9); 
                    break;
                case MSGaddress_TYPE_0D:
                    if(category == 0x01)
                    {
                        return_buf = (uint8_t*)malloc(7); 
                        memcpy(return_buf, DAx0D01Voltage, sizeof(DAx0D01Voltage));  
                        return_buf[3] = MSGparam; 
                        return_buf[4] = MSGparam; 
                        ResponseCMD(return_buf, 7); 

                    }
                    else
                    {
                        return_buf = (uint8_t*)malloc(7); 
                        memcpy(return_buf, DAx0D02Voltage, sizeof(DAx0D02Voltage));  
                        return_buf[3] = MSGparam+1; 
                        return_buf[4] = MSGparam+1; 
                        ResponseCMD(return_buf, 7); 

                    }

                    break;
                default:
                    return_buf = (uint8_t*)malloc(3); 
                    memcpy(return_buf, Write_ACK, sizeof(Write_ACK));                
                    ResponseCMD(return_buf, 3);    
                     
            }

        }
      
    } else if (msg_type == MSG_TYPE_WRITE) {   

        if(Header == Header_TYPE_DA)
        {
            switch (device_type)
            {
                case MSGaddress_TYPE_3:
                    if(category == 01)
                    {
                        simulate_touch(DATOUCH_PIN26, message[4]);
                    }
                    else if(category == 02)
                    {
                        simulate_touch(DATOUCH_PIN27, message[4]);
                    }
                    else if(category == 10)
                    {
                        chamber_set_state( message[4]);
                    }
                    return_buf = (uint8_t*)malloc(3); 
                    memcpy(return_buf, Write_ACK, sizeof(Write_ACK));                
                    ResponseCMD(return_buf, 3); 
                    break; 
            }
        }

   
      
    }
    else
    {
   
    }
    free(return_buf); // ← 加這行釋放記憶體
}
