# 1) Contexte & cas d’usage (réel)

**Projet : Smart Cold-Chain Node (SCN)** — un nœud de surveillance de chaîne du froid pour chambres froides / vitrines réfrigérées :

- **Mesures** : température & humidité d’air, contact de porte, tension d’alim 24 V → télémétrie locale et alerte.  
- **Actions** : pilotage d’une alarme locale (buzzer/LED) et d’un relais de ventilation de secours (faible charge).  
- **Communication** : **CAN (Classique)** vers un concentrateur (gateway) en backroom pour collecte/alertes, **UART** (console) pour maintenance.  
- **Journalisation** : ring buffer en RAM + stockage périodique en **SPI FRAM** (robuste en écriture) pour ne pas perdre de données lors de coupures.  
- **Sécurité & sûreté** : watchdog, brown-out, RTOS health monitor, checksum journaux.  
- **Énergie** : supervision tension d’entrée et température interne MCU.

> **Intérêt “indus”** : capteurs I²C/SPI, bus CAN, contraintes temps réel souples (mesures périodiques + gestion d’alarmes), persistance FRAM, stratégie de dégradé en cas de défaut — typique d’IoT industriel.

---

# 2) Cibles matérielles & logicielles

## 2.1 Matériel (HW)

- **MCU** : **STM32F429ZI** ou **STM32F407VG** (famille F4 : FPU, CAN, timers riches).  
  - Carte de dev recommandée : **NUCLEO-F429ZI** (ou NUCLEO-F407ZG).
- **Capteurs** :
  - **Temp/Hum** : SHT31 (I²C) ou HDC1080 (I²C).
  - **Temp MCU** : capteur interne + compensation.
  - **Contact porte** : entrée **GPIO** avec pull-up.
- **Stockage non volatile** : **FRAM SPI** (ex. MB85RS256B 32 KB ou 256 KB).  
- **Coms** :
  - **CAN** : transceiver TJA1051/TXN.  
  - **UART** : via ST-Link VCP (console).  
- **Sorties** : relais/SSR, buzzer (GPIO/PWM), LED d’état.  
- **Alim.** : entrée 24 V → step-down 5 V/3.3 V, mesure tension via pont + ADC.

---

# 3) Exigences fonctionnelles

## 3.1 Mesures & périodicité

- **Acquisition** toutes **1 s** :
  - Température/Humidité via I²C.  
  - Temp MCU (ADC interne).  
  - Tension d’alim (ADC).  
  - État contact porte (GPIO).  
- **Filtrage/Validation** : rejet outliers, hystérésis alarme, compensation calibrable.  

## 3.2 Journalisation

- **Buffer circulaire RAM** (512 entrées) + **commit périodique** (10 s) vers FRAM SPI.  
- **Entrée journal** = timestamp, T/HR, Tmcu, Vin, porte, flags, CRC-8.  
- **Rétention** ≥ 7 jours @ 1 Hz.  
- **Export** via UART CLI ou trame CAN spéciale.

## 3.3 Communication

- **CAN @ 250 kbps** — protocole léger propriétaire (télémétrie, alarmes, config).  
- **UART CLI (115200 8N1)** — commandes `status`, `get th`, `set thigh=..`, `log dump`, `reboot`, etc.

## 3.4 Actions & états

- **Alarmes** : LED rouge + buzzer si T hors plage > 5 s.  
- **Relais ventilation** : ON si T > seuil haut + hystérésis.  
- **LED état système** :  
  - 1 Hz : OK  
  - 2 Hz : alarme active  
  - rapide : mode config  
- **Modes** : RUN / DEGRADED / SAFE.  
- **Config persistante** : seuils, périodes, NodeID → FRAM.

---

# 4) Exigences non fonctionnelles

- **Temps réel** : gigue acquisition < 10 ms @ 1 Hz.  
- **Robustesse** : watchdog, brown-out, recovery, CRC.  
- **Portabilité** : HAL isolée, drivers mock pour SIM.  
- **Testabilité** : unit tests (logique), hooks SIM.  
- **Traçabilité** : Event Recorder / UART logs.  
- **Style** : clang-format, doxygen.  
- **Sécurité** : pas d’allocation dynamique, limites vérifiées.

---

# 5) Architecture 
## 5.1 Logicielle

- /app
- /core (application, state machine, config, alarms)
- /rtos (wrappers FreeRTOS: tasks, queues, timers, events)
- /drivers (hal_adc, hal_i2c, hal_spi, hal_can, hal_gpio, fram, sensor_th)
- /platform
- /hw (STM32 HAL, BSP Nucleo)
- /sim (stubs, data feeders, pseudo-can, virtual fram)
- /middleware (cli uart, can_proto, logger, ringbuf, crc)
- /tests (unit tests - Ceedling)

## 5.2 Configuration STM32

- **GPIO**
  - PG13 : LED
  - PA0  : Porte (input)
  - PD2  : Ventilation (output - relay)

- **PWM**
  - PD12 → TIM4_CH1 (Buzzer)

- **USART3**
  - PD8 / PD9

- **I²C1**
  - PB6 / PB7

- **SPI1**
  - PA5 / PA6 / PA7 + PB5 (CS)

- **CAN1**
  - PD0 / PD1

- **ADC1**
  - PA1 (Vin)
  - Channel 16 (Température MCU)

- **Horloge**
  - HSE = 8 MHz → PLL = 168 MHz
  - **SYSCLK = 168 MHz**
  - **HCLK = 168 MHz**
  - **PCLK1 = 42 MHz**
  - **PCLK2 = 84 MHz**
  - **Timers**
    - APB1 Timers = 84 MHz
    - APB2 Timers = 168 MHz

- **FreeRTOS activé**

# 6) Tests & validation

## 6.1 Unit tests (host)

- **Core** : états, hystérésis, CRC.  
- **Logger** : ring buffer, sérialisation, parse.  
- **Config** : set/get + mock FRAM.  
- **CAN proto** : pack/unpack trames.

## 6.2 Intégration (HW)

- **I²C** : lecture capteur, gestion erreurs.  
- **SPI FRAM** : cycles R/W, endurance.  
- **CAN** : trames périodiques, RX cmd.  
- **UART CLI** : scénario opérateur.  
- **Watchdog/BOR** : recovery tests.

## 6.3 Critères d’acceptation

- Jitter acquisition < 10 ms @ 1 Hz.  
- 0 % perte trames CAN @ 10 Hz.  
- Journal FRAM valide (10 cycles).  
- Scénarios SIM validés 1→5.

---

# 7) Livrables

1. **Code source** (MIT) + README pro (schémas, GIF SIM).  
2. **Deux projets** : Keil µVision (`SIM`, `NUCLEO-F429ZI`) + STM32CubeIDE.  
3. **Doc Doxygen** + User Guide (CLI, frames CAN).  
4. **Plan de tests** + rapports unit/integration.  
5. **Schéma bloc** + brochage Nucleo↔capteurs.  
6. **Captures Event Recorder** + trames CAN (run type).  
7. **Release v1.0** (tag Git, changelog, artefacts .hex/.elf).

---

# 8) Annexes utiles

- **Protocole CAN (TLV)** :  
  `0x01 TEMP`, `0x02 HUM`, `0x03 TMCU`, `0x04 VIN`, `0x05 DOOR`, `0x06 FLAGS`, `0x10 THIGH`, `0x11 TLOW`, `0x12 HYST`, etc.
- **CLI exemples** :  
  `get telem`, `get cfg`, `set thigh 7.0`, `log dump 100`, `can id 0x34`, `reboot`
- **Erreurs normalisées** :  
  `ERR_I2C_TIMEOUT`, `ERR_SPI_WRITE`, `ERR_CAN_TX_OVR`, `ERR_FRAM_CRC`, etc.
- **Patterns** :  
  Ring buffer lock-free, state machine alarme, driver abstraction, adapter CLI↔config, CRC module.

