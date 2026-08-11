#include "protocol_service.h"

#include <string.h>

static void handle_frame(
    const uint8_t frame[PROTOCOL_FRAME_SIZE],
    void *context)
{
    protocol_service_t *service = context;
    app_protocol_inputs_t inputs = {0};
    app_protocol_result_t result;

    if (service->ports.read_chamber_status != NULL) {
        inputs.chamber_status = service->ports.read_chamber_status(
            service->ports.context);
    }

    app_protocol_handle(frame, &inputs, &result);

    if (result.effect.type != APP_EFFECT_NONE &&
        service->ports.apply_effect != NULL) {
        service->ports.apply_effect(&result.effect, service->ports.context);
    }

    if (result.has_response && service->ports.response_ready != NULL) {
        service->ports.response_ready(
            result.response.data,
            result.response.length,
            APP_PROTOCOL_DEFAULT_RESPONSE_DELAY_MS,
            service->ports.context);
    }
}

void protocol_service_init(
    protocol_service_t *service,
    const protocol_service_ports_t *ports)
{
    if (service == NULL) {
        return;
    }

    memset(service, 0, sizeof(*service));
    protocol_stream_parser_init(&service->parser);
    if (ports != NULL) {
        service->ports = *ports;
    }
}

void protocol_service_feed(
    protocol_service_t *service,
    const uint8_t *data,
    size_t length,
    uint32_t now_ms)
{
    if (service == NULL) {
        return;
    }

    protocol_stream_parser_feed(
        &service->parser,
        data,
        length,
        now_ms,
        handle_frame,
        service);
}

bool protocol_service_expire(
    protocol_service_t *service,
    uint32_t now_ms,
    uint32_t timeout_ms)
{
    return service != NULL && protocol_stream_parser_expire(
        &service->parser, now_ms, timeout_ms);
}

