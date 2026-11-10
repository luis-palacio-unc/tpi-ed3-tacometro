#ifndef TEST_MODELO_BASE_TIEMPOS_H
#define TEST_MODELO_BASE_TIEMPOS_H
#include "Base_Tiempos.h"
#include "Base_Tiempos_Puerto.h"
#include <stdbool.h>
#include <stddef.h>
typedef enum { MODELO_ENTRAR, MODELO_ESCRIBIR, MODELO_SALIR } Modelo_Operacion;
typedef struct { Modelo_Operacion op; Base_Tiempos_Registro reg; uint32_t valor; } Modelo_Evento;
typedef struct {
    uint32_t control, recarga, actual, icsr, prioridad, primask;
    bool pendiente, inyectar_al_salir, error;
    unsigned entregados;
    size_t cantidad;
    Modelo_Evento eventos[64];
} Modelo_Base;
extern Modelo_Base modelo_base;
void Modelo_Limpiar(void);
void Modelo_LimpiarTraza(void);
void Modelo_Tick(void);
void Modelo_Avanzar(uint32_t ticks);
void Modelo_Despachar(void);
#endif
