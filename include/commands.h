// commands.h
#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>


extern const uint8_t Write_ACK[];

// 指令屬性
typedef enum {
    MSG_TYPE_READ = 0x52,
    MSG_TYPE_WRITE = 0x57,
} MessageType;

typedef enum {
    CMD_TYPE_READ = 0x01,
    CMD_TYPE_WRITE = 0x02,
    CMD_TYPE_RESET = 0x03, 
} command_type_t;

typedef struct {
    command_type_t type;
    uint8_t param;
    uint8_t data[8];
} command_t;

typedef struct {
    uint8_t status;
    uint8_t data[1024];
} response_t;

#endif // COMMANDS_H