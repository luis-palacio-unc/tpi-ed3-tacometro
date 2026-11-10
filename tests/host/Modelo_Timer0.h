#ifndef TEST_MODELO_TIMER0_H
#define TEST_MODELO_TIMER0_H
#include "LPC17xx.h"
#include <stdbool.h>
#include <stddef.h>
typedef struct { char operacion; uint32_t tcr,tc,pc,pr,mcr,ccr,ctcr,ir; } Modelo_Evento;
extern Modelo_Evento modelo_eventos[32];
extern size_t modelo_cantidad;
extern bool modelo_irq_habilitada,modelo_irq_pendiente,modelo_error;
void Modelo_Limpiar(void);
void Modelo_AvanzarPclk(uint64_t ciclos);
#endif
