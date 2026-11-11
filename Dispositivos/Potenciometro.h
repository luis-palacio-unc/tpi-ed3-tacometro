/**
 * @file Potenciometro.h
 * @brief Consulta de la muestra del potenciometro almacenada por ADC/DMA.
 * @details Potenciometro.c posee la muestra y expone el resultado ADC de
 * 12 bits; Mandos realiza la normalizacion.
 * @note ESW.2.1.12. AD0.0/P0.23; configuracion y arranque diferidos.
 */
#ifndef TPI_POTENCIOMETRO_H
#define TPI_POTENCIOMETRO_H

#include <stdint.h>

/**
 * @brief Devuelve el resultado actual del potenciometro desde memoria.
 * @return Muestra ADC sin signo, de 0 a 4095 cuentas inclusive.
 * @pre El soporte debe destinar a ADC_DMA_Pot la palabra ADGDR de AD0.0,
 * con transferencias DMA de 32 bits alineadas y sin escritores CPU concurrentes.
 * @note USW.2.1.12.1. Una lectura volatile. No valida flags ni modifica la muestra;
 * tampoco inicia adquisicion, espera o conserva cache. Cero no indica disponibilidad.
 * Puede devolver la muestra anterior o la nueva durante una transferencia;
 * no garantiza antiguedad ni sincronizacion entre distintos canales.
 */
uint16_t Pot_Read(void);

#endif /* TPI_POTENCIOMETRO_H */
