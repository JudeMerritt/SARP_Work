// Written by Jude Merritt

#pragma once

#include <stdint.h>

/**
 * @brief Initialize an SPI interface.
 *
 * Prepares the specified SPI instance for communication. This must be called
 * before attempting any SPI transfers.
 *
 * @param inst  Identifier of the SPI instance to initialize.
 *
 * @return 1 if initialization succeeds.
 */
int spi_init(uint8_t inst);

/**
 * @brief Perform a blocking SPI data transfer.
 *
 * Sends data from the source buffer while simultaneously receiving data into
 * the destination buffer. The function does not return until the entire
 * transfer is complete.
 *
 * @param inst  SPI instance to use for the transfer.
 * @param src   Pointer to the transmit buffer.
 * @param dst   Pointer to the receive buffer.
 * @param size  Number of bytes to transfer.
 *
 * @return 1 when the transfer finishes successfully.
 */
int spi_transfer_sync(uint8_t inst, void* src, void* dst, uint8_t size);

// TODO: Revise comments after .c is compete. E.g. are their five instances, or more?