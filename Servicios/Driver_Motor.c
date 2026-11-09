/**
 * @file Driver_Motor.c
 * @brief Validacion del mando por mil y envio a una funcion de salida externa.
 * @details Rechaza argumentos/rango antes de invocar; traduce aceptacion del proveedor.
 * La operacion explicita de cero reutiliza el mismo contrato de envio y errores.
 * @note ESW.2.1.8. Sin dependencia de aplicacion, dispositivos, reloj o PWM concreto.
 */
#include "Driver_Motor.h"
#include <stddef.h>

/** @addtogroup driver_motor
 * @{
 */

Driver_Motor_Estado Driver_Motor_Aplicar(const Driver_Motor_Salida *salida,
                                      uint16_t mando_por_mil)
{
    if (salida == NULL || salida->escribir == NULL) {
        return DRIVER_MOTOR_ERROR_ARGUMENTO;
    }
    if (mando_por_mil > DRIVER_MOTOR_MANDO_MAX) {
        return DRIVER_MOTOR_ERROR_RANGO;
    }
    return salida->escribir(salida->contexto, mando_por_mil)
        ? DRIVER_MOTOR_ACEPTADA : DRIVER_MOTOR_ERROR_SALIDA;
}

Driver_Motor_Estado Driver_Motor_SolicitarCero(const Driver_Motor_Salida *salida)
{
    return Driver_Motor_Aplicar(salida, 0U);
}

/** @} */
