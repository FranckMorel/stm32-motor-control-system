/******************************************************************************
 * @file    spi.c
 * @author  Morel
 * @brief   SPI1 driver for ST7735S TFT communication.
 *
 * This module configures SPI1 in master mode and provides blocking transmit
 * and receive functions. SPI1 is used to communicate with the ST7735S TFT
 * display.
 *
 * Pin mapping:
 *  - PA5: SPI1_SCK  (AF5)
 *  - PA7: SPI1_MOSI (AF5)
 *
 * Configuration:
 *  - Master mode
 *  - SPI mode 0: CPOL = 0, CPHA = 0
 *  - 8-bit data frame
 *  - MSB first
 *  - Software slave management enabled
 ******************************************************************************/

#include "stm32f401xe.h"
#include <stdint.h>

#define SPI1_ENABLE                 (1U << 12)

#define SPI1_CPHA                   (1U << 0)
#define SPI1_CPOL                   (1U << 1)
#define SPI1_BIDIMODE               (1U << 15)
#define SPI1_LSBFIRST               (1U << 7)
#define SPI1_MASTER                 (1U << 2)
#define SPI1_DFF_16BIT              (1U << 11)
#define SPI1_SSM                    (1U << 9)
#define SPI1_SSI                    (1U << 8)
#define SPI1_ENABLE_BIT             (1U << 6)

#define SPI_SR_TXE_FLAG             (1U << 1)
#define SPI_SR_BSY_FLAG             (1U << 7)
#define SPI_SR_RXNE_FLAG            (1U << 0)

#define SPI1_AF                     5U
#define SPI1_SCK_PIN                5U
#define SPI1_MOSI_PIN               7U

#define GPIO_MODE_AF                2U
#define GPIO_AF_MASK                15U

#define SPI_BAUDRATE_DIV8           2U

/******************************************************************************
 * @brief Configures GPIO pins used by SPI1.
 *
 * PA5 and PA7 are configured in alternate function mode AF5:
 *  - PA5: SPI1_SCK
 *  - PA7: SPI1_MOSI
 *
 * GPIOA clock is enabled at board level before this function is called.
 ******************************************************************************/
void spi_gpio_init(void)
{
    /* PA5: SPI1_SCK, Alternate Function AF5 */
    GPIOA->MODER &= ~(3U << (SPI1_SCK_PIN * 2U));
    GPIOA->MODER |=  (GPIO_MODE_AF << (SPI1_SCK_PIN * 2U));

    GPIOA->AFR[0] &= ~(GPIO_AF_MASK << (SPI1_SCK_PIN * 4U));
    GPIOA->AFR[0] |=  (SPI1_AF << (SPI1_SCK_PIN * 4U));

    /* PA7: SPI1_MOSI, Alternate Function AF5 */
    GPIOA->MODER &= ~(3U << (SPI1_MOSI_PIN * 2U));
    GPIOA->MODER |=  (GPIO_MODE_AF << (SPI1_MOSI_PIN * 2U));

    GPIOA->AFR[0] &= ~(GPIO_AF_MASK << (SPI1_MOSI_PIN * 4U));
    GPIOA->AFR[0] |=  (SPI1_AF << (SPI1_MOSI_PIN * 4U));
}

/******************************************************************************
 * @brief Configures and enables SPI1.
 *
 * SPI1 is configured as a blocking master transmitter for the ST7735S display.
 * The baud rate is set to fPCLK / 8. With APB2 running at 84 MHz, this results
 * in an SPI clock of approximately 10.5 MHz, which is below the typical maximum
 * SPI clock supported by the ST7735S display module.
 ******************************************************************************/
void spi1_config(void)
{
    RCC->APB2ENR |= SPI1_ENABLE;

    /* Disable SPI before changing configuration bits. */
    SPI1->CR1 &= ~SPI1_ENABLE_BIT;

    /* Baud rate: fPCLK / 8 */
    SPI1->CR1 &= ~(7U << 3U);
    SPI1->CR1 |=  (SPI_BAUDRATE_DIV8 << 3U);

    /* SPI mode 0: CPOL = 0, CPHA = 0 */
    SPI1->CR1 &= ~SPI1_CPOL;
    SPI1->CR1 &= ~SPI1_CPHA;

    /* 2-line unidirectional data mode */
    SPI1->CR1 &= ~SPI1_BIDIMODE;

    /* MSB first */
    SPI1->CR1 &= ~SPI1_LSBFIRST;

    /* Master mode */
    SPI1->CR1 |= SPI1_MASTER;

    /* 8-bit data frame */
    SPI1->CR1 &= ~SPI1_DFF_16BIT;

    /* Software slave management */
    SPI1->CR1 |= SPI1_SSM;
    SPI1->CR1 |= SPI1_SSI;

    /* Enable SPI1 */
    SPI1->CR1 |= SPI1_ENABLE_BIT;
}

/******************************************************************************
 * @brief Transmits a data buffer over SPI1.
 *
 * This function uses blocking polling. It waits until the transmit buffer is
 * empty before writing each byte. After the last byte, it waits until SPI is no
 * longer busy before returning.
 *
 * @param data Pointer to the transmit data buffer.
 * @param size Number of bytes to transmit.
 ******************************************************************************/
void spi1_transmit(const uint8_t *data, uint32_t size)
{
    uint32_t i = 0U;
    volatile uint8_t dummy;

    while (i < size)
    {
        while ((SPI1->SR & SPI_SR_TXE_FLAG) == 0U)
        {
        }

        SPI1->DR = data[i];
        i++;
    }

    while ((SPI1->SR & SPI_SR_TXE_FLAG) == 0U)
    {
    }

    while ((SPI1->SR & SPI_SR_BSY_FLAG) != 0U)
    {
    }

    /*
     * Clear possible overrun condition by reading DR followed by SR.
     * This is required because received data is ignored during transmit-only use.
     */
    dummy = (uint8_t)SPI1->DR;
    dummy = (uint8_t)SPI1->SR;
    (void)dummy;
}

/******************************************************************************
 * @brief Receives data over SPI1.
 *
 * SPI is full-duplex internally. To generate clock pulses during reception,
 * dummy bytes are written to the data register.
 *
 * @param data Pointer to the receive buffer.
 * @param size Number of bytes to receive.
 ******************************************************************************/
void spi1_receive(uint8_t *data, uint32_t size)
{
    while (size > 0U)
    {
        SPI1->DR = 0x00U;

        while ((SPI1->SR & SPI_SR_RXNE_FLAG) == 0U)
        {
        }

        *data = (uint8_t)SPI1->DR;
        data++;
        size--;
    }
}





/*Draft

// NOTES*

// alternate funtion mapping -> Data sheet stm32f401re

// PA7 -> MOSI , AF
// PA5 -> SCK , AF



#include "stm32f401xe.h"

#define SPI1EN 			(1U<<12)
#define GPIOAEN			(1U<<0)
#define SPI_CPHA		(1U<<0)
#define SPI_CPOL		(1U<<1)
#define FUll_DuplexEN	(1U<<10)
#define LSBFirst		(1U<<7)
#define MSTR			(1U<<2)
#define DFF 			(1U<<11)
#define SSM 			(1U<<9)
#define SSI				(1U<<8)
#define SPIEN			(1U<<6)

#define SR_TXE			(1U<<1)
#define SR_BSY			(1U<<7)
#define SR_RXNE			(1U<<0)



void spi_gpio_init(void){

	// set PA5 and PA7 to alternate function Mode
	GPIOA->MODER &= ~(3U << (5*2));
	GPIOA->MODER |= (2U << (5*2));

	GPIOA->MODER &= ~(3U << (7*2));
	GPIOA->MODER |= (2U << (7*2));

	//set alternate function type to SPI1
	//Pins 0 to 7 -> alternate function LOW Register
	//Pins 8 to 15 -> alternate function HIGH Register


	//PA5 to AF5 , 20 - 23 ( 0101 )
	GPIOA->AFR[0] &= ~(15U << (5*4));
	GPIOA->AFR[0] |= (5U << (5*4));

	//PA7 to AF5 , 28 - 31 ( 0101 )
	GPIOA->AFR[0] &= ~(15U << (7*4));
	GPIOA->AFR[0] |= (5U << (7*4));

}

void spi1_config(void){
	RCC -> APB2ENR |= SPI1EN;

	//Set clock to fpclk/2
	//BR zu 010 -> 84MHz/8 , SCL max beim ST7735S 15MHz

	SPI1->CR1 &= ~(7U << 3);
	SPI1->CR1 |= (2U << 3);

	// CPOL und CPHA auf MODE0 -> 0,0 laut Datenblatt ST7735S
	SPI1 -> CR1 &= ~(SPI_CPOL);
	SPI1 -> CR1 &= ~(SPI_CPHA);

	// Enable Full duplex
	SPI1 -> CR1 &= ~(FUll_DuplexEN);

	// Enable MSB Fisrt, spezifisch für ST7735S
	SPI1 -> CR1 &= ~(LSBFirst);

	// Set Master Mode
	SPI1 -> CR1 |= (MSTR);

	// Set 8-Bit Data Mode
	SPI1 -> CR1 &= ~(DFF);

	// Enable Software slave management
	SPI1 -> CR1 |= (SSM);
	SPI1 -> CR1 |= (SSI);

	// Enable SPI periph
	SPI1 -> CR1 |= (SPIEN);

}


void spi1_transmit(uint8_t *data, uint32_t size){
	uint32_t i = 0;
	uint8_t temp;

	while(i<size)
	{
	  // Wait until TXE ist set -> SPI_SR
		while(!(SPI1 -> SR & (SR_TXE))){}

	  // Write the data to data Buffer
		SPI1 -> DR = data[i];
		i++;
	}

	// Wait until TX Buffer empty
	while(!(SPI1 -> SR & (SR_TXE))){}

	// Wait for BSY flag to reset
	while(SPI1 -> SR & SR_BSY){}

	// Clear the OVR flag -> Reference Manual(Error flags)
	temp = SPI1 -> DR;
	temp = SPI1 -> SR;

}


void spi1_receive(uint8_t *data, uint32_t size){

	while(size){
		SPI1 ->DR = 0;

		while(!(SPI1 -> SR & SR_RXNE)){}

		*data++ = (SPI1 -> DR);
		 size--;
	}
}

*/

