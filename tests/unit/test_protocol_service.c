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

static protocol_service_t create_service(fixture_t *fixture)
{
    const protocol_service_ports_t ports = {
        .read_chamber_status = read_chamber,
        .apply_effect = apply_effect,
        .response_ready = response_ready,
        .context = fixture,
    };
    protocol_service_t service;

    protocol_service_init(&service, &ports);
    return service;
}

static void test_service_reads_input_and_delivers_delayed_response(void)
{
    fixture_t fixture = {.chamber_status = 2u};
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
    test_service_applies_effect_before_delivering_response();
    test_service_keeps_partial_frame_until_completed();

    puts("protocol_service: all tests passed");
    return EXIT_SUCCESS;
}

