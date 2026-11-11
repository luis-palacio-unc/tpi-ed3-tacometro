/**
 * @file Driver_Motor.h
 * @brief Aplicacion del mando al motor, en unidades por mil.
 * @details Unico camino de la aplicacion hacia el PWM del motor.
 * @note ESW.2.1.8. Usa el dispositivo Motor.
 */
#ifndef TPI_DRIVER_MOTOR_H
#define TPI_DRIVER_MOTOR_H

#include <stdint.h>

/**
 * @defgroup driver_motor ESW.2.1.8 - Driver de motor
 * @brief Inicializacion y mando del motor.
 * @{
 */

/**
 * @brief Configurar el PWM del motor, que arranca con mando cero.
 * @details Llama Motor_Init.
 * @pre Sistema_Init ejecutado.
 * @note USW.2.1.8.3. Llamar una vez antes de Driver_Motor_Aplicar.
 */
void Driver_Motor_Init(void);

/**
 * @brief Aplicar un mando al motor.
 * @details Limita el valor a 1000 y lo entrega a Motor_Write.
 * @param[in] por_mil Mando de 0 a 1000 por mil; un valor mayor se aplica como 1000.
 * @pre Driver_Motor_Init ejecutado.
 * @note USW.2.1.8.4. No confirma el efecto electrico ni el movimiento.
 */
void Driver_Motor_Aplicar(uint16_t por_mil);

/** @} */
#endif /* TPI_DRIVER_MOTOR_H */
