#include "app_protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static app_protocol_result_t handle(const uint8_t request[APP_PROTOCOL_REQUEST_SIZE])
{
    const app_protocol_inputs_t inputs = {.chamber_status = 2u};
    app_protocol_result_t result;

    app_protocol_handle(request, &inputs, &result);
    return result;
}

static void check_response(
    const app_protocol_result_t *result,
    const uint8_t *expected,
    size_t expected_length)
{
    CHECK(result->has_response);
    CHECK(result->response.length == expected_length);
    CHECK(memcmp(result->response.data, expected, expected_length) == 0);
}

static void test_fa_write_returns_ack_without_effect(void)
{
    const uint8_t request[] = {0xFA, 0x57, 0x01, 0x01, 0x01};
    const uint8_t expected[] = {0xC3, 0x0D, 0x0A};
    const app_protocol_result_t result = handle(request);

    check_response(&result, expected, sizeof(expected));
    CHECK(result.effect.type == APP_EFFECT_NONE);
}

static void test_da_touch_write_returns_effect_and_ack(void)
{
    const uint8_t request[] = {0xDA, 0x57, 0x03, 0x01, 0x02};
    const uint8_t expected[] = {0xC3, 0x0D, 0x0A};
    const app_protocol_result_t result = handle(request);

    check_response(&result, expected, sizeof(expected));
    CHECK(result.effect.type == APP_EFFECT_TOUCH_0);
    CHECK(result.effect.value == 2u);
}

static void test_da_chamber_write_uses_hex_category_0a(void)
{
    const uint8_t request[] = {0xDA, 0x57, 0x03, 0x0A, 0x02};
    const app_protocol_result_t result = handle(request);

    CHECK(result.has_response);
    CHECK(result.effect.type == APP_EFFECT_SET_CHAMBER_STATE);
    CHECK(result.effect.value == 2u);
}

static void test_da_micro_switch_reset_write_returns_ack(void)
{
    const uint8_t request[] = {0xDA, 0x57, 0x0B, 0x00, 0x00};
    const uint8_t expected[] = {0xC3, 0x0D, 0x0A};
    const app_protocol_result_t result = handle(request);

    check_response(&result, expected, sizeof(expected));
    CHECK(result.effect.type == APP_EFFECT_SET_RESET_MICRO_SWITCH);
    CHECK(result.effect.value == 0u);
}

static void test_da_micro_switch_legacy_reset_write_returns_ack(void)
{
    const uint8_t request[] = {0xDA, 0x57, 0x03, 0x0B, 0x00};
    const uint8_t expected[] = {0xC3, 0x0D, 0x0A};
    const app_protocol_result_t result = handle(request);

    check_response(&result, expected, sizeof(expected));
    CHECK(result.effect.type == APP_EFFECT_SET_RESET_MICRO_SWITCH);
    CHECK(result.effect.value == 0u);
}

static void test_da_write_to_other_device_is_silent(void)
{
    const uint8_t request[] = {0xDA, 0x57, 0x02, 0x01, 0x02};
    const app_protocol_result_t result = handle(request);

    CHECK(!result.has_response);
    CHECK(result.effect.type == APP_EFFECT_NONE);
}

static void test_unsupported_information_parameter_is_deterministic_silent(void)
{
    const uint8_t request[] = {0xFA, 0x52, 0x01, 0x00, 0x07};
    const app_protocol_result_t result = handle(request);

    CHECK(!result.has_response);
    CHECK(result.effect.type == APP_EFFECT_NONE);
}

static void test_da_micro_switch_read_changes_after_ten_reads(void)
{
    const uint8_t reset_request[] = {0xDA, 0x57, 0x0B, 0x00, 0x00};
    const uint8_t request[] = {0xDA, 0x52, 0x0B, 0x00, 0x00};
    const uint8_t expected_zero[] = {
        0xDA, 0x0B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x0A,
    };
    const uint8_t expected_one[] = {
        0xDA, 0x0B, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0D, 0x0A,
    };

    (void)handle(reset_request);
    for (size_t count = 0u; count < 9u; ++count) {
        const app_protocol_result_t result = handle(request);
        check_response(&result, expected_zero, sizeof(expected_zero));
    }

    app_protocol_result_t result = handle(request);
    check_response(&result, expected_one, sizeof(expected_one));

    result = handle(request);
    check_response(&result, expected_one, sizeof(expected_one));
}

static void test_touch_response_fits_fixed_capacity(void)
{
    const uint8_t request[] = {0xFA, 0x52, 0x01, 0x06, 0x00};
    const app_protocol_result_t result = handle(request);

    CHECK(result.has_response);
    CHECK(result.response.length == APP_PROTOCOL_MAX_RESPONSE_SIZE);
    CHECK(result.response.data[0] == 0xFAu);
    CHECK(result.response.data[64] == 0x0Du);
    CHECK(result.response.data[65] == 0x0Au);
}

int main(void)
{
    test_fa_write_returns_ack_without_effect();
    test_da_touch_write_returns_effect_and_ack();
    test_da_chamber_write_uses_hex_category_0a();
    test_da_micro_switch_reset_write_returns_ack();
    test_da_micro_switch_legacy_reset_write_returns_ack();
    test_da_write_to_other_device_is_silent();
    test_unsupported_information_parameter_is_deterministic_silent();
    test_da_micro_switch_read_changes_after_ten_reads();
    test_touch_response_fits_fixed_capacity();

    puts("application_protocol: all tests passed");
    return EXIT_SUCCESS;
}

