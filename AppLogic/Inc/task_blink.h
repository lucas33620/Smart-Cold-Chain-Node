/**
 * @file    task_blink.h
 * @brief   Tâche de signalisation (LED et buzzer).
 *          Indique l’état système via patterns visuels/sonores :
 *          - Lecture du groupe d’événements système (EVT_SYS_*)
 * 			- Pilotage de la LED d’état (GPIOG, PG13)
 * 			- Pilotage du buzzer (TIM4_CH1 PWM)
 * 			- Patterns :
 *      			- 1 Hz   = OK
 *      			- 2 Hz   = Alarme active
 *      			- 5 Hz   = Défaut capteur
 *      			- Fixe   = Mode configuration (future extension)
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.fr)
 */

#pragma once

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Démarre la tâche de signalisation (LED + buzzer).
 * @param evt_sys  Groupe d’événements système (EVT_SYS_*).
 */
void TaskBlink_Start(EventGroupHandle_t evt_sys);

/**
 * @brief Force un pattern LED spécifique (par ex. en configuration).
 * @param period_ms  Période du clignotement forcé (0 = fixe).
 */
void TaskBlink_ForcePattern(uint32_t period_ms);

#ifdef __cplusplus
}
#endif
