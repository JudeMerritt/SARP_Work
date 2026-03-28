// Written by Jude Merritt

#include <stdint.h>
#include "myWork/adc.h"
#include "myWork/new_spi.h"
#include "myWork/systick.h"
#include "include/mmio.h"
#include "include/errc.h"

#define RESET 0x06
#define START 0x08
#define STOP  0x0A
#define RDATA 0x12
#define STATUS_REG 0x01
// The largest transfer that will be done with the spi_rreg function
#define MAX_RREG_SIZE 6

struct adc_spi_dev DEV;

static int spi_rreg(uint8_t reg_addr, uint8_t data_size, ti_errc_t* errc) {
    if (data_size > (MAX_RREG_SIZE - 2) || data_size == 0) {
        *errc = TI_ERRC_INVALID_ARG;
        return -1;
    }

    uint8_t src[MAX_RREG_SIZE] = {0};
    uint8_t dst[MAX_RREG_SIZE] = {0};

    src[0] = 0x20 | reg_addr;
    src[1] = data_size - 1;

    // Two command bytes + the number of registers to read
    uint8_t tot_size = 2 + data_size;
    *errc = spi_transfer_sync(DEV.inst, DEV.ss_pin, src, dst, tot_size); // TODO: Make sure that SPI is returning an actual error code

    if (*errc != TI_ERRC_NONE) {
        return -1;
    }

    uint32_t result = 0;
    for (int i = 0; i < data_size; i++) {
        result = (result << 8) | dst[2 + i];
    }

    return result;
}

static int spi_wreg(uint8_t reg_addr, uint16_t data_size, uint32_t data, ti_errc_t* errc) {
    if (data_size > (MAX_RREG_SIZE - 2) || data_size == 0) {
        *errc = TI_ERRC_INVALID_ARG;
        return -1;
    }

    uint8_t src[MAX_RREG_SIZE] = {0};
    uint8_t dst[MAX_RREG_SIZE] = {0};

    src[0] = 0x40 | reg_addr;
    src[1] = data_size - 1;

    for (int i = 0; i < data_size; i++) {
        // Move data into src while accounting for MSB formatting
        src[2 + i] = (uint8_t)(data >> (8 * (data_size - 1 - i)));
    }

    uint8_t tot_size = 2 + data_size;
    *errc = spi_transfer_sync(DEV.inst, DEV.ss_pin, src, dst, tot_size);

    if (*errc != TI_ERRC_NONE) {
        return -1;
    }

    return 1;
}

static int32_t spi_single_command(uint8_t cmd, uint8_t transfer_size) {
    if (transfer_size < 1 || transfer_size > 4) {
        return -1;
    }

    uint8_t src[4] = {cmd, 0, 0, 0};
    uint8_t dst[4] = {0, 0, 0, 0};

    spi_transfer_sync(DEV.inst, DEV.ss_pin, src, dst, transfer_size);

    if (transfer_size == 1) {
        return dst[0];
    } else {
        int32_t result = (dst[1] << 16) | (dst[2] << 8) | dst[3];
        return result;
    } 
}

void adc_init(struct adc_spi_dev *dev, ti_errc_t* errc) {
    if (dev->inst < 1 || dev->inst > 6) {
        *errc = TI_ERRC_INVALID_ARG;
        return;
    }

    *errc = TI_ERRC_NONE;
    DEV  = *dev;

    // Reset ADC
    uint8_t err = spi_single_command(RESET, 1);
    if (err == -1 || *errc != TI_ERRC_NONE) {
        return;
    }

    // Delay recommended by datasheet after RESET
    systick_delay(5);

    // Wait until ADC is ready for communication
    bool is_ready = false;
    int timeout = 100000; 
    while (!is_ready) {
        uint8_t status_reg = spi_rreg(STATUS_REG, 1, errc);
        
        if ((status_reg & 0x40) == 0 && *errc == TI_ERRC_NONE) {
            is_ready = true;
        } else if (timeout == 0) {
            *errc = TI_ERRC_TIMEOUT;
            return; 
        }

        timeout--;
    }

    // Enable internal reference 
    spi_wreg(0x05, 1, 0x12, errc);
}

int adc_read_voltage(const struct adc_channel* channel, ti_errc_t* errc) {
    if (DEV.inst < 1 || DEV.inst > 6 || !channel) {
        *errc = TI_ERRC_INVALID_ARG;
        return -1;
    }

    *errc = TI_ERRC_NONE;

    // Configure Input Multiplexer
    uint8_t mux_val = (channel->pos_pin << 4) | (channel->neg_pin & 0x0F);
    spi_wreg(0x02, 1, mux_val, errc); // Returns -1 if unexpected return, otherwise 1

    // Set gain
    uint8_t pga_val = 0x08 | (channel->gain & 0x07);
    spi_wreg(0x03, 1, pga_val, errc);

    // Set reference voltage
    uint8_t ref_val = 0x12 | ((channel->source & 0x03) << 2);
    spi_wreg(0x05, 1, ref_val, errc);

    if (*errc != TI_ERRC_NONE) {
        return -1;
    }

    // Wait for device ready flag
    int timeout = 100;
    while ((spi_rreg(STATUS_REG, 1, errc) & 0x40) != 0 && timeout > 0) {
        timeout--;
    }

    // Request data
    int32_t result = spi_single_command(RDATA, 4);

    // If result is negative, ensure that top eight bit are flipped to 1
    if (result & 0x800000) {
        result |= 0xFF000000;
    }

    // Voltage conversion math
    float divisor = (float)((1 << 23) - 1); 

    // Convert the 3-bit gain code (0-7) into the actual multiplier (1, 2, 4... 128)
    float actual_gain = (float)(1 << (channel->gain & 0x07));

    float final_voltage = ((float)result * channel->ref_voltage) / (actual_gain * divisor);

    return (int32_t)(final_voltage * 1000);
}

int adc_read_voltage_diff(struct adc_channel channel1, struct adc_channel channel2, ti_errc_t* errc) { 
    int32_t voltage1 = adc_read_voltage(&channel1, errc);
    int32_t voltage2 = adc_read_voltage(&channel2, errc);

    return voltage1 - voltage2;
}

// You don't need to disconnect a pin to change the idac pins
void adc_set_idac(enum idac_mag magnitude, enum adc_pin pin1, enum adc_pin pin2, ti_errc_t* errc) {
    if (DEV.inst < 1 || DEV.inst > 6) {
        *errc = TI_ERRC_INVALID_ARG;
        return;
    }

    // Set IDAC magnitude
    spi_wreg(0x06, 1, magnitude, errc);

    if (*errc != TI_ERRC_NONE) {
        return;
    }

    uint8_t mux_pins = ((pin2 & 0x0F) << 4) | (pin1 & 0x0F);
    spi_wreg(0x07, 1, mux_pins, errc);
}

void adc_set_gpio(enum adc_pin pin, bool default_high, bool input, ti_errc_t* errc) {
    if (DEV.inst < 1 || DEV.inst > 6) {
        *errc = TI_ERRC_INVALID_ARG;
        return;
    }

    // Only ADC pins 8 - 11 work
    uint8_t gpio_idx = pin - 0x08;

    // Enable GPIO function
    uint8_t gpiocon_val;
    gpiocon_val |= (1 << gpio_idx);
    spi_wreg(0x11, 1, gpiocon_val, errc);
    if (*errc != TI_ERRC_NONE) {
        return;
    }

    uint8_t gpiodat_val;
    if (default_high && input) {
        gpiodat_val = 1 << (gpio_idx + 4) | gpio_idx;
    } else if (default_high && !input) {
        gpiodat_val = 1 << gpio_idx;
    } else if (!default_high && input) {
        gpiodat_val = 1 << (gpio_idx + 4);
    } 

    // Set as output/input and default high/low
    spi_wreg(0x10, 1, gpiodat_val, errc);
}

char* adc_get_channel_name(struct adc_channel channel) {
    return channel.name;
}


/**
 * Notes:
 * 1. Start and reset pins are perminently tied to high, clk is tied to low, and data ready is left hanging. 
 * Only standard spi pins are used.
 * 
 * 2. TODO: Reduce magic numbers. For example STATUS_REG & 0x40 is RDY_FLAG.
 * 
 * 3. TODO: Double check voltage math. Would prob be good to have someone else look through it and make sure 
 * it tracks the documentation. Are floats ok? 
 */