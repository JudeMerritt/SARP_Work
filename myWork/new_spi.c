#include "myWork/new_spi.h"
#include "include/mmio.h"
#include <stdint.h>

enum inst {
    INST_ONE = 1,
    INST_TWO,
    INST_THREE,
    INST_FOUR,
    INST_FIVE
};

int spi_init(uint8_t inst) {
    // Configure pins
    switch (inst) {
        case INST_ONE:
            SET_FIELD(RCC_AHB4ENR, RCC_AHB4ENR_GPIOAEN);
            // Set push pull
            CLR_FIELD(GPIOx_OTYPER[0], GPIOx_OTYPER_OTx[4]);
            CLR_FIELD(GPIOx_OTYPER[0], GPIOx_OTYPER_OTx[5]);
            CLR_FIELD(GPIOx_OTYPER[0], GPIOx_OTYPER_OTx[7]);
            // Set mode 
            WRITE_FIELD(GPIOx_MODER[0], GPIOx_MODER_MODEx[4], 0b01);
            WRITE_FIELD(GPIOx_MODER[0], GPIOx_MODER_MODEx[5], 0b10);
            WRITE_FIELD(GPIOx_MODER[0], GPIOx_MODER_MODEx[6], 0b10);
            WRITE_FIELD(GPIOx_MODER[0], GPIOx_MODER_MODEx[7], 0b10);
            // Set alternate mode
            WRITE_FIELD(GPIOx_AFRL[0], GPIOx_AFRL_AFSELx[5], 0b101);
            WRITE_FIELD(GPIOx_AFRL[0], GPIOx_AFRL_AFSELx[6], 0b101);
            WRITE_FIELD(GPIOx_AFRL[0], GPIOx_AFRL_AFSELx[7], 0b101);
            // Set high speed
            WRITE_FIELD(GPIOx_OSPEEDR[0],GPIOx_OSPEEDR_OSPEEDx[5], 0b11);
            WRITE_FIELD(GPIOx_OSPEEDR[0],GPIOx_OSPEEDR_OSPEEDx[6], 0b11);
            WRITE_FIELD(GPIOx_OSPEEDR[0],GPIOx_OSPEEDR_OSPEEDx[7], 0b11);
            break;
        case INST_TWO:
            // ...
            break;
        case INST_THREE:
            // ...
            break;
        case INST_FOUR:
            // ...
            break;
        case INST_FIVE:
            // ...
            break;
    }

    SET_FIELD(RCC_CR, RCC_CR_HSION);
    while(!READ_FIELD(RCC_CR, RCC_CR_HSIRDY));

    // Set clock source
    if (inst < 4) {
        WRITE_FIELD(RCC_D2CCIP1R, RCC_D2CCIP1R_SPI123SRC, 0b100);
    } else {
        // other one
    }
    // Enable SPI clock
    switch (inst) {
        case INST_ONE:
            SET_FIELD(RCC_APB2ENR, RCC_APB2ENR_SPI1EN);
            break;
        case INST_TWO:
            // ...
            break;
        case INST_THREE:
            // ...
            break;
        case INST_FOUR:
            // ...
            break;
        case INST_FIVE:
            // ...
            break;
    }

    // Ensure SPI hardware is disabled before config
    CLR_FIELD(SPIx_CR1[inst], SPIx_CR1_SPE);
    while(READ_FIELD(SPIx_CR1[inst], SPIx_CR1_SPE));

    CLR_FIELD(SPIx_CGFR[inst], SPIx_CGFR_I2SMOD);

    SET_FIELD(GPIOx_ODR[0], GPIOx_ODR_ODx[4]);

    WRITE_FIELD(SPIx_CFG1[inst], SPIx_CFG1_FTHVL, 0x0);

    // Set baudrate prescaler ( /64 )
    WRITE_FIELD(SPIx_CFG1[inst], SPIx_CFG1_MBR, 0b101); // TODO: Should be 000?
    // Set data size
    WRITE_FIELD(SPIx_CFG1[inst], SPIx_CFG1_DSIZE, 0b00111);
    // Set clock polarities
    CLR_FIELD(SPIx_CFG2[inst], SPIx_CFG2_CPOL);
    CLR_FIELD(SPIx_CFG2[inst], SPIx_CFG2_CPHA);
    // Slave management
    SET_FIELD(SPIx_CFG2[inst], SPIx_CFG2_SSM);
    SET_FIELD(SPIx_CR1[inst], SPIx_CR1_SSI);
    // Set SPI as master
    SET_FIELD(SPIx_CFG2[inst], SPIx_CFG2_MASTER);

    //WRITE_WO_FIELD(SPIx_IFCR[inst], SPIx_IFCR_MODFC, 1U);

    // Enable SPI
    SET_FIELD(SPIx_CR1[inst], SPIx_CR1_SPE);

    return 1;
}

int spi_transfer_sync(uint8_t inst, void* src, void* dst, uint8_t size) {
    CLR_FIELD(GPIOx_ODR[0], GPIOx_ODR_ODx[4]);

    WRITE_FIELD(SPIx_CR2[inst], SPIx_CR2_TSIZE, size);


    while (!READ_FIELD(SPIx_SR[inst], SPIx_SR_TXP));

    *(volatile uint8_t *)SPIx_TXDR[inst] = ((uint8_t *)src)[0];

    SET_FIELD(SPIx_CR1[inst], SPIx_CR1_CSTART);

    for (int i = 0; i < size; i++) {
        if (i > 0) {
            while (!READ_FIELD(SPIx_SR[inst], SPIx_SR_TXP));

            *(volatile uint8_t *)SPIx_TXDR[inst] = ((uint8_t *)src)[i];
        }
        

        while (!READ_FIELD(SPIx_SR[inst], SPIx_SR_RXP) && !READ_FIELD(SPIx_SR[inst], SPIx_SR_EOT));

        ((uint8_t *)dst)[i] = *(volatile uint8_t *)SPIx_RXDR[inst];
    }

    while (!READ_FIELD(SPIx_SR[inst], SPIx_SR_EOT));

    SET_FIELD(SPIx_IFCR[inst], SPIx_IFCR_EOTC);

    SET_FIELD(GPIOx_ODR[0], GPIOx_ODR_ODx[4]);

    return 1;
}