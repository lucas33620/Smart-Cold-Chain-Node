/**
 * @file    task_acq.c
 *
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.fr)
*/

#include "task_acq.h"
#include "telem.h"
#include "config.h"
#include "main.h"
#include "stm32f4xx_hal_gpio.h" // prototypage HAL_GPIO_ReadPin
#include "sensor_th.h"   /* bool SensorTH_Read(float* t_c, float* rh_pct) */
#include "adc_utils.h"   /* float Adc_ReadMCUTempC(void); float Adc_ReadVinV(void); */
#include <stdbool.h>

static TaskHandle_t s_task = NULL;
static QueueHandle_t s_q_telem = NULL;
static void task_acq_entry(void *arg);
static void acquire_once(telem_t *out);

#if defined(SIM_TARGET) && (SIM_TARGET != 0)
static struct {
    int     use_fixed;
    float   t_c, rh_pct, vin_v;
    uint8_t door;
} s_sim = {0};
#endif

/* API */
void TaskAcq_RequestImmediateSample(void)
{
    if (s_task) {
    	/* réveille la tâche sans attendre la période */
        (void)xTaskNotifyGive(s_task);
    }
}

void TaskAcq_Start(QueueHandle_t q_telem)
{
	/* On commence l'acquisition */
	s_q_telem = q_telem; // On remplis la queu de la donnée
    configASSERT(s_q_telem != NULL); // On assure qu'elle a bien été recu

    BaseType_t send_status = xTaskCreate(task_acq_entry, "acq",
                                384, NULL,
                                tskIDLE_PRIORITY + 2, &s_task); // stacks word 1536 octets

    configASSERT(send_status == pdPASS);
}

#if defined(SIM_TARGET) && (SIM_TARGET != 0)
void TaskAcq_SimSet(float t_c, float rh_pct, uint8_t door, float vin_v)
{
    s_sim.use_fixed = 1;
    s_sim.t_c   = t_c;
    s_sim.rh_pct= rh_pct;
    s_sim.door  = door;
    s_sim.vin_v = vin_v;
}
#endif

/* Mesure des capteurs + échantillonages */
static void acquire_once(telem_t* out)
{
    telem_t t = {0};

#if defined(SIM_TARGET) && (SIM_TARGET != 0)
    if (s_sim.use_fixed) {
        t.t_c    = s_sim.t_c;
        t.rh_pct = s_sim.rh_pct;
        t.vin_v  = s_sim.vin_v;
        t.door   = s_sim.door ? 1u : 0u;
    } else {
        /* profil simple en SIM ET si non fixé: rampe douce */
        static float x = 3.5f;
        static float h = 65.0f;
        x += 0.02f; if (x > 6.5f) x = 3.0f;
        h += 0.10f; if (h > 80.0f) h = 55.0f;
        t.t_c    = x;
        t.rh_pct = h;
        t.vin_v  = 24.0f;
        t.door   = 0u;
    }
    t.t_mcu_c = 35.0f;
    t.flags   = TELEM_FLAG_NONE;

#else
    /* Lecture des capteurs Temp/Hum via I2C */
    float th_c = 0.0f, rh = 0.0f;
    if (!SensorTH_Read(&th_c, &rh))
    {
    	t.flags |= TELEM_FLAG_SENSOR_FAULT;
    } else {
    	t.t_c    = th_c;
    	t.rh_pct = rh;
    }

    /* Lecture ADC */
	t.t_mcu_c = Adc_ReadMCUTempC(); // Température MCU
	t.vin_v   = Adc_ReadVinV(); 	// Tension Vin

	/* Lecture GPIO */
    t.door = (HAL_GPIO_ReadPin(DOOR_GPIO_Port, DOOR_Pin) == GPIO_PIN_SET) ? 1u : 0u; // Porte
#endif

    /* TIMER */
    t.ts_ms = (uint32_t)(xTaskGetTickCount() * TICK_MS);

    *out = t;
}

static void task_acq_entry(void* arg)
{
    (void)arg;
    const TickType_t period = pdMS_TO_TICKS(PERIOD_ACQ_MS);
    TickType_t last = xTaskGetTickCount()

    for (;;) {
        // attend soit notif immédiate, soit l’échéance périodique
        // 1) fenêtre de notif non bloquante
        if (ulTaskNotifyTake(pdTRUE, 0) == 0) {
        // 2) sinon délai périodique précis
            vTaskDelayUntil(&last, period);
        } else {
            // notif reçue -> pas de délai, acquisition immédiate
            last = xTaskGetTickCount(); // recale la base pour la prochaine période
        }

        telem_t sample;
        acquire_once(&sample);

        (void)xQueueSend(s_q_telem, &sample, 0); // ok si drop tolérable
        // Si queue taille 1 et on veut garder "le plus récent":
        // (void)xQueueOverwrite(s_q_telem, &sample);
    }
}
