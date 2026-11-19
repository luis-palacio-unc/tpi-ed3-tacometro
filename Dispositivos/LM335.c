/**
 * @file LM335.c
 * @brief Conversion entera de la muestra del LM335 almacenada por ADC/DMA.
 * @details Reutiliza ADC_GDR_RESULT original; conserva redondeo B05 sin flotantes.
 * Pasa de kelvin a Celsius y aplica el ajuste de calibracion de Analogica.h.
 * @note ESW.2.1.11. Sin configuracion, acceso a registros, estados o normalizacion de alarma.
 */
#include "LM335.h"
#include "Analogica.h"
#include "Adquisicion_ADC_DMA.h"
#include "lpc17xx_adc.h"

/** @addtogroup lm335
 * @{
 */

/** @brief Resolucion del ADC LPC1769: 12 bits de RESULT. */
#define LM335_ADC_BITS 12U
/** @brief Cuentas por referencia, distintas del ultimo codigo inclusivo. */
#define LM335_ADC_CUENTAS (UINT32_C(1) << LM335_ADC_BITS)
/** @brief Ganancia nominal del LM335: 10 mV por kelvin. */
#define LM335_MV_POR_GRADO UINT32_C(10)
/** @brief Factor exacto: 100 centesimas por grado / 10 mV por grado. */
#define LM335_CENTESIMAS_POR_MV (LM335_CENTESIMAS_POR_GRADO / LM335_MV_POR_GRADO)
/** @brief 0 grados Celsius en centesimas de kelvin: 273,15 K. */
#define LM335_CERO_CELSIUS_CENTESIMAS INT32_C(27315)

/**
 * @brief Palabra ADC de la muestra LM335, propiedad de este dispositivo.
 * @details Destino DMA de 32 bits (AD0DR1) alineado a 4 bytes en .bss local del LPC1769.
 * RESULT ocupa 15:4; los demas campos no participan en la conversion.
 * @note Extern interno en Adquisicion_ADC_DMA.h. DMA es el unico escritor
 * productivo; el soporte asocia AD0.1 a este destino por descriptor.
 */
volatile uint32_t ADC_DMA_LM335;

/**
 * @brief Convertir un codigo ADC al modelo nominal del LM335 en centesimas Celsius.
 * @param[in] muestra Codigo ya extraido de RESULT, entre 0 y 4095 inclusive.
 * @return Redondeo al entero mas cercano, empate hacia arriba; int32_t con signo.
 * @pre Referencia positiva representable y ajuste acotado, asegurados por
 * Analogica.h al compilar.
 * @details Numerador = muestra * VREF_mV * centesimas_por_mV; divisor = cuentas ADC.
 * Se redondea por el resto una sola vez, sin tension intermedia truncada, y da
 * centesimas de kelvin. Despues se restan los 273,15 K del cero Celsius y se suma
 * LM335_AJUSTE_CENTESIMAS; como ambos son enteros, el redondeo no cambia. El dominio
 * admitido de VREF y del ajuste hace que el numerador quepa en uint64_t y el
 * resultado en int32_t.
 * @note USW.2.1.11.2. Unidad interna nueva; IDs .11.1/.11.2 retirados, sin reciclar.
 */
static int32_t LM335_Convert(uint16_t muestra)
{
    const uint64_t numerador = (uint64_t)muestra * ADC_VREF_MV * LM335_CENTESIMAS_POR_MV;
    uint64_t centesimas = numerador / LM335_ADC_CUENTAS;
    const uint64_t resto = numerador % LM335_ADC_CUENTAS;
    if (resto >= LM335_ADC_CUENTAS / 2U) {
        ++centesimas;
    }
    return (int32_t)centesimas - LM335_CERO_CELSIUS_CENTESIMAS + LM335_AJUSTE_CENTESIMAS;
}

int32_t getTemperatura(void)
{
    return LM335_Convert((uint16_t)ADC_GDR_RESULT(ADC_DMA_LM335));
}

/** @} */
