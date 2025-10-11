# 📦 App — Modules utilitaires & middleware

**But :** regroupe tous les modules réutilisables (drivers custom, bibliothèques maison, middlewares).  
Chaque module est indépendant et testable séparément.  
Tous les fichiers d’en-tête (`.h`) se trouvent dans **App/Inc** et les sources (`.c`) dans **App/Src**.

---

## Contenu

| Fichier | Rôle |
|----------|------|
| **logger.c / logger.h** | Gestion d’un ring buffer RAM et commit périodique vers la FRAM SPI (journalisation télémétrie). |
| **fram_spi.c / fram_spi.h** | Driver de la mémoire **FRAM SPI** (MB85RS256B) : lecture/écriture robuste et endurante. |
| **sensor_th.c / sensor_th.h** | Driver capteur de **température / humidité** (SHT31 ou HDC1080) via bus I²C. |
| **can_proto.c / can_proto.h** | Sérialisation et désérialisation des trames **CAN** (télémétrie, alarmes, configuration). |
| **cli_uart.c / cli_uart.h** | Gestion du **CLI UART** : parsing des commandes utilisateur (`status`, `set`, `log`, etc.). |
| **crc_utils.c / crc_utils.h** | Fonctions CRC8/CRC16 et utilitaires de validation des données. |
| **relay.c / relay.h** | Pilotage du **relais de ventilation** (ON/OFF avec hystérésis). |
| **mock_*.[ch]** *(optionnel)* | Simulations pour Keil µVision (drivers fictifs : capteur, CAN, FRAM, etc.). |
