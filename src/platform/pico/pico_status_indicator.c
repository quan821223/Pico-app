#include "pico_status_indicator.h"

#include "hardware/gpio.h"

static const board_profile_t *active_profile;
static bool indicator_available;

void pico_status_indicator_init(const board_profile_t *profile)
{
    active_profile = profile;
    indicator_available = profile != NULL &&
        profile->indicator_kind == BOARD_INDICATOR_GPIO &&
        profile->indicator_pin != BOARD_PIN_UNUSED;

    if (indicator_available) {
        gpio_init(profile->indicator_pin);
        gpio_set_dir(profile->indicator_pin, GPIO_OUT);
        gpio_put(profile->indicator_pin, 0);
    }
}

void pico_status_indicator_set(bool enabled)
{
    if (indicator_available) {
        gpio_put(active_profile->indicator_pin, enabled);
    }
}

void pico_status_indicator_toggle(void)
{
    if (indicator_available) {
        const uint8_t pin = active_profile->indicator_pin;
        gpio_put(pin, !gpio_get(pin));
    }
}

bool pico_status_indicator_is_available(void)
{
    return indicator_available;
}

