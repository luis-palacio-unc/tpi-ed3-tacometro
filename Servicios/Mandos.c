/**
 * @file Mandos.c
 * @brief Conversion del potenciometro a por mil y de la llave a modo.
 * @details Sin estado propio.
 * @note ESW.2.1.5. Contratos publicos en Mandos.h.
 */
#include "Mandos.h"
#include "Limites.h"
#include "Potenciometro.h"
#include "Selector.h"

/** @brief Muestra maxima del potenciometro: ADC de 12 bits. */
#define MANDOS_POT_MAX UINT32_C(4095)

void Mandos_Init(void)
{
    Selector_Init();
}

uint16_t Mandos_LeerManual(void)
{
    const uint32_t muestra = Pot_Read();
    return (uint16_t)((muestra * TPI_MANDO_MANUAL_MAX + MANDOS_POT_MAX / 2U) / MANDOS_POT_MAX);
}

Mandos_Modo Mandos_LeerModo(void)
{
    return Selector_Read() ? MANDOS_MODO_MANUAL : MANDOS_MODO_AUTOMATICO;
}
