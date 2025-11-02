/**
 * @file    logger.c
 * @brief   Gestion d’un ring buffer RAM et commit périodique vers la FRAM SPI.
 *          Version simple basée sur FreeRTOS Message Buffer.
 */
#include "logger.h"
#include "fram_spi.h"    // pour Fram_WriteSample(...) à adapter
#include <string.h>

static MessageBufferHandle_t s_msgbuffer = NULL;

/* Taille réelle du buffer en octets */
#define LOGGER_BUF_BYTES   (LOGGER_RING_CAPACITY * sizeof(telem_t))

int Logger_Init(void)
{
    s_msgbuffer = xMessageBufferCreate(LOGGER_BUF_BYTES);
    if (s_msgbuffer == NULL)
        return -1;
    return 0;
}

int Logger_Push(const telem_t* sample)
{
    if ((s_msgbuffer == NULL) || (sample == NULL))
        return -1;

    /* envoi non bloquant */
    size_t sent = xMessageBufferSend(
        s_msgbuffer,
        (const void *)sample,
        sizeof(telem_t),
        0 /* no wait */
    );

    if (sent != sizeof(telem_t)) {
        /* buffer plein → on drop pour l’instant */
        return -1;
    }

    return 0;
}

void Logger_Flush(void)
{
    if (s_msgbuffer == NULL)
        return;

    telem_t tmp;

    /* On vide tout le buffer RAM tant qu’il y a des messages */
    for (;;)
    {
        size_t got = xMessageBufferReceive(
            s_msgbuffer,
            &tmp,
            sizeof(tmp),
            0 /* no wait */
        );

        if (got != sizeof(tmp)) {
            /* plus rien à lire */
            break;
        }

        /* TODO: adapter à API réelle fram_spi */
        /* ex: Fram_WriteSample(&tmp, sizeof(tmp)); */
    }
}

void Logger_Clear(void)
{
    if (s_msgbuffer == NULL)
        return;

    xMessageBufferReset(s_msgbuffer);
}

size_t Logger_GetCount(void)
{
    if (s_msgbuffer == NULL)
        return 0;

    /* approx: octets utilisés / taille d’un sample */
    size_t used = xMessageBufferSpacesAvailable(s_msgbuffer);
    /* ce n’est pas exact → on pourra améliorer plus tard */
    return (LOGGER_BUF_BYTES - used) / sizeof(telem_t);
}
