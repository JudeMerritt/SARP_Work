#include <stdint.h>
#include <stdbool.h>
#include "include/mmio.h"
#include "include/errc.h"

#pragma once

typedef enum {
    QSPI_MODE_NONE,
    QSPI_MODE_SINGLE,
    QSPI_MODE_DUAL,
    QSPI_MODE_QUAD
} qspi_mode_t;

typedef struct {
    uint8_t instruction;
    qspi_mode_t instruction_mode; // TODO: These structs are essetntially filled with everything you could possibly need. 
    uint32_t address;             //       Go back through your code and see if you can remove any of them. 
    qspi_mode_t address_mode;     //       It's almost always best to reduce the complexity of using the driver if possible.
    uint8_t address_size;
    uint8_t dummy_cycles;
    qspi_mode_t data_mode;
    uint32_t data_size;
} qspi_cmd_t;

typedef struct {
    uint8_t instruction;
    uint8_t match_value;
    uint8_t mask;
    uint16_t interval;
} qspi_poll_t;

ti_errc_t qspi_init();

ti_errc_t qspi_command(qspi_cmd_t *cmd, uint8_t *buf, bool is_read);

ti_errc_t qspi_poll_status(qspi_poll_t *poll_cfg);

ti_errc_t qspi_enter_memory_mapped(qspi_cmd_t *read_cmd);

ti_errc_t qspi_exit_memory_mapped();