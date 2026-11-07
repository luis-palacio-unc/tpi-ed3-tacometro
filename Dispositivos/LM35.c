/**
 * @file LM35.c
 * @brief Calculo entero de temperatura del modelo ideal LM35 sin offset.
 * @details TI 7.3.1: 10 mV por grado equivalen a 100 uV por centesima Celsius.
 * Producto y divisor caben en uint64_t; redondear por resto evita sumar al numerador.
 * El dominio valido da hasta 42949673 centesimas, representables en uint32_t.
 * @note ESW.2.1.11. Libgcc existente resuelve division/modulo en ARM; sin libc.
 */
#include "LM35.h"
#include <stddef.h>

/** @addtogroup lm35
 * @{
 */

/** @brief Factor nominal del LM35: microvoltios por centesima de grado Celsius. */
#define LM35_UV_POR_CENTESIMA UINT64_C(100)

bool LM35_InicializarLectura(LM35_Lectura *lectura)
{
    if (lectura == NULL) {
        return false;
    }
    lectura->centesimas_c = 0U;
    lectura->estado = LM35_SIN_DATOS;
    return true;
}

LM35_Estado LM35_ConvertirMuestra(LM35_Lectura *lectura,
                                        uint32_t muestra, uint32_t escala_adc,
                                        uint32_t referencia_uv)
{
    if (lectura == NULL) {
        return LM35_ERROR_ARGUMENTO;
    }
    lectura->centesimas_c = 0U;
    if (escala_adc == 0U) {
        lectura->estado = LM35_ERROR_ESCALA;
        return lectura->estado;
    }
    if (referencia_uv == 0U) {
        lectura->estado = LM35_ERROR_REFERENCIA;
        return lectura->estado;
    }
    if (muestra >= escala_adc) {
        lectura->estado = LM35_ERROR_MUESTRA;
        return lectura->estado;
    }
    const uint64_t numerador = (uint64_t)muestra * referencia_uv;
    const uint64_t divisor = (uint64_t)escala_adc * LM35_UV_POR_CENTESIMA;
    uint64_t centesimas = numerador / divisor;
    const uint64_t resto = numerador % divisor;
    if (resto >= divisor / 2U) {
        ++centesimas;
    }
    lectura->centesimas_c = (uint32_t)centesimas;
    lectura->estado = LM35_VALIDA;
    return lectura->estado;
}

/** @} */
