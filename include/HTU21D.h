/**
 * Texas Instruments HTU21D temperature and humidity sensor stand-alone library for pico sdk.
 * 
 * It makes the initial setup and communication with the sensor faster. The structure should always
 * be predictable, as in runtime only the event function will make an I2C communication. The other
 * functions are for calculations and setup.
 * 
 * @file HTU21D.h
 * @author Maik Steiger (maik.steiger@tu-dortmund.de)
 * @brief TI HTU21D sensor library for the Raspberry PI Pico.
 * @version 1.0
 * @date 2021-11-10
 * 
 * @copyright Copyright (c) 2021
 */
#ifndef HTU21D_H_
#define HTU21D_H_
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define HTU21D_ADDRESS 0x40

#ifdef __cplusplus
extern "C"
{
#endif
    enum HTU21D_RESOLUTION
    {
        HTU21D_RES_HUMI12_TEMP14 = 0x00,
        HTU21D_RES_HUMI8_TEMP12 = 0x01,
        HTU21D_RES_HUMI10_TEMP13 = 0x80,
        HTU21D_RES_HUMI11_TEMP11 = 0x81
    };

    enum HTU21D_HEATER_SWITCH
    {
        HTU21D_HEATER_ON = 0x04,
        HTU21D_HEATER_OFF = 0xFB
    };

#ifndef I2C_INFORMATION_S_
#define I2C_INFORMATION_S_
    struct i2c_information
    {
        i2c_inst_t *instance;
        uint8_t address;
    };
#endif

    struct htu21d_configuration
    {
        enum HTU21D_RESOLUTION resolution;
        uint8_t read_temperature;
        uint8_t read_humidity;
    };

    typedef struct htu21d
    {
        struct i2c_information i2c;
        struct htu21d_configuration configuration;
        uint16_t raw_temperature;
        uint16_t raw_humidity;
        double temperatureC;
        double temperatureF;
        double humidity;
    } htu21d_t;

    /**
     * @brief Initializes a HTU21D struct and returns it.
     * 
     * @param i2c_inst_t* i2c_instance: I2C bus instance. Needed for multicore applications. 
     * @return struct htu21d 
     */
    struct htu21d htu21d_init(i2c_inst_t *i2c_instance);

    /**
     * @brief Sets the measurement resolution, which is returned after an event call.
     * 
     * @param htu21d_t* self: Reference to itself
     * @param HTU21D_RESOLUTION resolution: Resolution of the measurements 
     */
    void htu21d_set_resolution(struct htu21d *self, enum HTU21D_RESOLUTION resolution);

    /**
     * @brief Software reset for the passed in device.
     * 
     * @param htu21d_t* self: Reference to itself
     */
    void htu21d_soft_reset(struct htu21d *self);

    /**
     * @brief Activates or deactivates the on-device heater.
     * 
     * @param htu21d_t* self: Reference to itself
     * @param HTU21D_HEATER_SWITCH heater_switch: Should the heater be turned on or off
     */
    void htu21d_set_heater(struct htu21d *self, enum HTU21D_HEATER_SWITCH heater_switch);

    /**
     * @brief Fetches all the data from the device. The results are getting stored into their buffers.
     * 
     * @param htu21d_t* self: Reference to itself
     * @return int: If everything went correct it should return 1, otherwise 0
     */
    int htu21d_event(struct htu21d *self);

    /**
     * @brief Enables or disables temperature measurements readings.
     * If the state is set to 1, then temperature will be read after an event call.
     * Otherwise temperature readings will be skipped.
     * 
     * @param htu21d_t* self: Reference to itself
     * @param uint8_t state: 1 to enable or 0 to disable 
     */
    void htu21d_set_temperature_measuring(struct htu21d *self, uint8_t state);

    /**
     * @brief Enables or disables humidity measurements readings.
     * If the state is set to 1, then humidity will be read after an event call.
     * Otherwise humidity readings will be skipped.
     * 
     * @param htu21d_t* self: Reference to itself
     * @param uint8_t state: 1 to enable or 0 to disable 
     */
    void htu21d_set_humidity_measuring(struct htu21d *self, uint8_t state);

    /**
     * @brief Returns the fetched temperature in celcius.
     * You must call htu21d_event before calling this function.
     * 
     * @param htu21d_t* self: Reference to itself
     * @return double: Temperature in celsius
     */
    double htu21d_get_temperature_c(struct htu21d *self);

    /**
     * @brief Returns the fetched temperature in fahrenheit.
     * You must call htu21d_event before calling this function.
     * 
     * @param htu21d_t* self: Reference to itself
     * @return double: Temperature in fahrenheit
     */
    double htu21d_get_temperature_f(struct htu21d *self);

    /**
     * @brief Returns the fetched air humidity.
     * You must call htu21d_event before calling this function.
     * 
     * @param htu21d_t* self: Reference to itself
     * @return double: Humidity in a scala from 0-100
     */
    double htu21d_get_humidity(struct htu21d *self);

#ifdef __cplusplus
}
#endif
#endif