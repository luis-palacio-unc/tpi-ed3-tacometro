/**
 * @file Buzzer.h
 * @brief Accionamiento logico de la interfaz de buzzer activo por alto en P2.7.
 * @details Salida GPIO iniciada en bajo; no genera tonos ni requiere PWM.
 * @note ESW.2.1.15. Usa la biblioteca original de GPIO y PINSEL.
 */
#ifndef TPI_BUZZER_H
#define TPI_BUZZER_H

#include <stdbool.h>

/**
 * @brief Configurar P2.7 como salida GPIO con el buzzer desactivado.
 * @details PINSEL_ConfigPin selecciona funcion 00 (GPIO) en PINSEL4 (UM10360 tabla
 * 84), sin resistencias ni drenador abierto; Buzzer_Write(false) carga 0 en el
 * latch (FIO2CLR, tabla 108) antes de que GPIO_SetDir ponga el bit 7 de FIO2DIR
 * como salida (tabla 104). Hasta esta llamada P2.7 es entrada con pull-up de reset.
 * @pre GPIO alimentado (PCONP.PCGPIO, valor de reset y de SystemInit) y FIO2MASK
 * bit 7 en 0 (reset).
 * @note USW.2.1.15.2. Llamar una vez desde el lazo privilegiado antes de
 * Buzzer_Write; no bloquea ni usa interrupciones.
 */
void Buzzer_Init(void);

/**
 * @brief Solicitar activacion/desactivacion del buzzer mediante GPIO_SetPinState.
 * @details Activar lleva P2.7 a alto, desactivar a bajo. Delega mascara/escritura al
 * driver original. Sin contexto ni retorno; no confirma sonido ni estado electrico.
 * @param[in] activar true activa la interfaz del buzzer; false la desactiva.
 * @pre Buzzer_Init ejecutado: P2.7 como GPIO de salida y habilitado por FIOMASK;
 * acceso privilegiado serializado y configuracion estable, sin escritores concurrentes.
 * @note USW.2.1.15.1. Sin cache, reintentos, lectura GPIO o estado mutable propio.
 */
void Buzzer_Write(bool activar);

#endif
