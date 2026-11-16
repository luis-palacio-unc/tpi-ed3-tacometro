/**
 * @file Analogica.h
 * @brief Referencia configurable para interpretar las muestras analogicas.
 * @details Default 3300 mV por decision del usuario (VREFP a 3,3 V de la placa);
 * no acredita la tension real del pin ni el cableado.
 * Incluye el ajuste de calibracion de un punto del sensor de temperatura LM335.
 * @note B05-R1/B18. Medir VREFP en la placa y ajustar si difiere.
 */
#ifndef TPI_ANALOGICA_H
#define TPI_ANALOGICA_H

#include <stdint.h>

/**
 * @brief Referencia ADC en mV; default 3300, reemplazable al compilar.
 * @details El fondo de escala del ADC es la tension de VREFP (UM10360 cap. 29,
 * tabla 530); la referencia externa de 2,54 V del anexo A no se usa.
 * El limite garantiza centesimas representables en int32_t para el modelo LM335.
 */
#ifndef ADC_VREF_MV
#define ADC_VREF_MV UINT32_C(3300)
#endif
#if ADC_VREF_MV <= 0 || ADC_VREF_MV > INT32_MAX / 10
#error "ADC_VREF_MV debe ser positivo y permitir salida int32_t en centesimas."
#endif

/**
 * @brief Ajuste que se suma a la temperatura del LM335, en centesimas Celsius.
 * @details Default 0 (curva nominal). El LM335 sin calibrar puede apartarse varios
 * grados; se corrige en un punto: ajuste = temperatura de un termometro de
 * referencia menos la informada por el firmware, ambas en centesimas. Reemplazable
 * al compilar. Limitado a +-50 grados para detectar valores mal cargados.
 */
#ifndef LM335_AJUSTE_CENTESIMAS
#define LM335_AJUSTE_CENTESIMAS INT32_C(0)
#endif
#if LM335_AJUSTE_CENTESIMAS < -5000 || LM335_AJUSTE_CENTESIMAS > 5000
#error "LM335_AJUSTE_CENTESIMAS debe estar entre -5000 y 5000 centesimas."
#endif

#endif /* TPI_ANALOGICA_H */
