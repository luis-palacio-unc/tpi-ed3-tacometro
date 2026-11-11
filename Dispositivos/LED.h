/**
 * @file LED.h
 * @brief Accionamiento logico del LED rojo integrado de OM13085.
 * @details P0.22, activo por bajo segun esquema revD; salida GPIO iniciada apagada.
 * @note ESW.2.1.14. Usa la biblioteca original de GPIO y PINSEL.
 */
#ifndef TPI_LED_H
#define TPI_LED_H

#include <stdbool.h>

/**
 * @brief Configurar P0.22 como salida GPIO con el LED apagado.
 * @details PINSEL_ConfigPin selecciona funcion 00 (GPIO) en PINSEL1 (UM10360 tabla
 * 81), sin resistencias ni drenador abierto; LED_Write(false) carga 1 en el latch
 * (FIO0SET, tabla 106) antes de que GPIO_SetDir ponga el bit 22 de FIO0DIR como
 * salida (tabla 104), para no encender el LED activo por bajo durante el cambio.
 * @pre GPIO alimentado (PCONP.PCGPIO, valor de reset y de SystemInit) y FIO0MASK
 * bit 22 en 0 (reset).
 * @note USW.2.1.14.2. Llamar una vez desde el lazo privilegiado antes de LED_Write;
 * no bloquea ni usa interrupciones.
 */
void LED_Init(void);

/**
 * @brief Solicitar encendido/apagado del LED usando GPIO_SetPinState.
 * @details Activar lleva P0.22 a bajo, desactivar a alto. El driver calcula mascara
 * y escritura SET/CLR. Sin contexto ni retorno; no diagnostica fallos electricos.
 * @param[in] activar true enciende; false apaga, sin decidir la condicion de alarma.
 * @pre LED_Init ejecutado: P0.22 como GPIO de salida y habilitado por FIOMASK;
 * revision/conexion compatibles, acceso privilegiado serializado y configuracion estable.
 * @note USW.2.1.14.1. Sin cache, reintentos, lectura GPIO ni estado mutable propio.
 */
void LED_Write(bool activar);

#endif
