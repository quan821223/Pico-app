#include "board_profile.h"

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

static void check_io_pins_are_unique(const board_profile_t *profile)
{
    const uint8_t pins[] = {
        profile->debug_uart_tx_pin,
        profile->debug_uart_rx_pin,
        profile->chamber_output_0_pin,
        profile->chamber_output_1_pin,
        profile->chamber_address_0_pin,
        profile->chamber_address_1_pin,
        profile->touch_output_0_pin,
        profile->touch_output_1_pin,
    };

    for (size_t left = 0; left < sizeof(pins); ++left) {
        CHECK(pins[left] != BOARD_PIN_UNUSED);
        if (profile->indicator_pin != BOARD_PIN_UNUSED) {
            CHECK(pins[left] != profile->indicator_pin);
        }
        for (size_t right = left + 1u; right < sizeof(pins); ++right) {
            CHECK(pins[left] != pins[right]);
        }
    }
}

static void test_all_profiles_are_valid_and_unique(void)
{
    for (board_id_t id = BOARD_ID_PICO; id < BOARD_ID_COUNT; ++id) {
        const board_profile_t *profile = board_profile_get(id);

        CHECK(board_profile_is_valid(profile));
        CHECK(profile->id == id);
        check_io_pins_are_unique(profile);

        for (board_id_t other = (board_id_t)(id + 1); other < BOARD_ID_COUNT;
             ++other) {
            const board_profile_t *other_profile = board_profile_get(other);
            CHECK(strcmp(profile->pico_sdk_board, other_profile->pico_sdk_board) != 0);
        }
    }
}

static void test_expected_board_specific_differences(void)
{
    const board_profile_t *pico = board_profile_get(BOARD_ID_PICO);
    const board_profile_t *pico_w = board_profile_get(BOARD_ID_PICO_W);
    const board_profile_t *zero = board_profile_get(BOARD_ID_WAVESHARE_RP2040_ZERO);
    const board_profile_t *pico2 = board_profile_get(BOARD_ID_PICO2);

    CHECK(pico->indicator_kind == BOARD_INDICATOR_GPIO);
    CHECK(pico_w->indicator_kind == BOARD_INDICATOR_CYW43);
    CHECK(zero->indicator_kind == BOARD_INDICATOR_WS2812);
    CHECK(zero->indicator_pin == 16u);
    CHECK(zero->debug_uart_tx_pin == 0u);
    CHECK(zero->debug_uart_rx_pin == 1u);
    CHECK(pico2->mcu_family == BOARD_MCU_RP2350);
    CHECK(pico2->flash_size_bytes == 4u * 1024u * 1024u);
}

static void test_active_profile_uses_compile_time_selection(void)
{
    CHECK(board_profile_active() == board_profile_get(BOARD_ID_PICO));
    CHECK(board_profile_get(BOARD_ID_COUNT) == NULL);
}

int main(void)
{
    test_all_profiles_are_valid_and_unique();
    test_expected_board_specific_differences();
    test_active_profile_uses_compile_time_selection();

    puts("board_profile: all tests passed");
    return EXIT_SUCCESS;
}
