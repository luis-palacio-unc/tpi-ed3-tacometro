/**
 * @file Sensor_Optico.h
 * @brief Conteo acumulado de eventos del sensor optico mediante TIMER1.
 * @details Entrada CAP1.0 en P1.18, funcion11 de PINSEL3, contador de flancos
 * ascendentes. El sensor XD-51 del TPI no altera esta interfaz.
 * @note ESW.2.1.10. Las ventanas de medicion las cierra Timer (ESW.2.1.17).
 */
#ifndef TPI_SENSOR_OPTICO_H
#define TPI_SENSOR_OPTICO_H

#include <stdint.h>

/**
 * @brief Configurar y arrancar TIMER1 como contador de flancos de CAP1.0/P1.18.
 * @details Con la biblioteca original y en este orden: TIM_InitCounter alimenta
 * TIMER1 (PCONP.PCTIM1, UM10360 tabla 46), detiene y reinicia el contador y escribe
 * T1CTCR = 0x01: modo contador en flancos ascendentes de CAP1.0 (cap. 21, tabla
 * 429). PINSEL_ConfigPin asigna P1.18 = CAP1.0, funcion 11 de PINSEL3 (tabla 83),
 * con pull-up interno (PINMODE3 = 00, tablas 77 y 91) porque la salida del modulo
 * optico suele ser un comparador a colector abierto, y sin drenador abierto.
 * TIM_Enable arranca la cuenta (T1TCR = 0x01, tabla 428).
 * PR, MCR y CCR conservan su valor de reset (0): un flanco por cuenta, sin match
 * ni captura que alteren TC, como exige la tabla 429 para el canal seleccionado.
 * @pre TIMER1 exclusivo y en estado de reset; SystemInit ejecutado. La senal debe
 * respetar 1/(2 PCLK) por nivel y frecuencia menor que PCLK/4 (seccion 21.6.3).
 * @note USW.2.1.10.2. Llamar una vez desde el lazo privilegiado, antes de
 * Timer_Start; no bloquea ni usa interrupciones. Abre una epoca nueva de conteo.
 */
void SensorOptico_Init(void);

/**
 * @brief Consultar el conteo de TIMER1 mediante TIM_ReadTimer original.
 * @details Entrega eventos acumulados, sin convertir a tiempo/RPM ni aplicar
 * factor de ranuras o limite de consignas. No reinicia/detiene el contador.
 * @return Conteo uint32_t modulo2^32; cero es valido, no un estado de error.
 * El consumidor resta muestras de la misma epoca con menos de2^32 eventos entre
 * ellas; tras reinicio/reconfiguracion debe adquirir una referencia nueva.
 * @pre SensorOptico_Init ejecutado: TIMER1 exclusivo y arrancado como contador de flancos
 * ascendentes CAP1.0/P1.18, senal compatible. Bits CCR del canal en000 y ningun
 * match/captura que reinicie/detenga TC; configuracion estable y acceso privilegiado
 * desde el lazo o desde el ISR de TIMER0. TIMER0 conserva la base temporal de B11.
 * @note USW.2.1.10.1. Sin contexto/estado propio/ISR/heap/esperas; no detecta
 * parada, vueltas completas omitidas, fallos electricos ni valida tiempos reales.
 */
uint32_t SensorOptico_Read(void);

#endif /* TPI_SENSOR_OPTICO_H */
