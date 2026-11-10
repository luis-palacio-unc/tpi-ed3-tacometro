/**
 * @file Buzzer.h
 * @brief Accionamiento logico de la interfaz de buzzer activo por alto en P2.7.
 * @details Emplea GPIO preconfigurado; no genera tonos ni requiere PWM.
 * @note ESW.2.1.15. Inicializacion e integracion fisica quedan para el final.
 */
#ifndef TPI_BUZZER_H
#define TPI_BUZZER_H

#include <stdbool.h>

/**
 * @brief Solicitar activacion/desactivacion del buzzer mediante GPIO_SetPinState.
 * @details Activar lleva P2.7 a alto, desactivar a bajo. Delega mascara/escritura al
 * driver original. Sin contexto ni retorno; no confirma sonido ni estado electrico.
 * @param[in] activar true activa la interfaz del buzzer; false la desactiva.
 * @pre P2.7 previamente configurado como GPIO de salida y habilitado por FIOMASK;
 * acceso privilegiado serializado y configuracion estable, sin escritores concurrentes.
 * @note USW.2.1.15.1. Sin cache, reintentos, lectura GPIO o estado mutable propio.
 */
void Buzzer_Write(bool activar);

#endif
