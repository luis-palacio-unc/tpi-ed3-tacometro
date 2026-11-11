/**
 * @file Potenciometro.c
 * @brief Lectura del resultado ADC desde la palabra transferida a memoria.
 * @details Reutiliza ADC_GDR_RESULT del driver original sin acceder al periferico.
 * @note ESW.2.1.12. Sin estados, validaciones o configuracion propias.
 */
#include "Potenciometro.h"
#include "Adquisicion_ADC_DMA.h"
#include "lpc17xx_adc.h"

uint16_t Pot_Read(void)
{
    return (uint16_t)ADC_GDR_RESULT(ADC_DMA_Pot);
}
