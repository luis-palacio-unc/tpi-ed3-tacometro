/**
 * @file Analogica.h
 * @brief Referencia configurable para interpretar las muestras analogicas.
 * @details El default permite compilar y no acredita VREF ni cableado de la placa.
 * @note B05-R1. Confirmar referencia fisica e inicializacion al integrar ADC/DMA.
 */
#ifndef TPI_ANALOGICA_H
#define TPI_ANALOGICA_H

#include <stdint.h>

/**
 * @brief Referencia ADC en mV; default provisional 2540, reemplazable al compilar.
 * @details El anexo A indica 2,54 V; verificar el valor fisico al integrar.
 * El limite garantiza centesimas representables en int32_t para el modelo LM35.
 */
#ifndef ADC_VREF_MV
#define ADC_VREF_MV UINT32_C(2540)
#endif
#if ADC_VREF_MV <= 0 || ADC_VREF_MV > INT32_MAX / 10
#error "ADC_VREF_MV debe ser positivo y permitir salida int32_t en centesimas."
#endif

#endif /* TPI_ANALOGICA_H */
