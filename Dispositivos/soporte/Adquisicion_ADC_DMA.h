/**
 * @file Adquisicion_ADC_DMA.h
 * @brief Enlace interno con la adquisicion ADC/DMA.
 * @details El soporte conserva la responsabilidad de configurar ADC/DMA para
 * LM35 y Potenciometro. La muestra pertenece a Potenciometro.c; este
 * encabezado declara su destino para la futura configuracion DMA.
 * @note Configuracion, arranque y secuencia multicanal pendientes de integracion.
 */
#ifndef TPI_ADQUISICION_ADC_DMA_H
#define TPI_ADQUISICION_ADC_DMA_H

#include <stdint.h>

/**
 * @brief Palabra ADGDR transferida por DMA para AD0.0/P0.23.
 * @details Definida una sola vez en Potenciometro.c. El soporte utilizara su
 * direccion como destino DMA. Son 32 bits, alineados naturalmente a 4 bytes.
 * RESULT ocupa bits 15:4; Pot_Read extrae las 12 cuentas con el driver original.
 * @note Uso interno de dispositivos. El DMA sera el unico escritor en ejecucion;
 * la CPU consulta volatile. Se ubica en .bss de SRAM local, accesible por GPDMA
 * con el microcontrolador activo. No representa una lectura disponible al arrancar.
 * La inicializacion y el destino/canal efectivos se estableceran al integrar.
 */
extern volatile uint32_t ADC_DMA_Pot;

#endif /* TPI_ADQUISICION_ADC_DMA_H */
