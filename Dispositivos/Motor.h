/**
 * @file Motor.h
 * @brief Mando directo del motor mediante PWM1.1/P2.0 con MR0 fijo en 1000.
 * @details Motor_Init configura PWM1 a 1 kHz con resolucion de 1000 pasos;
 * Motor_Write solo actualiza el match de salida.
 * @note ESW.2.1.9. Usa la biblioteca original sin conocer modos, RPM o regulacion.
 */
#ifndef TPI_MOTOR_H
#define TPI_MOTOR_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Configurar y arrancar PWM1.1 en P2.0 a 1 kHz con mando inicial cero.
 * @details Con la biblioteca original y en este orden: PWM_InitTimer(PWM_TICK, 25)
 * alimenta PWM1 (PCONP.PCPWM1, UM10360 tabla 46), fija PCLK_PWM1 = CCLK/4
 * (tabla 40), detiene y reinicia el contador y deja PR = 24: con CCLK 100 MHz el
 * contador avanza a 1 MHz. PWM_ConfigMatch programa MR0 = 1000 con reset del
 * contador y sin interrupcion (PWM1MCR = 0x002, tabla 450): periodo de 1 ms, 1 kHz.
 * PWM_ConfigMatch programa MR1 = 0 sin acciones: salida continuamente baja (cap. 24,
 * seccion 24.4.1, regla 1). Ambas escrituras activan su bit de PWM1LER (tabla 453).
 * PWM_ChannelConfig deja el canal 1 en flanco simple (unico modo de PWM1.1, tabla
 * 444); PWM_PinConfig asigna P2.0 = PWM1.1, funcion 01 de PINSEL4 (tabla 84), sin
 * resistencias; PWM_ChannelEnable habilita la salida (PWM1PCR.PWMENA1, tabla 452);
 * PWM_CounterEnable y PWM_Enable arrancan contador y modo PWM (PWM1TCR = 0x09,
 * tabla 448).
 * @pre SystemInit ejecutado con CCLK = 100 MHz; PWM1 exclusivo. Hasta esta llamada
 * P2.0 es entrada GPIO con pull-up de reset: la etapa de potencia debe tolerarlo.
 * @note USW.2.1.9.2. Llamar una vez desde el lazo privilegiado antes de Motor_Write;
 * no bloquea ni usa interrupciones. Revisar errata PWM.1 con mando 1000 en placa.
 */
void Motor_Init(void);

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
 * @pre Motor_Init ejecutado: PWM1 exclusivo/configurado/arrancado; PWM1.1 en P2.0 funcion01,
 * single-edge activo alto y MR0=1000 efectivo/estable. Reset por MR0 y sin otros
 * matches que alteren TC. Acceso privilegiado serializado, sin update pendiente.
 * @note USW.2.1.9.1. Sin estado/contexto/cache/ISR/heap/esperas/reintentos.
 * El latch se aplica al proximo ciclo; varias ordenes pueden dejar solo la ultima.
 * Revisar errata PWM.1 al integrar; aceptacion logica no valida transitorios fisicos.
 */
bool Motor_Write(uint16_t mando_por_mil);

#endif /* TPI_MOTOR_H */
