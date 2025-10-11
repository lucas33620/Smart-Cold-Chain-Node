/**
 * @file    telem.h
 * @brief   Types communs de télémétrie et événements SCN.
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.fr)
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

/* Échantillon de télémétrie (mis en queue par task_acq) */
typedef struct {
    float    t_c;       /* °C air */
    float    t_mcu_c;   /* °C MCU */
    float    rh_pct;    /* % HR */
    float    vin_v;     /* V in */
    uint8_t  door;      /* 0/1 */
    uint8_t  _rsv;      /* alignement simple si besoin */
    uint16_t _rsv2;
    telem_flags_t flags;/* flags courants (alarme, défauts) */
    uint32_t ts_ms;     /* uptime ms (xTaskGetTickCount()*TICK_MS) */
} telem_t;

/* Type d’événement générique (pour queues d’événements) */
typedef uint32_t event_t;

#ifdef __cplusplus
}
#endif
