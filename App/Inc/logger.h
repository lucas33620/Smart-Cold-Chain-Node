/**
 * @file    logger.h
 * @brief   Gestion d’un ring buffer RAM et commit périodique vers la FRAM SPI.
 *          Le logger agit comme un journal interne pour les échantillons de télémétrie.
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.fr)
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "telem.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Initialise le logger et son buffer interne.
 * @return 0 si succès, -1 sinon.
 */
int Logger_Init(void);

/**
 * @brief Ajoute un échantillon dans le ring buffer RAM.
 * @param sample  Pointeur vers la structure `telem_t`.
 * @return 0 si succès, -1 si buffer plein.
 */
int Logger_Push(const telem_t* sample);

/**
 * @brief Vide le contenu du buffer RAM vers la FRAM SPI.
 *        Utilisé sur `EVT_SYS_COMMIT_REQ` ou lors d’un arrêt.
 */
void Logger_Flush(void);

/**
 * @brief Efface complètement le buffer RAM.
 */
void Logger_Clear(void);

/**
 * @brief Retourne le nombre approximatif d’échantillons stockés.
 * @return Compte d’entrées dans le ring.
 */
size_t Logger_GetCount(void);

#ifdef __cplusplus
}
#endif
