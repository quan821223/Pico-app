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

#ifndef TUD_CDC_DESCRIPT_H
#define TUD_CDC_DESCRIPT_H

#include <stddef.h>
#include "pico/stdlib.h"
#include "ALL.h"

#ifdef __cplusplus
extern "C" {
#endif


extern UHC_type_CMD tud_cdc_buffer;


extern bool tud_cdc_ReturnRequest();

// extern void usb_send(const void *data, size_t len); 

extern void receive_data(uint8_t * data, uint8_t length);

extern void process_message(uint8_t * message, uint8_t length);

extern void reset_usb_parser_state();

#ifdef __cplusplus
}
#endif

#endif /* TUD_CDC_DESCRIPT_H */