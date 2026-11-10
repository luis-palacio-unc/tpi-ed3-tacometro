/**
 * @file Selector.h
 * @brief Lectura del contacto del selector conectado a P2.6, activo por alto.
 * @details Usa GPIO existente; la interpretacion de modo y el antirrebote son externos.
 * @note ESW.2.1.13. La configuracion del periferico se realizara al integrar el equipo.
 */
#ifndef TPI_SELECTOR_H
#define TPI_SELECTOR_H

#include <stdbool.h>

/**
 * @brief Consultar la actividad logica del contacto mediante el driver GPIO.
 * @details Aisla P2.6 de la lectura del puerto; no modifica entradas ni otras salidas.
 * @return true si P2.6 esta alto; false si esta bajo. No representa validez ni modo.
 * @pre Pin previamente configurado como GPIO de entrada, polarizacion correcta y
 * FIOMASK habilitando bit6. Acceso privilegiado serializado y configuracion estable.
 * @note USW.2.1.13.1. Una lectura, sin esperas, antirrebote, interrupciones ni estado propio.
 */
bool Selector_Read(void);

#endif
