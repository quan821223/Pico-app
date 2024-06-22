#include "haw/HTU21D.h"
#include <stdio.h>

#define USER_REGISTER_WRITE 0xE6
#define USER_REGISTER_READ 0xE7

#define HEATER_REGISTER_WRITE 0xE6
#define HEATER_REGISTER_READ 0xE7

#define SOFT_RESET 0xFE

#define SERIAL1_READ1 0xFA
#define SERIAL1_READ2 0x0F
#define SERIAL2_READ1 0xFC
#define SERIAL2_READ2 0xC9

#define TRIGGER_TEMP_MEASURE_HOLD 0xE3
#define TRIGGER_TEMP_MEASURE_NOHOLD 0xF3

#define TRIGGER_HUMI_MEASURE_HOLD 0xE5
#define TRIGGER_HUMI_MEASURE_NOHOLD 0xF5

#define POLYNOMIAL 0x13100

int i2c_read_reg(struct i2c_information *i2c, const uint8_t reg, uint8_t *buf, const size_t len)
{
    i2c_write_blocking(i2c->instance, i2c->address, &reg, 1, true);
    return i2c_read_blocking(i2c->instance, i2c->address, buf, len, false);
}

int i2c_write(struct i2c_information *i2c, const uint8_t data)
{
    return i2c_write_blocking(i2c->instance, i2c->address, &data, 1, false);
}

const uint8_t check_crc8(uint16_t data)
{
    for (uint8_t bit = 0; bit < 16; bit++)
    {
        if (data & 0x8000)
            data = (data << 1) ^ POLYNOMIAL;
        else
            data <<= 1;
    }

    return data >>= 8;
}

struct htu21d htu21d_init(i2c_inst_t *i2c_instance)
{
    struct htu21d htu21d;
    htu21d.i2c.instance = i2c_instance;
    htu21d.i2c.address = HTU21D_ADDRESS;

    htu21d.raw_humidity = 0;
    htu21d.raw_temperature = 0;

    htu21d_set_temperature_measuring(&htu21d, 0);
    htu21d_set_humidity_measuring(&htu21d, 0);
    htu21d_set_resolution(&htu21d, HTU21D_RES_HUMI12_TEMP14);

    return htu21d;
}

void htu21d_set_resolution(struct htu21d *self, enum HTU21D_RESOLUTION resolution)
{
    uint8_t user_register = 0;
    i2c_read_reg(&self->i2c, USER_REGISTER_READ, &user_register, 1);
    user_register &= 0x7E;
    user_register |= resolution;

    uint8_t data[2] = {USER_REGISTER_WRITE, user_register};
    i2c_write_blocking(self->i2c.instance, self->i2c.address, (uint8_t *)data, 2, false);

    self->configuration.resolution = resolution;
}

void htu21d_soft_reset(struct htu21d *self)
{
    static const uint8_t htu21d_soft_reset_delay_ms = 15;
    i2c_write(&self->i2c, SOFT_RESET);
    sleep_ms(htu21d_soft_reset_delay_ms);
}

void htu21d_set_heater(struct htu21d *self, enum HTU21D_HEATER_SWITCH heater_switch)
{
    uint8_t user_register = 0;
    i2c_read_reg(&self->i2c, USER_REGISTER_READ, &user_register, 1);
    uint8_t data[2] = {USER_REGISTER_WRITE, user_register};

    if (heater_switch == HTU21D_HEATER_ON)
        user_register |= heater_switch;
    else
        user_register &= heater_switch;

    i2c_write_blocking(self->i2c.instance, self->i2c.address, (uint8_t *)data, 2, false);
}

void htu21d_set_temperature_measuring(struct htu21d *self, uint8_t state)
{
    if (state == 0)
        self->configuration.read_temperature = 0;
    else
        self->configuration.read_temperature = 1;
}

void htu21d_set_humidity_measuring(struct htu21d *self, uint8_t state)
{
    if (state == 0)
        self->configuration.read_humidity = 0;
    else
        self->configuration.read_humidity = 1;
}

int htu21d_event(struct htu21d *self)
{
    int result = 1;
    self->raw_temperature = 0;
    self->raw_humidity = 0;
    self->temperatureC = 0.0;
    self->temperatureF = 0.0;
    self->humidity = 0.0;

    if (self->configuration.read_temperature)
    {
        int8_t qntRequest = 3;
        uint8_t checksum = 0;

        result = i2c_write(&self->i2c, TRIGGER_TEMP_MEASURE_HOLD);

        switch (self->configuration.resolution)
        {
        case HTU21D_RES_HUMI12_TEMP14:
            sleep_ms(85);
            break;
        case HTU21D_RES_HUMI10_TEMP13:
            sleep_ms(43);
            break;
        case HTU21D_RES_HUMI8_TEMP12:
            sleep_ms(22);
            break;
        case HTU21D_RES_HUMI11_TEMP11:
            sleep_ms(11);
            break;
        }

        uint8_t data[3];
        int res = i2c_read_blocking(self->i2c.instance, self->i2c.address, (uint8_t *)data, 3, false);

        self->raw_temperature = (data[0] << 8) | data[1];
        checksum = data[2];

        if (check_crc8(self->raw_temperature) != checksum)
            return PICO_ERROR_GENERIC;
    }

    if (self->configuration.read_humidity)
    {
        uint8_t checksum = 0;

        result = i2c_write(&self->i2c, TRIGGER_HUMI_MEASURE_HOLD);

        switch (self->configuration.resolution)
        {
        case HTU21D_RES_HUMI12_TEMP14:
            sleep_ms(29);
            break;
        case HTU21D_RES_HUMI10_TEMP13:
            sleep_ms(9);
            break;
        case HTU21D_RES_HUMI8_TEMP12:
            sleep_ms(4);
            break;
        case HTU21D_RES_HUMI11_TEMP11:
            sleep_ms(15);
            break;
        }

        uint8_t data[3];
        i2c_read_blocking(self->i2c.instance, self->i2c.address, (uint8_t *)data, 3, false);
        self->raw_humidity = (data[0] << 8) | data[1];
        checksum = data[2];

        if (check_crc8(self->raw_humidity) != checksum)
            return PICO_ERROR_GENERIC;

        self->raw_humidity ^= 0x02;
    }

    return result;
}

double htu21d_get_temperature_c(struct htu21d *self)
{
    if (self->temperatureC != 0.0)
    {
        return self->temperatureC;
    }
    else if (self->temperatureC == 0 && self->raw_temperature != 0)
    {
        self->temperatureC = (0.002681 * (double)self->raw_temperature - 46.85);
        return self->temperatureC;
    }

    return 0.0;
}

double htu21d_get_temperature_f(struct htu21d *self)
{
    if (self->temperatureF != 0.0)
    {
        return self->temperatureF;
    }
    else if (htu21d_get_temperature_c(self) != 0.0)
    {
        self->temperatureF = htu21d_get_temperature_c(self) * 1.8 + 32;
        return self->temperatureF;
    }

    return 0.0;
}

double htu21d_get_humidity(struct htu21d *self)
{
    if (self->humidity != 0.0)
    {
        return self->humidity;
    }
    else if (self->humidity == 0.0 && self->raw_humidity != 0)
    {
        self->humidity = 0.001907 * (double)self->raw_humidity - 6;

        if (self->humidity > 100)
            self->humidity = 100.0;
        else if (self->humidity < 0)
            self->humidity = 0.0;

        return self->humidity;
    }

    return 0.0;
}