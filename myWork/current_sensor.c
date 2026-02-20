#include <stdint.h>
#include <stdbool.h> 
#include "include/errc.h"
#include "include/i2c.h"
#include "myWork/current_sensor.h"

static uint16_t _i2c_addr;
static float    _current_lsb;
static float    _power_lsb;

// INA228 Registers
#define REG_CONFIG     0x00
#define REG_ADC_CONFIG 0x01
#define REG_SHUNT_CAL  0x02
#define REG_VBUS       0x05
#define REG_DIETEMP    0x06
#define REG_CURRENT    0x07
#define REG_POWER      0x08

// Helper method to write to a 16-bit register. 
static ti_errc_t write_reg16(uint8_t reg, uint16_t value) {
    uint8_t pkt[3];
    pkt[0] = reg;
    pkt[1] = (uint8_t)(value >> 8);
    pkt[2] = (uint8_t)(value);
    
    return i2c_write_blocking(_i2c_addr, pkt, 3);
}

// Helper method to request data from INA228 then read it
static ti_errc_t read_reg(uint8_t reg, uint8_t* dest, size_t len) {
    ti_errc_t err = i2c_write_blocking(_i2c_addr, &reg, 1);
    if (err != TI_ERRC_NONE) {return err;}

    return i2c_read_blocking(_i2c_addr, dest, len);
}

current_sensor_init(current_sensor_config_t config) {
    _i2c_addr = config.i2c_address;

    // Calculate LSBs
    _current_lsb = config.max_expected_current / 524288; // divided by 2^19
    _power_lsb   = config.
}
