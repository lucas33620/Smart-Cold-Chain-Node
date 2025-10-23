/**
 * @file    task_blink.c
 * @brief   Gestion de la LED d’état et du buzzer selon les flags système.
 *          Lecture périodique de l’EventGroup partagé pour afficher
 *          les statuts RUN / ALARM / FAULT / CONFIG.
 * @copyright
 *   © 2025 SYLORIA — MIT License
 *   Auteur : BAQUEY Lucas (contact@syloria.fr)
 */

#include "task_blink.h"
#include "core_init.h"
#include "main.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"


/* Objets internes */
static TaskHandle_t        s_task    = NULL;
static EventGroupHandle_t  s_evt_sys = NULL;

/* Pattern forcé */
static uint32_t            s_forced_period = 0;
static uint8_t             s_force_active  = 0;


static inline void led_off(void)
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

static inline void led_on(void)
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

static inline void buzzer_on(void)
{
	HAL_TIM_PWM_Start(&BUZZER_TIM, BUZZER_TIM_CHANNEL);
}

static inline void buzzer_off(void)
{
	HAL_TIM_PWM_Stop(&BUZZER_TIM, BUZZER_TIM_CHANNEL);
}

/**
 * @brief Retourne la période de clignotement en fonction des bits actifs.
 */
static uint32_t get_period_from_flags(EventBits_t bits)
{
	if (bits & EVT_SYS_SENSOR_FAULT)
	        return 100;   /* 5 Hz clignotement rapide */
	    if (bits & EVT_SYS_ALARM_ACTIVE)
	        return 500;   /* 2 Hz */
	    if (bits & EVT_SYS_DOOR_OPEN)
	        return 1000;  /* 1 Hz mais buzzer actif court */
	    return 1000;       /* OK : 1 Hz */
}

/**
 * @brief Retourne 1 si buzzer doit être activé
 */
static uint8_t buzzer_should_be_on(EventBits_t bits)
{
    if (bits & EVT_SYS_ALARM_ACTIVE)
        return 1u;  /* buzzer actif si alarme */
    return 0u;
}


/**
 * @brief Tâche principale : clignotement LED et gestion buzzer.
 */

static void task_blink_entry(void *arg)
{
    (void)arg;
    TickType_t last = xTaskGetTickCount();

    for (;;)
    {
        EventBits_t bits = xEventGroupGetBits(s_evt_sys);
        uint32_t period = s_force_active ? s_forced_period : get_period_from_flags(bits);
        uint8_t buzz_on = buzzer_should_be_on(bits);

        led_on();
        if (buzz_on) buzzer_on();
        vTaskDelayUntil(&last, pdMS_TO_TICKS(period / 2));

        led_off();
        if (buzz_on) buzzer_off();
        vTaskDelayUntil(&last, pdMS_TO_TICKS(period));

        last = xTaskGetTickCount();
    }
}

/* API */
void TaskBlink_Start(EventGroupHandle_t evt_sys)
{
    s_evt_sys = evt_sys;
    configASSERT(s_evt_sys);

    BaseType_t ok = xTaskCreate(task_blink_entry, "blink",
                                256, NULL,
                                tskIDLE_PRIORITY + 1, &s_task);
    configASSERT(ok == pdPASS);
}

void TaskBlink_ForcePattern(uint32_t period_ms)
{
    s_force_active  = (period_ms > 0);
    s_forced_period = period_ms;
}
