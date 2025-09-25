# 1) Contexte & cas d’usage (réel)

**Projet : Smart Cold-Chain Node (SCN)** — un nœud de surveillance de chaîne du froid pour chambres froides / vitrines réfrigérées :

- **Mesures** : température & humidité d’air, contact de porte, tension d’alim 24 V → télémétrie locale et alerte.
- **Actions** : pilotage d’une alarme locale (buzzer/LED) et d’un relais de ventilation de secours (faible charge).
- **Communication** : **CAN (Classique)** vers un concentrateur (gateway) en backroom pour collecte/alertes, **UART** (console) pour maintenance.
- **Journalisation** : ring buffer en RAM + stockage périodique en **SPI FRAM** (robuste en écriture) pour ne pas perdre de données lors de coupures.
- **Sécurité & sûreté** : watchdog, brown-out, RTOS health monitor, checksum journaux.
- **Énergie** : supervision tension d’entrée et température interne MCU.

> Intérêt “indus” : capteurs I²C/SPI, bus CAN, contraintes temps réel souples (mesures périodiques + gestion d’alarmes), persistance FRAM, stratégie de dégradé en cas de défaut — typique d’IoT industriel.
> 

---

# 2) Cibles matérielles & logicielles

## 2.1 Matériel (HW)

- **MCU** : **STM32F429ZI** ou **STM32F407VG** (famille F4 : FPU, CAN, timers riches).
    - Carte de dev recommandée : **NUCLEO-F429ZI** (ou NUCLEO-F407ZG).
- **Capteurs** :
    - **Temp/Hum** : SHT31 (I²C) ou HDC1080 (I²C).
    - **Temp MCU** : capteur interne + compensation.
    - **Contact porte** : entrée **GPIO** avec pull-up.
- **Stockage non volatile** : **FRAM SPI** (ex. MB85RS256B 32 KB ou 256 KB selon besoin).
- **Coms** :
    - **CAN** : transceiver TJA1051/TXN. (Nucleo: shield ou breadboard).
    - **UART** : via ST-Link VCP pour console.
- **Sorties** :
    - **Relais/SSR** (pilotage via GPIO + transistor), **buzzer** (GPIO/PWM), **LED état** (GPIO).
- **Alimentation** :
    - Entrée 24 V → step-down 5 V/3.3 V (maquette : USB 5 V). Mesure tension via **pont + ADC** (option).

> Tous ces choix ont des équivalents faciles à “stubber” pour la cible SIM (Keil).
> 

## 2.2 Logiciel (tooling)

- **Langage** : C (C99).
- **RTOS** : **FreeRTOS v10.x** (CMSIS-RTOS2 wrapper facultatif).
- **Keil µVision (MDK-ARM)** :
    - **Cible SIM** : build « Simulation » (Event Recorder, stubs drivers, tests timing/états).
    - **Cible HW** : build « NUCLEO-F429ZI ».
- **STM32CubeIDE** : projet parallèle pour démonstration client (build & flash).
- **GCC ARM / CMake** (optionnel): build CI.
- **GitHub** : repo public, Actions CI, badges, Releases.
- **Qualité** : MISRA-C:2012 (guidelines, pas 100 % de conformité), **clang-format**, **cppcheck**, **CMock/Ceedling** (unit tests host).

---

# 3) Exigences fonctionnelles

## 3.1 Mesures & périodicité

- **Acquisition** toutes **1 s** :
    - Température/HR via **I²C** (SHT31/HDC1080).
    - Temp MCU (ADC interne).
    - Tension d’alim (ADC) si câblée.
    - État **contact porte** (GPIO).
- **Filtrage/Validation** :
    - Rejet outliers simples (z-score ou min/max plausibles).
    - Hystérésis alarme température (limites hautes/basses configurables).
- **Compensation** : offset calibrable (NVM) pour capteur T/H.

## 3.2 Journalisation

- **Buffer circulaire RAM** (ex: 512 entrées) + **commit périodique** (toutes 10 s) vers **FRAM SPI**.
- **Entrée journal** = timestamp (uptime), T/HR, T_mcu, V_in, porte, flags alarmes, CRC-8.
- **Rétention** minimale : 7 jours @1 Hz (dimensionnement FRAM ajustable).
- **Export** via **UART** (commande CLI) ou lecture par trame CAN spéciale.

## 3.3 Communication

- **CAN** @ 250 kbps (ajustable) — protocole **léger propriétaire** (frames de télémétrie + alarmes + config):
    - **Telemetry Frame** (ID 0x180 + noeudID) : T/HR/Tmcu/Vin/door/flags.
    - **Alarm Frame** (ID 0x280 + noeudID) : seuil dépassé, état porte > X s, capteur en défaut.
    - **Config/Diag** : lecture/écriture seuils, période, reset stats, dump FRAM.
- **UART CLI** (115200 8N1) :
    - `status`, `get th`, `set thigh=<..>`, `log dump`, `log erase`, `can id=<..>`, `reboot`, etc.
    - Auto-help + complétion simple (option).

## 3.4 Actions & états

- **Alarmes** :
    - LED rouge clignotante + buzzer intermittent si T hors plage > 5 s.
    - Relais ventilation **ON** si T > seuil_haut + hystérésis.
- **LED état système** (verte) :
    - **1 Hz** : OK
    - **2 Hz** : alarme active
    - **rapide** : mode boot/config
- **Modes** :
    - **RUN** (nominal), **DEGRADED** (capteur HS → alarme soft + CAN diag), **SAFE** (watchdog recovery).
- **Config persistante** :
    - Seuils/hystérésis/périodes/NodeID CAN → stockés en FRAM (ou flash emul eeprom).

---

# 4) Exigences non fonctionnelles

- **Temps réel** : gigue acquisition < **10 ms** @ période 1 s ; traitement complet < **50 ms**.
- **Robustesse** : watchdog actif (IWDG), brown-out reset, recovery après reset (pas de perte de config), CRC journaux.
- **Portabilité** : couche **HAL** isolée, **mock drivers** pour SIM.
- **Testabilité** : unit tests sur logique (sans HAL), hooks d’injection capteurs en SIM.
- **Traçabilité** : Event Recorder (Keil) / logs UART.
- **Style** : clang-format + conventions de nommage; commentaires doxygen.
- **Sécurité** : pas d’allocation dynamique en boucle; limites vérifiées; timeouts bus.

---

# 5) Architecture logicielle

```
/app
  /core        (application, state machine, config, alarms)
  /rtos        (wrappers FreeRTOS: tasks, queues, timers, events)
  /drivers     (hal_adc, hal_i2c, hal_spi, hal_can, hal_gpio, fram, sensor_th)
  /platform
     /hw       (STM32 HAL, BSP Nucleo)
     /sim      (stubs, data feeders, pseudo-can, virtual fram)
  /middleware  (cli uart, can_proto, logger, ringbuf, crc)
  /tests       (hosted unit tests - Ceedling)

```

## 5.1 Tâches FreeRTOS (exemple)

- **tsk_acq (prio 2, 1 Hz)** : acquisitions I²C/ADC + filtrage + push queue.
- **tsk_proc (prio 2, 10 Hz)** : calcul états/alarme + mise à jour config/FRAM.
- **tsk_can (prio 1, 10 Hz)** : émission trames périodiques, RX commandes.
- **tsk_cli (prio 1, event-driven)** : shell UART.
- **tsk_blink (prio 0, 5 Hz)** : LED état/buzzer pattern.
- **timer_commit (SW timer, 10 s)** : flush ring buffer vers FRAM.
- **watchdog kicker** : dans idle hook + checkpoints critiques.

**Comms inter-tâches** : **queues** (télémétrie), **event groups** (flags alarme), **mutex** (FRAM/SPI).

---

# 6) Simulation Keil µVision (cible SIM)

- **Build “SIM”** : remplace drivers **platform/sim** :
    - **Capteurs simulés** : générateurs pseudo-aléatoires paramétrables (ramps, steps, pannes).
    - **CAN simulé** : boucle locale / log Event Recorder (ou UART trace).
    - **FRAM simulée** : RAM + fichier map (si autorisé) ou simple buffer.
    - **GPIO/LED/Buzzer** : états remontés via Event Recorder / printf.
- **Scénarios SIM** :
    1. **Nominal** : T=4 °C, HR=60 %, porte fermée → aucune alarme, LED 1 Hz.
    2. **Porte ouverte > 30 s** → alarme + buzzer, trame CAN “Door”.
    3. **T dérive** : palier au-dessus du seuil → hystérésis + relais ON, journaux persistés.
    4. **Capteur HS** (NACK I²C) → mode DEGRADED, alerte CAN diag.
    5. **Reset watchdog** simulé → recovery, config intacte, continuité journal.
- **Mesures en SIM** : timestamps, périodicité des trames, couverture des chemins d’alarmes.

---

# 7) Tests & validation

## 7.1 Unit tests (host)

- **Core** : calcul états, hystérésis, anti-rebonds, CRC.
- **Logger** : ring buffer, sérialisation entrée journal, parse dump.
- **Config** : set/get + persistance mock FRAM.
- **Can_proto** : pack/unpack trames, checks.

## 7.2 Intégration (HW)

- **I²C** : lecture régulière capteur, gestion erreurs (timeout/retry).
- **SPI FRAM** : cycles écriture/lecture, endurance spot-check.
- **CAN** : trames périodiques, RX commandes modification seuils.
- **UART CLI** : scénario opérateur (dump, set seuils, reboot).
- **Watchdog/brown-out** : tests de recovery.

## 7.3 Critères d’acceptation (extract)

- Jitter acquisition < 10 ms @1 Hz, loss rate 0 % sur 24 h.
- 100 % trames CAN attendues @10 Hz sur 1 h, sans overrun.
- Journal FRAM lisible après 10 cycles marche/arrêt, CRC ok.
- Scénarios SIM 1→5 passent, logs conformes.

---

# 8) Livrables

1. **Code source** (licence MIT) + **README** pro (contexte, schémas, GIF démo SIM).
2. **Deux projets**:
    - **Keil µVision** (targets `SIM`, `NUCLEO-F429ZI`).
    - **STM32CubeIDE** (build/flash HW).
3. **Doc Doxygen** (API) + **User Guide** (CLI, frames CAN, intégration).
4. **Plan de tests** + rapports (unit/integration) + scripts Ceedling/GitHub Actions.
5. **Schéma d’architecture** (bloc) + **schéma brochage** (Nucleo ↔ capteurs).
6. **Capture Event Recorder** & **trames CAN** d’un run type (pour portfolio).
7. **Release v1.0** (tag Git, changelog, artefacts .hex/.elf).

---

# 9) Organisation GitHub (portfolio-friendly)

```
.
├─ /app
├─ /platform/hw, /platform/sim
├─ /docs (readme, arch.md, can_protocole.md, test_plan.md)
├─ /keil (uVision project)
├─ /cubeide (STM32CubeIDE project)
├─ /ci (scripts, ceedling, cppcheck config, clang-format)
└─ .github/workflows (build, unit, static-analysis, doxygen-pages)

```

- **Branching** : `main` (stable), `dev`, feature branches.
- **CI** :
    - Build SIM (host) + unit tests sur chaque PR.
    - Static analysis (**cppcheck**) + format check.
    - Doxygen → **GitHub Pages**.
    - Release : artefacts `.elf/.hex`, archive docs.
- **Badges** : build, tests, coverage, docs.

---

# 10) Planning indicatif (2–3 semaines, solo)

- **S1** : skeleton projet (tasks FreeRTOS, drivers mock, CLI UART), capteur I²C (sim), logger RAM, CAN proto.
- **S2** : FRAM SPI + persistance, modes/états/alarme, scénarios SIM, unit tests, Event Recorder.
- **S3** : bring-up HW Nucleo, intégration capteur réel + CAN, fin tests, docs, release v1.0.

---

# 11) Table de configuration (extraits)

- `TEMP_HIGH` = 6.0 °C | `TEMP_LOW` = 0.0 °C | `HYST` = 0.5 °C
- `PERIOD_ACQ_MS` = 1000 | `PERIOD_CAN_MS` = 100 | `COMMIT_MS` = 10000
- `NODE_ID` = 0x12 | `CAN_BAUD` = 250000 | `UART_BAUD` = 115200

---

# 12) Mapping vers tes objectifs

1. **Révision C/FreeRTOS**
    - Tâches/Queues/Events/Timers, watchdog, drivers I²C/SPI/CAN/UART, CLI, gestion erreurs, persistance, state machine — panorama quasi complet d’un embarqué indus.
2. **Versionning GitHub (pro)**
    - Repo structuré, CI, badges, releases, docs en Pages, tests automatiques → crédible pour recrutements & clients.
3. **Portfolio Syloria**
    - README orienté “produit”, captures Event Recorder/CLI, GIF simulation, frames CAN, schémas — réutilisable en **case study** sur ton site.

---

# 13) Annexes utiles (à implémenter)

- **Protocole CAN (TLV minimal)** :
    - 0x01 TEMP (°C *100 int16), 0x02 HUM (% *100 uint16), 0x03 TMCU, 0x04 VIN (mV), 0x05 DOOR (bool), 0x06 FLAGS, 0x10 THIGH, 0x11 TLOW, 0x12 HYST…
- **CLI exemples** :
    - `get telem`, `get cfg`, `set thigh 7.0`, `log dump 100`, `can id 0x34`, `reboot`
- **Erreurs normalisées** :
    - `ERR_I2C_TIMEOUT`, `ERR_SPI_WRITE`, `ERR_CAN_TX_OVR`, `ERR_FRAM_CRC`, etc. (compteur + dernier code).
- **Patterns** :
    - Ring buffer lock-free (SP), State machine alarme, Driver abstraction (HW/SIM), Adapter CLI↔config, CRC module.