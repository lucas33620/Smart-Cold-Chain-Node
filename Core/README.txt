But : logique haute-niveau (application, state machine, tâches FreeRTOS).

Contenu typique :

app_main.c / app_main.h → point d’entrée applicatif, crée les tâches FreeRTOS.

task_acq.c / task_acq.h → tâche acquisition capteurs.

task_proc.c / task_proc.h → tâche traitement + alarmes.

task_can.c / task_can.h → comms CAN.

task_cli.c / task_cli.h → interface UART/CLI.

task_blink.c / task_blink.h → LED/buzzer.

config.h → constantes globales (seuils défaut, périodes, NodeID).