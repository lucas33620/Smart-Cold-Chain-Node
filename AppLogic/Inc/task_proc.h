/**
 * @file    task_proc.h
 * @brief   Tâche de traitement temps réel exécutée après chaque acquisition (task_acq).
 *          Elle récupère les échantillons de télémétrie (queue), applique les règles métier
 *          et met à jour les états/flags systèmes.
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.fr)
 */

#pragma once

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
#include "config.h"
#include "telem.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Démarre la tâche de traitement.
 * @param q_telem   Queue d’entrée contenant les échantillons `telem_t`.
 * @param q_events  Queue de sortie (événements système pour CAN/CLI).
 * @param evt_sys   Groupe d’événements système partagé (flags globaux).
 */
void TaskProc_Start(QueueHandle_t q_telem,
                    QueueHandle_t q_events,
                    EventGroupHandle_t evt_sys);

/* Force le recalcul immédiat des seuils (utilie en config dynamique) */
void TaskProc_ForceReeval(void);

/* Accès lecture au derniers échantillon */
telem_t TaskProc_GetLast(void); //Assurer une version à jour

#ifdef __cplusplus
}
#endif

