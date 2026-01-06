/**
 * This file is part of the Titan Flight Computer Project
 * Copyright (c) 2025 UW SARP
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @file util/include/delay.c
 * @authors Jude Merritt
 * @brief SysTick delay utility
 */

#include <stdint.h>
#include "internal/include/mmio.h"
#include "util/include/errc.h"
#include "periphs/include/systick.h"

// ((480 * 1,000,000) * 0.001 - 1)
#define RELOAD_VAL 0x752FF

ti_errc_t systick_init() {
    //Program reload value
    WRITE_FIELD(STK_RVR, STK_RVR_RELOAD, RELOAD_VAL);

    //Set clock source 
    SET_FIELD(STK_CSR, STK_CSR_CLKSOURCE);

    //Enable SysTick
    SET_FIELD(STK_CSR, STK_CSR_ENABLE);

    return TI_ERRC_NONE;
}

ti_errc_t systick_delay(uint32_t delay) {
    if (delay == 0) return TI_ERRC_INVALID_ARG;

    //Run delay loop
    for (int i = 0; i < delay; i++) {   
        //Clear current value
        WRITE_FIELD(STK_CVR, STK_CVR_CURRENT, 0U);

        while (IS_FIELD_CLR(STK_CSR, STK_CSR_COUNTFLAG)){
            asm("NOP");
        }
    }

    return TI_ERRC_NONE;
}

/**
 * TODO: 
 * 1. Could do some error checks before returning TI_ERRC_NONE in the init function.
 * 
 * 2. For systick driver if you allow the client to specifiy the countdown time make sure that the 
 *    speed of the m7 core is divisable by their input. For example, 480Mhz is divisable by 1000 ms / 1 s. 
 *    You can warn the client of this fact -- that for best accuracy this fact must be true -- or you can 
 *    just return an error code. The former is probably best. 
 */