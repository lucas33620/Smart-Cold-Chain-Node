/**
 * @file    task_proc.c
 * @brief   Traitement télémétrie: seuils, hystérésis, dwell, états système,
 *          journalisation RAM et commit FRAM sur demande.
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.fr)
 */

#include <string.h>
#include "task_proc.h"
#include "logger.h"		// pUsh & FLush
#include "relay.h"      // void Relay_SetState(uint8_t on);
#include "core_init.h"		// EventGroup
#include "main.h"

/* Définition des objets */
static TaskHandle_t        s_task      = NULL;
static QueueHandle_t       s_q_telem   = NULL;  /* in  (telem_t) */
static QueueHandle_t       s_q_events  = NULL;  /* out (event_t / flags) */
static EventGroupHandle_t  s_evt_sys   = NULL;

static telem_t             s_last      = {0};
static uint8_t             s_alarm_on  = 0u;
static uint8_t             s_relay_on  = 0u;

static uint32_t s_outside_since_ms = 0;  		// Maintenir le déclenchement de l'alarme si T hors plage > ALARM_DWELL_MS

#define PROC_NOTIFY_REEVAL (1UL)  		// Notification pour ForceReeval (mesure immédiate)

/* Return 1 si T ou of range */
static inline uint8_t temp_out_of_range(float t)
{
	return (t > TEMP_HIGH_C) || (t < TEMP_LOW_C);
}

/* Appliquer l'hysteresis pour l'action ou la désactivation de l'alarme */
static inline uint8_t temp_back_in_range_hyst(float t)
{
    const float hi_clear = TEMP_HIGH_C - TEMP_HYST_C;
    const float lo_clear = TEMP_LOW_C  + TEMP_HYST_C;
    return (t <= hi_clear) && (t >= lo_clear);
}

/* Contrôle venitlisation si T haut avec hysterésis */
static uint8_t relay_shoud_be_on(float t)
{
	static uint8_t latched = 0u;
	if (!latched && (t > TEMP_HIGH_C + TEMP_HYST_C))
		lathed = 1u;
    else if (latched && (t < (TEMP_HIGH_C - TEMP_HYST_C)))
    	latched = 0u;
}

/* MAJ bits EventGroup system */
static void update_sys_bits(const telem_t* s)
{
	/* 🔹 set_bits → active un flag (met le bit à 1)
	*  🔹 clr_bits → désactive un flag (met le bit à 0
	*/
	EventBits_t set_bits = 0, clr_bits = 0;

	/* MAJ porte */
	if (s->door)
		set_bits |= EVT_SYS_DOOR_OPEN;
	else
		clr_bits |= EVT_SYS_DOOR_OPEN;

	/* MAJ défaut capteur */
	if (s->flags & TELEM_FLAG_SENSOR_FAULT)
		set_bits |= EVT_SYS_SENSOR_FAULT;
	else
		set_bits |= EVT_SYS_SENSOR_FAULT;

	/* MAJ alarme active */
	if (s->alarme)
		set_bits |= EVT_SYS_ALARM_ACTIVE;
	else
		clr_bits |= EVT_SYS_ALARM_ACTIVE;

	// Liste complète == on applique la MAJ
	if (set_bits) xEventGroupSetBits(s_evt_sys, set_bits);
	if (clr_bits) xEventGroupClearBits(s_evt_sys, clr_bits);
}

/* Envoie optionnel d'un événement synthétique vers q_events (mask flags) */
static void emit_event(uint32_t mask)
{
    if (s_q_events) {
        (void)xQueueSend(s_q_events, &mask, 0);
    }
}

/* Corps de tâche */
static void task_proc_entry(void* arg)
{
	/* Etape du processus :
	 * 1) Attente d'un nouvel échantillon OU d'une notification "force reeval"
	 * 2) Traitement de l'échantillon
	 * 3) Actions physiques : relais (hystérésis propre)
	 * 4) Bits système (LED/buzzer via task_blink, diffusion via task_can)
	 * 5) Journalisation RAM; commit FRAM sur demande
	 * 6) Optionnel : notifier un évènement condensé aux autres tâches (CAN/CLI)
	 */

	/* Etape 1 */

	/* Etpae 2 */
	/* Etape 3 */
	/* Etpae 4 */
	/* Etape 5 */
	/* Etape 6 */
}















