/**
 * @file Mandos.h
 * @brief Mandos del operador: perilla de velocidad manual y llave de modo.
 * @details Convierte el potenciometro en un mando por mil y el selector en un modo.
 * @note ESW.2.1.5. Usa los dispositivos Potenciometro y Selector.
 */
#ifndef TPI_MANDOS_H
#define TPI_MANDOS_H

#include <stdint.h>

/**
 * @defgroup mandos ESW.2.1.5 - Mandos
 * @brief Lectura del mando manual y del modo de operacion.
 * @{
 */

/** @brief Modo de operacion pedido con la llave selectora. */
typedef enum {
    MANDOS_MODO_MANUAL = 0, /**< El potenciometro fija el PWM. */
    MANDOS_MODO_AUTOMATICO = 1 /**< La consigna recibida por UART fija las RPM. */
} Mandos_Modo;

/**
 * @brief Configurar la entrada de la llave selectora.
 * @details Llama Selector_Init. El potenciometro no requiere inicializacion
 * propia: lo adquiere el soporte ADC/DMA que arranca Sistema_Init.
 * @pre Sistema_Init ejecutado.
 * @note USW.2.1.5.6. Llamar una vez antes de Mandos_LeerModo.
 */
void Mandos_Init(void);

/**
 * @brief Leer la perilla como mando del motor.
 * @details Convierte la muestra de Pot_Read (0..4095) con redondeo al cercano:
 * (muestra * 1000 + 2047) / 4095. Asi 0 da 0, 2048 da 500 y 4095 da 1000.
 * @return Mando de 0 a 1000 por mil (0 a 100 por ciento de PWM).
 * @pre Sistema_Init devolvio true: ya hay una conversion real del potenciometro.
 * @note USW.2.1.5.7. Sin filtro ni estado; cada llamada lee la muestra vigente.
 */
uint16_t Mandos_LeerManual(void);

/**
 * @brief Leer la llave selectora como modo de operacion.
 * @details La llave une P2.6 con GND y el pin tiene pull-up: abierta (nivel alto)
 * es MANUAL y cerrada (nivel bajo) es AUTOMATICO.
 * @return MANDOS_MODO_MANUAL o MANDOS_MODO_AUTOMATICO.
 * @pre Mandos_Init ejecutado.
 * @note USW.2.1.5.8. Sin antirrebote propio: la aplicacion la consulta una vez por
 * periodo de 500 ms, mucho mayor que el rebote de un contacto.
 */
Mandos_Modo Mandos_LeerModo(void);

/** @} */
#endif /* TPI_MANDOS_H */
