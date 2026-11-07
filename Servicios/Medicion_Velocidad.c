/**
 * @file Medicion_Velocidad.c
 * @brief Calculo acotado de velocidad con aritmetica entera de 64 bits.
 * @details Con 60 pulsos/vuelta, el factor para milesimas de RPM es 1 000 000 000.
 * UINT32_MAX * factor + UINT32_MAX/2 cabe en uint64_t; validar antes de estrechar.
 * @note ESW.2.1.3. El enlace ARM requiere auxiliares de division de libgcc existente.
 */
#include "Medicion_Velocidad.h"
#include <stddef.h>

/** @addtogroup medicion_velocidad
 * @{
 */

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
    Medicion_Velocidad_Lectura *lectura, uint32_t pulsos, uint32_t duracion_us)
{
    if (lectura == NULL) {
        return MEDICION_VELOCIDAD_ERROR_ARGUMENTO;
    }
    lectura->rpm_milesimas = 0U;
    if (duracion_us == 0U) {
        lectura->estado = MEDICION_VELOCIDAD_ERROR_DURACION;
        return lectura->estado;
    }
    const uint64_t numerador = (uint64_t)pulsos * UINT64_C(1000000000);
    const uint64_t redondeado = (numerador + (uint64_t)duracion_us / 2U) / duracion_us;
    if (redondeado > UINT32_MAX) {
        lectura->estado = MEDICION_VELOCIDAD_ERROR_RANGO;
        return lectura->estado;
    }
    lectura->rpm_milesimas = (uint32_t)redondeado;
    lectura->estado = MEDICION_VELOCIDAD_VALIDA;
    return lectura->estado;
}

/** @} */
