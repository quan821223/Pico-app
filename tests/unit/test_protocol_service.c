#include "protocol_service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t chamber_status;
    app_effect_t effect;
    uint8_t response[APP_PROTOCOL_MAX_RESPONSE_SIZE];
    size_t response_length;
    uint32_t delay_ms;
    size_t effect_count;
    size_t response_count;
    size_t control_count;
    uint16_t configured_delay_ms;
} fixture_t;

static void fail(const char *expression, int line)
{
    fprintf(stderr, "assertion failed at line %d: %s\n", line, expression);
    exit(EXIT_FAILURE);
}

#define CHECK(expression) \
    do { \
        if (!(expression)) { \
            fail(#expression, __LINE__); \
        } \
    } while (0)

static uint8_t read_chamber(void *context)
{
    return ((fixture_t *)context)->chamber_status;
}

static void apply_effect(const app_effect_t *effect, void *context)
{
    fixture_t *fixture = context;

    fixture->effect = *effect;
    ++fixture->effect_count;
}

static void response_ready(
    const uint8_t *data,
    size_t length,
    uint32_t delay_ms,
    void *context)
{
    fixture_t *fixture = context;

    CHECK(length <= sizeof(fixture->response));
    memcpy(fixture->response, data, length);
    fixture->response_length = length;
    fixture->delay_ms = delay_ms;
    ++fixture->response_count;
}

static uint16_t read_response_delay(void *context)
{
    return ((fixture_t *)context)->configured_delay_ms;
}

static void handle_control(
    const uint8_t request[APP_PROTOCOL_REQUEST_SIZE],
    app_response_t *response,
    void *context)
{
    fixture_t *fixture = context;
    const uint8_t control_response[] = {
        request[0], 0x00u, request[2], request[3], request[4], 0x0Du, 0x0Au,
    };

    memcpy(response->data, control_response, sizeof(control_response));
    response->length = sizeof(control_response);
    ++fixture->control_count;
}

static protocol_service_t create_service(fixture_t *fixture)
{
    const protocol_service_ports_t ports = {
        .read_chamber_status = read_chamber,
        .apply_effect = apply_effect,
        .response_ready = response_ready,
        .read_response_delay_ms = read_response_delay,
        .handle_control = handle_control,
        .context = fixture,
    };
    protocol_service_t service;

    protocol_service_init(&service, &ports);
    return service;
}

static void test_service_reads_input_and_delivers_delayed_response(void)
{
    fixture_t fixture = {
        .chamber_status = 2u,
        .configured_delay_ms = APP_PROTOCOL_DEFAULT_RESPONSE_DELAY_MS,
    };
    protocol_service_t service = create_service(&fixture);
    const uint8_t request[] = {0xDA, 0x52, 0x20, 0x00, 0x00};
    const uint8_t expected[] = {0xDA, 0x20, 0x03, 0x02, 0x0D, 0x0A};

    protocol_service_feed(&service, request, sizeof(request), 100u);

    CHECK(fixture.response_count == 1u);
    CHECK(fixture.response_length == sizeof(expected));
    CHECK(memcmp(fixture.response, expected, sizeof(expected)) == 0);
    CHECK(fixture.delay_ms == APP_PROTOCOL_DEFAULT_RESPONSE_DELAY_MS);
    CHECK(fixture.effect_count == 0u);
}

static void test_service_uses_runtime_delay_for_application_response(void)
{
    fixture_t fixture = {.configured_delay_ms = 125u};
    protocol_service_t service = create_service(&fixture);
    const uint8_t request[] = {0xFA, 0x52, 0x01, 0x1A, 0x00};

    protocol_service_feed(&service, request, sizeof(request), 150u);

    CHECK(fixture.response_count == 1u);
    CHECK(fixture.delay_ms == 125u);
}

static void test_service_routes_control_without_application_delay(void)
{
    fixture_t fixture = {.configured_delay_ms = 125u};
    protocol_service_t service = create_service(&fixture);
    const uint8_t request[] = {0xCF, 0x52, 0x02, 0x00, 0x00};
    const uint8_t expected[] = {0xCF, 0x00, 0x02, 0x00, 0x00, 0x0D, 0x0A};

    protocol_service_feed(&service, request, sizeof(request), 175u);

    CHECK(fixture.control_count == 1u);
    CHECK(fixture.response_count == 1u);
    CHECK(fixture.delay_ms == 0u);
    CHECK(fixture.response_length == sizeof(expected));
    CHECK(memcmp(fixture.response, expected, sizeof(expected)) == 0);
}

static void test_service_applies_effect_before_delivering_response(void)
{
    fixture_t fixture = {0};
    protocol_service_t service = create_service(&fixture);
    const uint8_t request[] = {0xDA, 0x57, 0x03, 0x02, 0x03};

    protocol_service_feed(&service, request, sizeof(request), 200u);

    CHECK(fixture.effect_count == 1u);
    CHECK(fixture.effect.type == APP_EFFECT_TOUCH_1);
    CHECK(fixture.effect.value == 3u);
    CHECK(fixture.response_count == 1u);
}

static void test_service_keeps_partial_frame_until_completed(void)
{
    fixture_t fixture = {0};
    protocol_service_t service = create_service(&fixture);
    const uint8_t first[] = {0xFA, 0x52};
    const uint8_t second[] = {0x01, 0x1A, 0x00};

    protocol_service_feed(&service, first, sizeof(first), 300u);
    CHECK(fixture.response_count == 0u);
    protocol_service_feed(&service, second, sizeof(second), 301u);
    CHECK(fixture.response_count == 1u);
}

int main(void)
{
    test_service_reads_input_and_delivers_delayed_response();
    test_service_uses_runtime_delay_for_application_response();
    test_service_routes_control_without_application_delay();
    test_service_applies_effect_before_delivering_response();
    test_service_keeps_partial_frame_until_completed();

    puts("protocol_service: all tests passed");
    return EXIT_SUCCESS;
}
