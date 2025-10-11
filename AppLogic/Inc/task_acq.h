/**
 * @file    task_acq.h
 * @brief   Tâche d'acquisition capteurs (T/H I²C, T_mcu, Vin, porte).
 *          Pousse des échantillons dans la file de télémétrie pour task_proc.
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.fr)
 */

#pragma once

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Flags de télémétrie (bitmask (TRUE/FALSE) */
typedef enum {
    TELEM_FLAG_NONE        = 0u,
    TELEM_FLAG_ALARM_HI    = (1u << 0),  // Température > seuil haut
    TELEM_FLAG_ALARM_LO    = (1u << 1),  // Température < seuil bas
    TELEM_FLAG_DOOR_OPEN   = (1u << 2),  // Porte ouverte
    TELEM_FLAG_SENSOR_FAULT= (1u << 3),  // Capteur T/H en défaut
    TELEM_FLAG_VIN_LOW     = (1u << 4),  // Tension d'alim basse
} telem_flags_t;

/* Définitions des échantillons de télémétrie */
typedef struct{
	float t_c;			// Température en °C
    float t_mcu_c;    	// Température interne MCU (°C)
	float rh_pct;		// Humidité relatif en %
	float vin_v;		// Tension alimentation en volt
	uint8_t door;		// O/F de la porte
    uint8_t  _rsv0;		//Alignement / futur usage
    uint8_t  _rsv1;		//Alignement / futur usage
    uint16_t _pad;		// Aligner la structure de la mémoire %4
    uint32_t flags;     // telem_flags_t (bitmask)
    uint32_t ts_ms;     //timestamp (uptime en ms)

}telem_t;

/* Hooks force un échantillon directement sans attendre les 1Hz */
void TaskAcq_RequestImmediateSample(void);

/* API */
/**
 * @brief Démarre la tâche d'acquisition (période = PERIOD_ACQ_MS).
 * @param q_telem  Queue de sortie où seront envoyés les telem_t.
*/
void TaskAcq_Start(QueueHandle_t q_telem);

/* Interface de simulation (config.h/SIM_TARGET == TRUE) */
/**
 * @brief Définit un profil simple (valeurs imposées) côté simulation.
 *        Si non appelé, un profil par défaut (rampe/dérive douce) est utilisé.
*/
#if defined(SIM_TARGET) && (SIM_TARGET != 0)

void TaskAcq_SimSet(float t_c, float rh_pct, uint8_t door, float vin_v);
#endif

#ifdef __cplusplus
}
#endif
