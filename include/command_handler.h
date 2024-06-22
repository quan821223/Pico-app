#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H


#include "commands.h"
#include <stdint.h>
#include "tusb.h"




typedef enum {
    Header_TYPE_FA = 0xFA,
    Header_TYPE_DA = 0xDA,
} HeaderType;

typedef struct {
    uint8_t Head_type;
    uint8_t Buffer_Len;
    char * UHC_Buffer;
} UHC_CMD;

void handle_command(const UHC_CMD *cmd);

void handle_type_fa(const UHC_CMD *cmd, response_t *resp);
void handle_type_da(const UHC_CMD *cmd, response_t *resp);
void response_usb_send(const void *data, size_t len);
#endif // COMMAND_HANDLER_H