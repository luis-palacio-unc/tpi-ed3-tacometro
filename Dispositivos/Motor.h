/**
 * @file Motor.h
 * @brief Actualizacion del mando del motor en PWM1.1/P2.0.
 * @details Convierte mando por mil a ticks para PWM single-edge activo alto.
 * Configuracion de pin, reloj, periodo y arranque se integraran al final.
 * @note ESW.2.1.9. Usa el driver original; desconoce modos, RPM y regulacion.
 */
#ifndef TPI_MOTOR_H
#define TPI_MOTOR_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Validar, convertir y solicitar un nuevo ciclo de trabajo.
 * @details Redondea al tick mas cercano, empate hacia arriba. Cero pide bajo
 * continuo; al redondear al periodo completo usa match=periodo+1 para alto
 * continuo. Actualiza solo MR1 mediante PWM_MatchUpdateSingle, una vez por
 * solicitud valida, incluidas repeticiones. No altera MR0 ni la frecuencia.
 * @param[in] mando_por_mil Mando uint16_t de 0..1000 inclusive (0..100 por ciento).
 * Validar origenes mas amplios/con signo antes de convertir a este tipo.
 * @param[in] periodo_ticks Periodo activo uint32_t en ticks, 1..UINT32_MAX-1.
 * Debe coincidir con MR0 efectivo y estable, sin actualizacion pendiente.
 * @return false si mando>1000 o periodo es 0/UINT32_MAX, sin llamar al driver
 * ni solicitar cero automaticamente. true si se despacho la orden valida;
 * no confirma nivel electrico, velocidad, detencion ni ausencia de fallo fisico.
 * @pre PWM1 exclusivo y preconfigurado/arrancado, PWM1.1 en P2.0 funcion01,
 * salida single-edge activa alta, reset por MR0 y sin otros matches que
 * reinicien/detengan TC. Parametros estables y acceso privilegiado serializado.
 * @note USW.2.1.9.1. Error de cuantizacion maximo medio tick. Sin contexto,
 * estado propio, ISR, heap, inicializacion, reinicio o espera. El driver aplica
 * el latch al proximo ciclo: varias ordenes previas pueden dejar solo la ultima.
 * Una orden valida posterior recupera tras rechazo; no hay reintentos.
 */
bool Motor_Write(uint16_t mando_por_mil, uint32_t periodo_ticks);

#endif /* TPI_MOTOR_H */
