/**
 * @file Adquisicion_ADC_DMA.c
 * @brief Reserva de la palabra destino para la muestra del potenciometro.
 * @details No configura ADC, DMA, pines, reloj ni interrupciones.
 * @note La adquisicion compartida con LM35 se completara al integrar.
 */
#include "Adquisicion_ADC_DMA.h"

volatile uint32_t ADC_DMA_Pot;
