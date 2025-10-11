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
#include "telem.h"

#ifdef __cplusplus
extern "C" {
#endif

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
