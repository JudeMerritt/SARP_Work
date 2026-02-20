//Written by Jude Merritt

#pragma once

#include <stdint.h>

int spi_init(uint8_t inst);

int spi_transfer_sync(uint8_t inst, void* src, void* dst, uint8_t size);