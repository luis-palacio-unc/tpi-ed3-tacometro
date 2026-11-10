/**
 * @file LED.h
 * @brief Accionamiento logico del LED rojo integrado de OM13085.
 * @details P0.22, activo por bajo segun esquema revD; operacion GPIO preconfigurada.
 * @note ESW.2.1.14. Configuracion fisica e integracion del arranque diferidas.
 */
#ifndef TPI_LED_H
#define TPI_LED_H

#include <stdbool.h>

/**
 * @brief Solicitar encendido/apagado del LED usando GPIO_SetPinState.
 * @details Activar lleva P0.22 a bajo, desactivar a alto. El driver calcula mascara
 * y escritura SET/CLR. Sin contexto ni retorno; no diagnostica fallos electricos.
 * @param[in] activar true enciende; false apaga, sin decidir la condicion de alarma.
 * @pre P0.22 previamente configurado como GPIO de salida y habilitado por FIOMASK;
 * revision/conexion compatibles, acceso privilegiado serializado y configuracion estable.
 * @note USW.2.1.14.1. Sin cache, reintentos, lectura GPIO ni estado mutable propio.
 */
void LED_Write(bool activar);

#endif
