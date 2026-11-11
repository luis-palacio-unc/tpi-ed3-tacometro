/**
 * @file Potenciometro.c
 * @brief Lectura del resultado ADC desde la palabra transferida a memoria.
 * @details Posee el almacenamiento de su muestra y reutiliza ADC_GDR_RESULT
 * del driver original sin acceder al periferico.
 * @note ESW.2.1.12. Sin estados, validaciones o configuracion propias.
 */
#include "Potenciometro.h"
#include "Adquisicion_ADC_DMA.h"
#include "lpc17xx_adc.h"

/**
 * @brief Palabra AD0DR0 del potenciometro, destino de la transferencia DMA.
 * @details Propiedad de Potenciometro; declaracion extern en el enlace interno
 * Adquisicion_ADC_DMA.h para la configuracion del productor.
 * @note Volatile, 32 bits, .bss local alineada a 4 bytes en LPC1769.
 */
volatile uint32_t ADC_DMA_Pot;

uint16_t Pot_Read(void)
{
    return (uint16_t)ADC_GDR_RESULT(ADC_DMA_Pot);
}
