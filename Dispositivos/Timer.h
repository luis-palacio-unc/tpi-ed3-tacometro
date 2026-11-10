/**
 * @file Timer.h
 * @brief Referencia temporal en milisegundos mediante el driver existente.
 * @details Configuracion fija por compilacion; interfaz sin tipos del microcontrolador.
 * @note ESW.2.1.17. Init y Start preceden a Read; uso serializado y timer exclusivo.
 */
#ifndef TPI_TIMER_H
#define TPI_TIMER_H

#include <stdint.h>

/**
 * @defgroup timer ESW.2.1.17 - Timer
 * @brief Wrappers de inicializacion, arranque y consulta temporal.
 * @details Reloj informado correcto y periodo representable/exacto son precondiciones
 * del driver. Reconfigurar abre nueva epoca: descartar marcas anteriores.
 * @{
 */
/** @brief Milisegundos por cuenta; default1, configurable mediante opcion de compilacion. */
#ifndef TIMER_BASE_MS
#define TIMER_BASE_MS UINT32_C(1)
#endif
_Static_assert(TIMER_BASE_MS>0 && TIMER_BASE_MS<=UINT32_MAX/UINT32_C(1000),
               "TIMER_BASE_MS debe ser positivo y convertible a microsegundos uint32.");

/**
 * @brief Configurar el timer con el periodo elegido, dejandolo detenido.
 * @details Delega alimentacion, reloj y prescaler en TIM_InitTimer. Requiere timer
 * exclusivo desde reset, sin match/captura previos, y SystemCoreClock correcto.
 * @note USW.2.1.17.1. Configuracion desde lazo privilegiado serializado.
 */
void Timer_Init(void);

/**
 * @brief Arrancar la cuenta previamente configurada mediante TIM_Enable.
 * @details Llamar despues de Timer_Init; no reinicializa ni cambia el periodo.
 * @note USW.2.1.17.14. Timer exclusivo, acceso privilegiado serializado.
 */
void Timer_Start(void);

/**
 * @brief Consultar TC mediante el driver y expresarlo en milisegundos.
 * @return Marca uint32 modular TC*TIMER_BASE_MS; cero puede ser una marca valida.
 * @pre Init y Start ejecutados, configuracion estable y acceso privilegiado serializado.
 * @note USW.2.1.17.2. No reinicia ni detiene el contador. Restar marcas de la misma
 * epoca con duracion real<2^31ms; no acredita precision de reloj/suspension/debug.
 */
uint32_t Timer_Read(void);

/** @} */
#endif
