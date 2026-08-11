#ifndef PROTOCOL_STREAM_PARSER_H
#define PROTOCOL_STREAM_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROTOCOL_FRAME_SIZE 5u
#define PROTOCOL_HEADER_FA 0xFAu
#define PROTOCOL_HEADER_DA 0xDAu

typedef struct {
    uint8_t frame[PROTOCOL_FRAME_SIZE];
    uint8_t length;
    uint32_t last_byte_time_ms;
    bool has_timestamp;
} protocol_stream_parser_t;

typedef void (*protocol_frame_handler_t)(
    const uint8_t frame[PROTOCOL_FRAME_SIZE],
    void *context);

void protocol_stream_parser_init(protocol_stream_parser_t *parser);

void protocol_stream_parser_feed(
    protocol_stream_parser_t *parser,
    const uint8_t *data,
    size_t length,
    uint32_t now_ms,
    protocol_frame_handler_t frame_handler,
    void *context);

bool protocol_stream_parser_expire(
    protocol_stream_parser_t *parser,
    uint32_t now_ms,
    uint32_t timeout_ms);

size_t protocol_stream_parser_pending_length(
    const protocol_stream_parser_t *parser);

#ifdef __cplusplus
}
#endif

#endif

