/**
 * @file    fram_spi.c
 * @brief   Driver FRAM SPI (MB85RSxxx) pour persistance du logger.
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.eu)
 */

#include "fram_spi.h"
#include "main.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;

/* Configuration du CHIP */
#define FRAM_CS_GPIO_Port   GPIOB
#define FRAM_CS_Pin         GPIO_PIN_5
#define FRAM_CS_LOW()   HAL_GPIO_WritePin(FRAM_CS_GPIO_Port, FRAM_CS_Pin, GPIO_PIN_RESET) 	//Je te parle
#define FRAM_CS_HIGH()  HAL_GPIO_WritePin(FRAM_CS_GPIO_Port, FRAM_CS_Pin, GPIO_PIN_SET) 	//Je t'écoute plus

/* Initialisation du driver */
int Fram_Init(void)
{
	FRAM_CS_HIGH();
	if (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY) {
			if (HAL_SPI_Init(&hspi1) != HAL_OK)
				return -1;
		}
	return 0;
}

/* Lecture brute dans le FRAM */
int Fram_Read(uint16_t addr, void *buf, size_t len)
{
	data_received = HAL_SPI_Receive(hspi1, sample, sizeof(telem_t), 5); 	//

}

/* Ecrit un échantillon de télémétrie dans le FRAM */
int Fram_WriteSample(const telem_t *sample)
{
	HAL_StatusTypeDef HAL_SPI_Transmit(hspi1, sample, sizeof(telem_t), uint32_t Timeout);
}

/* Efface la zone de log */
int Fram_LogClear(void)
{

}
