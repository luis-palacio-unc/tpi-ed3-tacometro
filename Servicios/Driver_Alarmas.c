/**
 * @file Driver_Alarmas.c
 * @brief Validacion y envio coordinado de solicitudes a dos salidas abstractas.
 * @details Funciones invocadas en instrucciones separadas para evitar cortocircuito
 * de la segunda salida ante fallo de la primera. Resultado de esta llamada solamente.
 * @note ESW.2.1.6. Proveedores externos; sin dependencia de aplicacion o dispositivos.
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
    const bool led_ok = salidas->escribir_led(salidas->contexto_led, activar);
    const bool buzzer_ok = salidas->escribir_buzzer(salidas->contexto_buzzer, activar);
    if (!led_ok && !buzzer_ok) {
        return DRIVER_ALARMAS_ERROR_AMBOS;
    }
    if (!led_ok) {
        return DRIVER_ALARMAS_ERROR_LED;
    }
    if (!buzzer_ok) {
        return DRIVER_ALARMAS_ERROR_BUZZER;
    }
    return DRIVER_ALARMAS_ACEPTADA;
}

/** @} */
