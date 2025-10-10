/**
 * @file    core_init.c
 * @brief   Création des queues, timers, event groups et démarrage des tâches.
 *          Expose uniquement des getters (objets statiques internes).
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.fr)
*/

#include "core_init.h"
#include "task_blink.h"
#include "task_acq.h"
#include "task_proc.h"
#include "task_can.h"
#include "task_cli.h"

typedef struct {
    float    t_c;       // température air
    float    rh_pct;    // humidité %
    float    t_mcu_c;   // temp MCU
    float    vin_v;     // tension entrée
    uint8_t  door;      // 0/1
    uint32_t flags;     // bits divers (ex: out-of-range, capteur HS...)
} telem_t;

typedef uint32_t event_t;  // bitmask d'événements système

/* ---------- Objets FreeRTOS (scope fichier) ---------- */
static QueueHandle_t      s_qTelem  = NULL;  /* task_acq -> task_proc */
static QueueHandle_t      s_qEvents = NULL;  /* task_proc -> can/cli  */
static TimerHandle_t      s_tCommit = NULL;  /* commit logger -> FRAM */
static EventGroupHandle_t s_evtSys  = NULL;  /* états/alertes système */

/* Callbacks
 * Wiki : déclencher périodiquement le “flush” du logger (vider le ring RAM vers la FRAM) toutes COMMIT_MS
*/
static void commit_cb(TimerHandle_t xTimer)
{
	(void)xTimer;
	/* Déclenche un “flush” asynchrone du logger via EventGroup. */
	xEventGroupSetBits(s_evtSys, EVT_SYS_COMMIT_REQ);
}

/* API */
void Core_Init(void)
{
	/* Création des queues */
	s_qTelem  = xQueueCreate(16, sizeof(telem_t));
	s_qEvents = xQueueCreate(16, sizeof(event_t));
    configASSERT(s_qTelem && s_qEvents); // A modifier en Prod car on ne sait pas laquelle est Null

    /* Event group */
       s_evtSys = xEventGroupCreate();
       configASSERT(s_evtSys); // A modifier en Prod

       /* Timer logiciel de commit périodique */
       s_tCommit = xTimerCreate("commit",
                                pdMS_TO_TICKS(COMMIT_MS),
                                pdTRUE, /* auto-reload */
                                NULL,
                                commit_cb);
       configASSERT(s_tCommit);

       /* Démarrage des tâches applicatives */
       TaskBlink_Start();
       TaskAcq_Start(s_qTelem);
       TaskProc_Start(s_qTelem, s_qEvents, s_evtSys);
       TaskCan_Start(s_qEvents, s_evtSys);
       TaskCli_Start();
}

void Core_Start(void)
{
   /* Armer les timers une fois tout créé */
   xTimerStart(s_tCommit, 0);
}

/* ---------- Getters ---------- */
QueueHandle_t Core_GetTelemQueue(void)      { return s_qTelem;  }
QueueHandle_t Core_GetEventsQueue(void)     { return s_qEvents; }
TimerHandle_t Core_GetCommitTimer(void)     { return s_tCommit; }
EventGroupHandle_t Core_GetSysEvents(void)  { return s_evtSys;  }
