# ⚙️ Core — Logique applicative & tâches FreeRTOS

**But :** contient la logique haut niveau du projet **Smart Cold-Chain Node (SCN)** :  
gestion des tâches FreeRTOS, machine d’états, traitement des mesures et déclenchement des alarmes.

Tous les fichiers d’en-tête (`.h`) se trouvent dans **Core/Inc**  
et les sources (`.c`) dans **Core/Src**.

---

## Contenu

| Fichier | Rôle |
|----------|------|
| **core_init.c / core_init.h** | Initialisation des **queues**, **timers** et **tâches FreeRTOS** de l’application. |
| **task_acq.c / task_acq.h** | Tâche d’acquisition capteurs : température, humidité, tension, état de porte. |
| **task_proc.c / task_proc.h** | Traitement et filtrage des mesures, gestion des **hystérésis**, alarmes et états système. |
| **task_can.c / task_can.h** | Communication **CAN** : envoi de télémétries, réception de commandes, diagnostics. |
| **task_cli.c / task_cli.h** | Interface **UART/CLI** : interprète les commandes utilisateur et renvoie les statuts. |
| **task_blink.c / task_blink.h** | Gestion **LED d’état** (1 Hz/2 Hz/rapide) et **buzzer** via PWM (TIM4_CH1). |
| **config.h** | Constantes globales : seuils par défaut, périodes, NodeID, paramètres CAN/UART. |

---

## Structure logique
1. `core_init.c` crée les objets FreeRTOS (queues, timers, tâches).
2. Chaque tâche (`task_*.c`) possède une fonction `*_Start()` appelée au démarrage.
3. Les modules `App/` sont utilisés ici (drivers, capteurs, logger, etc.).
4. L’ensemble forme la **boucle temps réel** du nœud de supervision.

---

## Modes de fonctionnement
- **RUN** : fonctionnement nominal.  
- **DEGRADED** : perte capteur / communication.  
- **SAFE** : recovery watchdog, mode minimal.
