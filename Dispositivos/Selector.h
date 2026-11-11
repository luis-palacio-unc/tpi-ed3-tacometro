/**
 * @file Selector.h
 * @brief Lectura del contacto del selector conectado a P2.6, activo por alto.
 * @details Entrada GPIO con pull-up interno y llave a GND: contacto abierto lee 1.
 * La interpretacion de modo y el antirrebote son externos.
 * @note ESW.2.1.13. Usa la biblioteca original de GPIO y PINSEL.
 */
#ifndef TPI_SELECTOR_H
#define TPI_SELECTOR_H

#include <stdbool.h>

/**
 * @brief Configurar P2.6 como entrada GPIO con pull-up interno.
 * @details PINSEL_ConfigPin selecciona funcion 00 (GPIO) en PINSEL4 (UM10360 tabla
 * 84), modo 00 = pull-up en PINMODE4 (tablas 77 y 92) y sin drenador abierto;
 * GPIO_SetDir deja el bit 6 de FIO2DIR como entrada (cap. 9, tabla 104).
 * @pre GPIO alimentado (PCONP.PCGPIO, valor de reset y de SystemInit) y FIO2MASK
 * bit 6 en 0 (reset). Llave entre P2.6 y GND.
 * @note USW.2.1.13.2. Llamar una vez desde el lazo privilegiado antes de
 * Selector_Read; no bloquea ni usa interrupciones.
 */
void Selector_Init(void);

/**
 * @brief Consultar la actividad logica del contacto mediante el driver GPIO.
 * @details Aisla P2.6 de la lectura del puerto; no modifica entradas ni otras salidas.
 * @return true si P2.6 esta alto; false si esta bajo. No representa validez ni modo.
 * @pre Selector_Init ejecutado: pin configurado como GPIO de entrada con pull-up y
 * FIOMASK habilitando bit6. Acceso privilegiado serializado y configuracion estable.
 * @note USW.2.1.13.1. Una lectura, sin esperas, antirrebote, interrupciones ni estado propio.
 */
bool Selector_Read(void);

#endif
