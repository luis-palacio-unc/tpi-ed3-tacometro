/**
 * @file Medicion_Velocidad.c
 * @brief Velocidad de giro con factor explicito y aritmetica acotada de 64 bits.
 * @details A=pulsos*1000000000 y D=duracion_us*factor caben en uint64_t.
 * El 60 convierte segundos a minutos, no representa pulsos/vuelta.
 * El resultado ideal es 60*(A/D): dividir A primero sin descartar su resto.
 * Seis bits de 60 producen cociente/resto de 60*r/D sin formar ese producto.
 * Comparar resto con D-resto evita overflow al doblar y al redondear; no hay
 * redondeo intermedio. La fraccion entera queda en 0..59 y el resto en 0..D-1.
 * @note ESW.2.1.3. libgcc existente resuelve division/modulo64; sin libc.
 */
#include "Medicion_Velocidad.h"
#include <stddef.h>

/** @addtogroup medicion_velocidad
 * @{
 */

/** @brief Conversion universal de unidades de tiempo, independiente del sensor. */
#define SEGUNDOS_POR_MINUTO 60U

bool Medicion_Velocidad_Inicializar(Medicion_Velocidad_Lectura *lectura)
{
    if (lectura == NULL) {
        return false;
    }
    lectura->rpm_milesimas = 0U;
    lectura->estado = MEDICION_VELOCIDAD_SIN_DATOS;
    return true;
}

Medicion_Velocidad_Estado Medicion_Velocidad_Actualizar(
    Medicion_Velocidad_Lectura *lectura, uint32_t pulsos, uint32_t duracion_us,
    uint32_t pulsos_por_vuelta)
{
    if (lectura == NULL) {
        return MEDICION_VELOCIDAD_ERROR_ARGUMENTO;
    }
    lectura->rpm_milesimas = 0U;
    if (duracion_us == 0U) {
        lectura->estado = MEDICION_VELOCIDAD_ERROR_DURACION;
        return lectura->estado;
    }
    if (pulsos_por_vuelta == 0U) {
        lectura->estado = MEDICION_VELOCIDAD_ERROR_PULSOS_POR_VUELTA;
        return lectura->estado;
    }
    const uint64_t numerador = (uint64_t)pulsos * UINT64_C(1000000000);
    const uint64_t divisor = (uint64_t)duracion_us * pulsos_por_vuelta;
    const uint64_t cociente = numerador / divisor;
    if (cociente > UINT32_MAX / SEGUNDOS_POR_MINUTO) {
        lectura->estado = MEDICION_VELOCIDAD_ERROR_RANGO;
        return lectura->estado;
    }
    const uint64_t resto_inicial = numerador % divisor;
    uint64_t resto = 0U;
    uint32_t fraccion = 0U;
    for (uint32_t bit = 32U; bit != 0U; bit >>= 1U) {
        fraccion *= 2U;
        if (resto >= divisor - resto) {
            resto -= divisor - resto;
            ++fraccion;
        } else {
            resto += resto;
        }
        if ((SEGUNDOS_POR_MINUTO & bit) != 0U) {
            if (resto >= divisor - resto_inicial) {
                resto -= divisor - resto_inicial;
                ++fraccion;
            } else {
                resto += resto_inicial;
            }
        }
    }
    uint64_t redondeado = cociente * SEGUNDOS_POR_MINUTO + fraccion;
    if (resto >= divisor - resto) {
        ++redondeado;
    }
    if (redondeado > UINT32_MAX) {
        lectura->estado = MEDICION_VELOCIDAD_ERROR_RANGO;
        return lectura->estado;
    }
    lectura->rpm_milesimas = (uint32_t)redondeado;
    lectura->estado = MEDICION_VELOCIDAD_VALIDA;
    return lectura->estado;
}

/** @} */
