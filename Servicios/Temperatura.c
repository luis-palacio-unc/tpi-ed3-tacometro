/**
 * @file Temperatura.c
 * @brief Disponibilidad y errores genericos de una temperatura fisica.
 * @details La conversion especifica pertenece al proveedor. El servicio solo
 * conserva Celsius/validez o invalida un dato anterior cuando la fuente falla.
 * @note ESW.2.1.4. Sin dependencia de dispositivos concretos ni estado global mutable.
 */
#include "Temperatura.h"
#include <stddef.h>

/** @addtogroup temperatura
 * @{
 */

bool Temperatura_Inicializar(Temperatura_Lectura *lectura)
{
    if (lectura == NULL) {
        return false;
    }
    lectura->centesimas_c = 0;
    lectura->estado = TEMPERATURA_SIN_DATOS;
    return true;
}

Temperatura_Estado Temperatura_Actualizar(Temperatura_Lectura *lectura,
                                        int32_t centesimas_c,
                                        Temperatura_EstadoFuente fuente)
{
    if (lectura == NULL) {
        return TEMPERATURA_ERROR_ARGUMENTO;
    }
    lectura->centesimas_c = 0;
    switch (fuente) {
        case TEMPERATURA_FUENTE_VALIDA:
            lectura->centesimas_c = centesimas_c;
            lectura->estado = TEMPERATURA_VALIDA;
            break;
        case TEMPERATURA_FUENTE_SIN_DATOS:
            lectura->estado = TEMPERATURA_SIN_DATOS;
            break;
        case TEMPERATURA_FUENTE_ERROR:
            lectura->estado = TEMPERATURA_ERROR_FUENTE;
            break;
        default:
            lectura->estado = TEMPERATURA_ERROR_ARGUMENTO;
            break;
    }
    return lectura->estado;
}

/** @} */
