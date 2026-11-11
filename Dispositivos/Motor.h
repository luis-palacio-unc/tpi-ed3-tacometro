/**
 * @file Motor.h
 * @brief Mando directo del motor mediante PWM1.1/P2.0 con MR0 fijo en 1000.
 * @details Configuracion de PWM, reloj y arranque se integraran al final.
 * @note ESW.2.1.9. Usa la biblioteca original sin conocer modos, RPM o regulacion.
 */
#ifndef TPI_MOTOR_H
#define TPI_MOTOR_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Validar y enviar el mando exacto al match de salida.
 * @details Con MR0=1000, cada unidad por mil corresponde a un tick de MR1.
 * Llama una vez PWM_MatchUpdateSingle(PWM_MATCH_1,mando_por_mil) por solicitud
 * valida, incluidas repeticiones. Envia 0 y 1000 sin ajustes especiales.
 * No calcula match, convierte, lee/configura MR0 ni cambia la frecuencia.
 * @param[in] mando_por_mil Mando uint16_t de 0..1000 inclusive, 0..100 por ciento.
 * Validar origenes mas amplios/con signo antes de convertir al tipo del parametro.
 * @return false si mando>1000, sin llamar al driver ni emitir cero. true si se
 * despacho la orden valida; no confirma efecto electrico, movimiento o detencion.
 * @pre PWM1 accesible/exclusivo/configurado/arrancado; PWM1.1 en P2.0 funcion01,
 * single-edge activo alto y MR0=1000 efectivo/estable. Reset por MR0 y sin otros
 * matches que alteren TC. Acceso privilegiado serializado, sin update pendiente.
 * @note USW.2.1.9.1. Sin estado/contexto/cache/init/ISR/heap/esperas/reintentos.
 * El latch se aplica al proximo ciclo; varias ordenes pueden dejar solo la ultima.
 * Revisar errata PWM.1 al integrar; aceptacion logica no valida transitorios fisicos.
 */
bool Motor_Write(uint16_t mando_por_mil);

#endif /* TPI_MOTOR_H */
