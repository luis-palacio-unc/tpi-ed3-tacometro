/**
 * @file LM35.c
 * @brief Conversion entera de la muestra del LM35 almacenada por ADC/DMA.
 * @details Reutiliza ADC_GDR_RESULT original; conserva redondeo B05 sin flotantes.
 * @note ESW.2.1.11. Sin configuracion, acceso a registros, estados o normalizacion de alarma.
 */
#include "LM35.h"
#include "Analogica.h"
#include "Adquisicion_ADC_DMA.h"
#include "lpc17xx_adc.h"

/** @addtogroup lm35
 * @{
 */

/** @brief Resolucion del ADC LPC1769: 12 bits de RESULT. */
#define LM35_ADC_BITS 12U
/** @brief Cuentas por referencia, distintas del ultimo codigo inclusivo. */
#define LM35_ADC_CUENTAS (UINT32_C(1) << LM35_ADC_BITS)
/** @brief Ganancia nominal del LM35: 10 mV por grado Celsius. */
#define LM35_MV_POR_GRADO UINT32_C(10)
/** @brief Factor exacto: 100 centesimas por grado / 10 mV por grado. */
#define LM35_CENTESIMAS_POR_MV (LM35_CENTESIMAS_POR_GRADO / LM35_MV_POR_GRADO)

/**
 * @brief Palabra ADC de la muestra LM35, propiedad de este dispositivo.
 * @details Destino DMA de 32 bits (AD0DR1) alineado a 4 bytes en .bss local del LPC1769.
 * RESULT ocupa 15:4; los demas campos no participan en la conversion.
 * @note Extern interno en Adquisicion_ADC_DMA.h. DMA es el unico escritor
 * productivo; el soporte asocia AD0.1 a este destino por descriptor.
 */
volatile uint32_t ADC_DMA_LM35;

/**
 * @brief Convertir un codigo ADC al modelo nominal del LM35 en centesimas Celsius.
 * @param[in] muestra Codigo ya extraido de RESULT, entre 0 y 4095 inclusive.
 * @return Redondeo al entero mas cercano, empate hacia arriba; int32_t no negativo.
 * @pre Referencia positiva representable, asegurada por Analogica.h al compilar.
 * @details Numerador = muestra * VREF_mV * centesimas_por_mV; divisor = cuentas ADC.
 * Se redondea por el resto una sola vez, sin tension intermedia truncada. El dominio
 * admitido de VREF hace que el numerador quepa en uint64_t y el resultado en int32_t.
 * @note USW.2.1.11.4. Unidad interna nueva; IDs .11.1/.11.2 retirados, sin reciclar.
 */
static int32_t LM35_Convert(uint16_t muestra)
{
    const uint64_t numerador = (uint64_t)muestra * ADC_VREF_MV * LM35_CENTESIMAS_POR_MV;
    uint64_t centesimas = numerador / LM35_ADC_CUENTAS;
    const uint64_t resto = numerador % LM35_ADC_CUENTAS;
    if (resto >= LM35_ADC_CUENTAS / 2U) {
        ++centesimas;
    }
    return (int32_t)centesimas;
}

int32_t LM35_Read(void)
{
    return LM35_Convert((uint16_t)ADC_GDR_RESULT(ADC_DMA_LM35));
}

/** @} */
