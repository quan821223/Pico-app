#include "pico_app_io.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include <stddef.h>

typedef struct {
    uint gpio_pin;
    repeating_timer_t timer;
} touch_output_t;

static touch_output_t touch_outputs[] = {
    {BOARD_PIN_UNUSED, {0}},
    {BOARD_PIN_UNUSED, {0}},
};

static const board_profile_t *active_profile;
static bool chamber_backdoor_enabled;
static uint8_t chamber_backdoor_value;

static bool release_touch_output(struct repeating_timer *timer)
{
    touch_output_t *output = timer->user_data;

    gpio_put(output->gpio_pin, 0);
    return false;
}

static void start_touch(size_t index, uint8_t steps)
{
    touch_output_t *output;

    if (index >= (sizeof(touch_outputs) / sizeof(touch_outputs[0]))) {
        return;
    }

    output = &touch_outputs[index];
    gpio_put(output->gpio_pin, 1);
    add_repeating_timer_ms(
        (int32_t)steps * 100,
        release_touch_output,
        output,
        &output->timer);
}

static void set_chamber_state(uint8_t state)
{
    switch (state) {
        case 0u:
            gpio_put(active_profile->chamber_output_0_pin, 0);
            gpio_put(active_profile->chamber_output_1_pin, 0);
            break;
        case 1u:
            gpio_put(active_profile->chamber_output_0_pin, 1);
            gpio_put(active_profile->chamber_output_1_pin, 0);
            break;
        case 2u:
            gpio_put(active_profile->chamber_output_0_pin, 0);
            gpio_put(active_profile->chamber_output_1_pin, 1);
            break;
        case 3u:
        default:
            gpio_put(active_profile->chamber_output_0_pin, 1);
            gpio_put(active_profile->chamber_output_1_pin, 1);
            break;
    }
}

bool pico_app_io_init(const board_profile_t *profile)
{
    if (!board_profile_is_valid(profile)) {
        return false;
    }

    active_profile = profile;
    touch_outputs[0].gpio_pin = profile->touch_output_0_pin;
    touch_outputs[1].gpio_pin = profile->touch_output_1_pin;

    gpio_init(profile->chamber_output_0_pin);
    gpio_set_dir(profile->chamber_output_0_pin, GPIO_OUT);
    gpio_init(profile->chamber_output_1_pin);
    gpio_set_dir(profile->chamber_output_1_pin, GPIO_OUT);
    set_chamber_state(3u);

    gpio_init(profile->chamber_address_0_pin);
    gpio_set_dir(profile->chamber_address_0_pin, GPIO_IN);
    gpio_pull_up(profile->chamber_address_0_pin);
    gpio_init(profile->chamber_address_1_pin);
    gpio_set_dir(profile->chamber_address_1_pin, GPIO_IN);
    gpio_pull_up(profile->chamber_address_1_pin);

    for (size_t index = 0;
         index < (sizeof(touch_outputs) / sizeof(touch_outputs[0]));
         ++index) {
        gpio_init(touch_outputs[index].gpio_pin);
        gpio_set_dir(touch_outputs[index].gpio_pin, GPIO_OUT);
        gpio_put(touch_outputs[index].gpio_pin, 0);
    }

    chamber_backdoor_enabled = false;
    chamber_backdoor_value = 0u;
    return true;
}

uint8_t pico_app_io_read_chamber_status(void)
{
    if (chamber_backdoor_enabled) {
        return chamber_backdoor_value;
    }

    if (active_profile == NULL) {
        return 0u;
    }

    return (uint8_t)((gpio_get(active_profile->chamber_address_1_pin) << 1u) |
        gpio_get(active_profile->chamber_address_0_pin));
}

void pico_app_io_apply_effect(const app_effect_t *effect)
{
    if (effect == NULL) {
        return;
    }

    switch (effect->type) {
        case APP_EFFECT_TOUCH_0:
            start_touch(0u, effect->value);
            break;
        case APP_EFFECT_TOUCH_1:
            start_touch(1u, effect->value);
            break;
        case APP_EFFECT_SET_CHAMBER_STATE:
            set_chamber_state(effect->value);
            break;
        case APP_EFFECT_NONE:
        default:
            break;
    }
}

void pico_app_io_set_chamber_backdoor_enabled(bool enabled)
{
    chamber_backdoor_enabled = enabled;
    chamber_backdoor_value = 3u;
}

bool pico_app_io_set_chamber_backdoor_value(uint8_t value)
{
    if (value > 3u) {
        return false;
    }

    chamber_backdoor_value = value;
    return true;
}
