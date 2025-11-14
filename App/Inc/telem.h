/**
 * @file    telem.h
 * @brief   Types communs de télémétrie et événements SCN.
 *          Partagé entre task_acq, task_proc, logger, fram_spi, etc.
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.eu)
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Flags de télémétrie */
typedef enum {
    TELEM_FLAG_NONE         = 0u,
    TELEM_FLAG_ALARM_HI     = (1u << 0),
    TELEM_FLAG_ALARM_LO     = (1u << 1),
    TELEM_FLAG_DOOR_OPEN    = (1u << 2),
    TELEM_FLAG_SENSOR_FAULT = (1u << 3),
    TELEM_FLAG_VIN_LOW      = (1u << 4),
} telem_flags_t;

/* Échantillon de télémétrie (mesure complète) */
typedef struct {
    float    t_c;       /* °C air */
    float    t_mcu_c;   /* °C MCU */
    float    rh_pct;    /* % HR */
    float    vin_v;     /* Tension d’alim en V */
    uint8_t  door;      /* 0 = fermé / 1 = ouvert */
    telem_flags_t flags;/* bits d’état / alarme */
    uint32_t ts_ms;     /* timestamp (tick FreeRTOS * TICK_MS) */
} telem_t;

/* Type d’événement générique (pour queues d’événements) */
typedef uint32_t event_t;

#ifdef __cplusplus
}
#endif
