But : modules utilitaires/middleware réutilisables (drivers custom, libs maison).
Tous les fichiers .h se trouve dans Inc

Contenu typique :

logger.c / logger.h → ring buffer + commit FRAM.

fram_spi.c / fram_spi.h → driver FRAM (SPI).

sensor_th.c / sensor_th.h → driver capteur T/H I²C.

can_proto.c / can_proto.h → sérialisation/desérialisation trames CAN.

cli_uart.c / cli_uart.h → parsing commandes.

crc_utils.c / crc_utils.h → fonctions CRC/validation.

mock_*.[ch] (optionnel, pour simulation Keil).