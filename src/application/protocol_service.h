#ifndef PROTOCOL_SERVICE_H
#define PROTOCOL_SERVICE_H

#include "app_protocol.h"
#include "protocol_stream_parser.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t (*protocol_read_chamber_status_t)(void *context);
typedef void (*protocol_apply_effect_t)(
    const app_effect_t *effect,
    void *context);
typedef void (*protocol_response_ready_t)(
    const uint8_t *data,
    size_t length,
    uint32_t delay_ms,
    void *context);

typedef struct {
    protocol_read_chamber_status_t read_chamber_status;
    protocol_apply_effect_t apply_effect;
    protocol_response_ready_t response_ready;
    void *context;
} protocol_service_ports_t;

typedef struct {
    protocol_stream_parser_t parser;
    protocol_service_ports_t ports;
} protocol_service_t;

void protocol_service_init(
    protocol_service_t *service,
    const protocol_service_ports_t *ports);

void protocol_service_feed(
    protocol_service_t *service,
    const uint8_t *data,
    size_t length,
    uint32_t now_ms);

bool protocol_service_expire(
    protocol_service_t *service,
    uint32_t now_ms,
    uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif

