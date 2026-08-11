#include "protocol_stream_parser.h"

#include <string.h>

static bool is_header(uint8_t byte)
{
    return byte == PROTOCOL_HEADER_FA || byte == PROTOCOL_HEADER_DA;
}

void protocol_stream_parser_init(protocol_stream_parser_t *parser)
{
    if (parser == NULL) {
        return;
    }

    memset(parser, 0, sizeof(*parser));
}

void protocol_stream_parser_feed(
    protocol_stream_parser_t *parser,
    const uint8_t *data,
    size_t length,
    uint32_t now_ms,
    protocol_frame_handler_t frame_handler,
    void *context)
{
    if (parser == NULL || (data == NULL && length != 0u)) {
        return;
    }

    for (size_t index = 0; index < length; ++index) {
        const uint8_t byte = data[index];

        if (is_header(byte)) {
            parser->length = 0u;
            parser->frame[parser->length++] = byte;
            parser->last_byte_time_ms = now_ms;
            parser->has_timestamp = true;
            continue;
        }

        if (parser->length == 0u) {
            continue;
        }

        parser->frame[parser->length++] = byte;
        parser->last_byte_time_ms = now_ms;
        parser->has_timestamp = true;

        if (parser->length == PROTOCOL_FRAME_SIZE) {
            if (frame_handler != NULL) {
                frame_handler(parser->frame, context);
            }
            parser->length = 0u;
            parser->has_timestamp = false;
        }
    }
}

bool protocol_stream_parser_expire(
    protocol_stream_parser_t *parser,
    uint32_t now_ms,
    uint32_t timeout_ms)
{
    if (parser == NULL || parser->length == 0u || !parser->has_timestamp) {
        return false;
    }

    if ((uint32_t)(now_ms - parser->last_byte_time_ms) < timeout_ms) {
        return false;
    }

    parser->length = 0u;
    parser->has_timestamp = false;
    return true;
}

size_t protocol_stream_parser_pending_length(
    const protocol_stream_parser_t *parser)
{
    return parser == NULL ? 0u : parser->length;
}

