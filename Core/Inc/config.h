#pragma once
/* ===== Produit : Smart Cold-Chain Node (SCN) ===== */

#include "main.h"   // pour les handles générés (I2C, TIM, etc.)

/* Horloge RTOS (ms/tick) */
#define TICK_MS                      1

/* Périodes (ms) */
#define PERIOD_ACQ_MS                1000   /* acquisition capteurs : 1 Hz   */
#define PERIOD_CAN_MS                100    /* télémétrie CAN : 10 Hz        */
#define PERIOD_BLINK_OK_MS           1000   /* LED état OK : 1 Hz            */
#define PERIOD_BLINK_ALARM_MS        500    /* LED état alarme : 2 Hz        */
#define COMMIT_MS                    10000  /* flush logger -> FRAM          */

/* Seuils température (°C) */
#define TEMP_HIGH_C                  4.0f   /* cible chaîne du froid d'après le site www.techni-froid.fr */
#define TEMP_LOW_C                   0.0f
#define TEMP_HYST_C                  0.5f
#define ALARM_DWELL_MS               5000   /* T hors plage >5s => alarme */

/* Réseau / IO */
#define NODE_ID                      0x12
#define CAN_BAUD                     250000
#define UART_BAUD                    115200

/* ADC (Vin sur PA1 + Temp MCU) */
#define VIN_ADC_CH                   ADC_CHANNEL_1   /* PA1 */
#define VIN_DIV_RTOP_OHM             100000.0f       /* exemple: 100k/10k */
#define VIN_DIV_RBOT_OHM             10000.0f
#define VIN_VREF_mV                  3300.0f
#define VIN_ADC_MAX                  4095.0f
/* Helper: Vin[V] = (adc/4095)*Vref * (Rtop+Rbot)/Rbot */

/* Capteurs I2C */
#define I2C_BUS                      hi2c1           /* handle CubeMX */
#define SHT31_I2C_ADDR               (0x44 << 1)     /* alternatif: 0x45 */
#define HDC1080_I2C_ADDR             (0x40 << 1)

/* GPIO / PWM / IO */
#define LED_GPIO_Port                GPIOG
#define LED_Pin                      GPIO_PIN_13

#define DOOR_GPIO_Port               GPIOA
#define DOOR_Pin                     GPIO_PIN_0

#define RELAY_GPIO_Port              GPIOD
#define RELAY_Pin                    GPIO_PIN_2

#define BUZZER_TIM                   htim4
#define BUZZER_TIM_CHANNEL           TIM_CHANNEL_1   /* PD12 -> TIM4_CH1 */

/* Journalisation */
#define LOGGER_RING_CAPACITY         512             /* entrées en RAM */
#define LOGGER_CRC8_POLY             0x31            /* x^8+x^5+x^4+1 */

/* Build target */
#ifndef SIM_TARGET
  #define SIM_TARGET 0
#endif
