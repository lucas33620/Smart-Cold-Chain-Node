/**
 * @file    core_init.h
 * @brief   Initialisation des objets FreeRTOS (queues, timers, tâches)
 *          pour l'application Smart Cold-Chain Node (SCN).
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.fr)
 */

#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Bits du groupe d'événements système */
#define EVT_SYS_ALARM_ACTIVE     (1U << 0)
#define EVT_SYS_SENSOR_FAULT     (1U << 1)
#define EVT_SYS_DOOR_OPEN        (1U << 2)
#define EVT_SYS_COMMIT_REQ      (1U << 3)


/* API de lifecycle */
void Core_Init(void);   /* crée queues/timers/tasks */
void Core_Start(void);  /* arme les timers si besoin */

/* Getters d’objets FreeRTOS (static + getters pour tests unitaires riches)*/
QueueHandle_t     Core_GetTelemQueue(void);		// File télémétrie brute (task_acq) -> traitement (task_proc)
QueueHandle_t     Core_GetEventsQueue(void);	// File d'événements/flags (task_proc ) -> diffusion (task_can/cli)
TimerHandle_t     Core_GetCommitTimer(void);	// Timer logiciel de commit périodique (logger RAM -> FRAM)
EventGroupHandle_t Core_GetSysEvents(void);		// Groupe événements système (états/alertes)

#ifdef __cplusplus
}
#endif
