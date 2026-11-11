/*
 * Dobles host de las funciones NVIC en linea de CMSIS (core_cm3.h).
 * Incluir ANTES que cualquier encabezado de la biblioteca y que el .c bajo prueba:
 * renombra las definiciones originales, que escribirian en 0xE000E100, y deja en
 * su lugar un modelo en RAM. No forma parte del firmware.
 */
#ifndef TPI_DOBLES_CMSIS_H
#define TPI_DOBLES_CMSIS_H

#define NVIC_EnableIRQ NVIC_EnableIRQ_cmsis
#define NVIC_DisableIRQ NVIC_DisableIRQ_cmsis
#define NVIC_SetPriority NVIC_SetPriority_cmsis
#define NVIC_SetPendingIRQ NVIC_SetPendingIRQ_cmsis
#include "LPC17xx.h"
#undef NVIC_EnableIRQ
#undef NVIC_DisableIRQ
#undef NVIC_SetPriority
#undef NVIC_SetPendingIRQ

#include <stdbool.h>
#include <string.h>

#define NVIC_MODELO_IRQS 35

typedef struct {
    bool habilitada[NVIC_MODELO_IRQS];
    bool pendiente[NVIC_MODELO_IRQS];
    uint32_t prioridad[NVIC_MODELO_IRQS];
    unsigned habilitaciones, deshabilitaciones, prioridades, pedidos;
    bool error;
} NvicModelo;

static NvicModelo nvic;

static inline bool NvicValida(IRQn_Type irq)
{
    if ((int)irq < 0 || (int)irq >= NVIC_MODELO_IRQS) { nvic.error = true; return false; }
    return true;
}

static inline void NvicReiniciar(void)
{
    memset(&nvic, 0, sizeof(nvic));
    for (unsigned i = 0U; i < NVIC_MODELO_IRQS; ++i) { nvic.prioridad[i] = UINT32_C(0xFFFFFFFF); }
}

static inline void NVIC_EnableIRQ(IRQn_Type irq)
{
    if (NvicValida(irq)) { nvic.habilitada[irq] = true; ++nvic.habilitaciones; }
}

static inline void NVIC_DisableIRQ(IRQn_Type irq)
{
    if (NvicValida(irq)) { nvic.habilitada[irq] = false; ++nvic.deshabilitaciones; }
}

static inline void NVIC_SetPriority(IRQn_Type irq, uint32_t prioridad)
{
    if (NvicValida(irq)) { nvic.prioridad[irq] = prioridad; ++nvic.prioridades; }
}

static inline void NVIC_SetPendingIRQ(IRQn_Type irq)
{
    if (NvicValida(irq)) { nvic.pendiente[irq] = true; ++nvic.pedidos; }
}

#endif /* TPI_DOBLES_CMSIS_H */
