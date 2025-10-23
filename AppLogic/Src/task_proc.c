/**
 * @file    task_proc.c
 * @brief   Traitement télémétrie : seuils, hystérésis, dwell, états système,
 *          journalisation RAM et commit FRAM sur demande.
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.fr)
 */

#include <string.h>
#include "task_proc.h"
#include "logger.h"
#include "relay.h"
#include "core_init.h"
#include "main.h"

/* ---------- Objets ---------- */
static TaskHandle_t        s_task      = NULL;
static QueueHandle_t       s_q_telem   = NULL;  /* in  (telem_t) */
static QueueHandle_t       s_q_events  = NULL;  /* out (event_t)  */
static EventGroupHandle_t  s_evt_sys   = NULL;

static telem_t             s_last      = {0};
static uint8_t             s_alarm_on  = 0u;
static uint8_t             s_relay_on  = 0u;
static uint32_t            s_outside_since_ms = 0;

/* Notification bits */
#define PROC_NOTIFY_REEVAL   (1UL)

/* --- Helpers internes --- */
static inline uint8_t temp_out_of_range(float t)
{
    return (t > TEMP_HIGH_C) || (t < TEMP_LOW_C);
}

static inline uint8_t temp_back_in_range_hyst(float t)
{
    const float hi_clear = TEMP_HIGH_C - TEMP_HYST_C;
    const float lo_clear = TEMP_LOW_C  + TEMP_HYST_C;
    return (t <= hi_clear) && (t >= lo_clear);
}

static uint8_t relay_should_be_on(float t)
{
    static uint8_t latched = 0u;

    if (!latched && (t > TEMP_HIGH_C + TEMP_HYST_C))
        latched = 1u;
    else if (latched && (t < (TEMP_HIGH_C - TEMP_HYST_C)))
        latched = 0u;

    return latched;
}

static void update_sys_bits(const telem_t* s)
{
    EventBits_t set_bits = 0, clr_bits = 0;

    /* Porte */
    if (s->door)
        set_bits |= EVT_SYS_DOOR_OPEN;
    else
        clr_bits |= EVT_SYS_DOOR_OPEN;

    /* Capteur */
    if (s->flags & TELEM_FLAG_SENSOR_FAULT)
        set_bits |= EVT_SYS_SENSOR_FAULT;
    else
        clr_bits |= EVT_SYS_SENSOR_FAULT;

    /* Alarme */
    if (s_alarm_on)
        set_bits |= EVT_SYS_ALARM_ACTIVE;
    else
        clr_bits |= EVT_SYS_ALARM_ACTIVE;

    /* Application */
    if (set_bits) xEventGroupSetBits(s_evt_sys, set_bits);
    if (clr_bits) xEventGroupClearBits(s_evt_sys, clr_bits);
}

static void emit_event(uint32_t mask)
{
    if (s_q_events)
        (void)xQueueSend(s_q_events, &mask, 0);
}

/* ---------- Tâche principale ---------- */
static void task_proc_entry(void* arg)
{
    (void)arg;
    telem_t sample = {0};

    for (;;)
    {
        /* 1) Attente d’un échantillon ou notification */
        if (xQueueReceive(s_q_telem, &sample, pdMS_TO_TICKS(50)) == pdFALSE)
        {
            if (ulTaskNotifyTake(pdTRUE, 0) & PROC_NOTIFY_REEVAL)
            {
                // rien à recalculer encore
            }
            continue;
        }

        /* 2) Copie du dernier échantillon */
        s_last = sample;

        /* 3) Détection hors plage avec dwell */
        if (temp_out_of_range(sample.t_c))
        {
            if (s_outside_since_ms == 0)
                s_outside_since_ms = sample.ts_ms;

            uint32_t dt = sample.ts_ms - s_outside_since_ms;
            if (dt >= ALARM_DWELL_MS)
                s_alarm_on = 1u;
        }
        else
        {
            s_outside_since_ms = 0;
            if (s_alarm_on && temp_back_in_range_hyst(sample.t_c))
                s_alarm_on = 0u;
        }

        /* 4) Mise à jour des flags de télémétrie */
        telem_flags_t f = sample.flags;
        if (s_alarm_on)
        {
            if (sample.t_c > TEMP_HIGH_C) f |= TELEM_FLAG_ALARM_HI;
            if (sample.t_c < TEMP_LOW_C)  f |= TELEM_FLAG_ALARM_LO;
        }
        s_last.flags = f;

        /* 5) Commande du relais */
        uint8_t want_state = relay_should_be_on(sample.t_c);
        if (want_state != s_relay_on)
        {
            s_relay_on = want_state;
            Relay_SetState(s_relay_on);
        }

        /* 6) MAJ EventGroup */
        update_sys_bits(&s_last);

        /* 7) Journalisation */
        Logger_Push(&s_last);

        /* 8) Commit FRAM sur requête */
        EventBits_t bits = xEventGroupWaitBits(s_evt_sys,
                                               EVT_SYS_COMMIT_REQ,
                                               pdTRUE, pdFALSE, 0);
        if (bits & EVT_SYS_COMMIT_REQ)
            Logger_Flush();

        /* 9) Diffusion d’un événement synthétique (CAN/CLI) */
        emit_event((uint32_t)s_last.flags);
    }
}

/* ---------- API ---------- */
void TaskProc_Start(QueueHandle_t q_telem,
                    QueueHandle_t q_events,
                    EventGroupHandle_t evt_sys)
{
    s_q_telem  = q_telem;
    s_q_events = q_events;
    s_evt_sys  = evt_sys;

    configASSERT(s_q_telem);
    configASSERT(s_evt_sys);

    BaseType_t ok = xTaskCreate(task_proc_entry, "proc",
                                512, NULL,
                                tskIDLE_PRIORITY + 2, &s_task);
    configASSERT(ok == pdPASS);
}

void TaskProc_ForceReeval(void)
{
    if (s_task)
        (void)xTaskNotify(s_task, PROC_NOTIFY_REEVAL, eSetBits);
}

telem_t TaskProc_GetLast(void)
{
    return s_last;
}
