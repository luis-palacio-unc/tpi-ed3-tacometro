/**
 * @file Analogica.h
 * @brief Referencia configurable para interpretar las muestras analogicas.
 * @details Default 3300 mV por decision del usuario (VREFP a 3,3 V de la placa);
 * no acredita la tension real del pin ni el cableado.
 * @note B05-R1/B18. Medir VREFP en la placa y ajustar si difiere.
 */
#ifndef TPI_ANALOGICA_H
#define TPI_ANALOGICA_H

#include <stdint.h>

/**
 * @brief Referencia ADC en mV; default 3300, reemplazable al compilar.
 * @details El fondo de escala del ADC es la tension de VREFP (UM10360 cap. 29,
 * tabla 530); la referencia externa de 2,54 V del anexo A no se usa.
 * El limite garantiza centesimas representables en int32_t para el modelo LM35.
 */
#ifndef ADC_VREF_MV
#define ADC_VREF_MV UINT32_C(3300)
#endif
#if ADC_VREF_MV <= 0 || ADC_VREF_MV > INT32_MAX / 10
#error "ADC_VREF_MV debe ser positivo y permitir salida int32_t en centesimas."
#endif

#endif /* TPI_ANALOGICA_H */
