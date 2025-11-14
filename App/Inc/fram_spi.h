/**
 * @file    fram_spi.h
 * @brief   Driver FRAM SPI pour stockage persistant (journal logger).
 *          Abstraction simple au-dessus du SPI1 + CS dédié.
 *          Conçu pour MB85RSxxx (FRAM série)
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.eu)
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include "main.h"       // pour hspi1
#include "config.h"     // pour éventuel pin CS
#include "telem.h"      // pour Fram_WriteSample(...)

#ifdef __cplusplus
extern "C" {
#endif

/* Opcodes FRAM MB85RS */
#define FRAM_OP_WREN        0x06u  // Active la fonction WRITE
#define FRAM_OP_WRDI        0x04u  // Désactive la fonction WRITE
#define FRAM_OP_RDSR        0x05u  // Lire le registre d'état 8 bits
#define FRAM_OP_WRSR        0x01u  // Ecrit le registre d’état
#define FRAM_OP_READ        0x03u  // Lecture de la mémoire
#define FRAM_OP_WRITE       0x02u  // Écriture sur la mémoire

/* Adresse de base réservée au logger */
#define FRAM_LOG_BASE_ADDR  0x0000u

int Fram_Init(void);
int Fram_Read(uint16_t addr, void *buf, size_t len);
int Fram_Write(uint16_t addr, const void *buf, size_t len);
int Fram_WriteSample(const telem_t *sample);
int Fram_LogClear(void);

#ifdef __cplusplus
}
#endif
