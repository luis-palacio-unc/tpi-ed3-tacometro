/**
 * @file Driver_Alarmas.c
 * @brief Validacion y envio coordinado de solicitudes a dos salidas abstractas.
 * @details Valida ambas funciones antes de invocar LED y luego buzzer, sin contexto
 * ni retorno de proveedor. El resultado informa solo argumento valido y despacho.
 * @note ESW.2.1.6. Sin dependencia de aplicacion, dispositivos o biblioteca del MCU.
 */
#include "Driver_Alarmas.h"
#include <stddef.h>

/** @addtogroup driver_alarmas
 * @{
 */

Driver_Alarmas_Estado Driver_Alarmas_Aplicar(const Driver_Alarmas_Salidas *salidas,
                                          bool activar)
{
    if (salidas == NULL || salidas->escribir_led == NULL || salidas->escribir_buzzer == NULL) {
        return DRIVER_ALARMAS_ERROR_ARGUMENTO;
    }
    salidas->escribir_led(activar);
    salidas->escribir_buzzer(activar);
    return DRIVER_ALARMAS_ACEPTADA;
}

/** @} */
