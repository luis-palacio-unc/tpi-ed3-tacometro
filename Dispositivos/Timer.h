/**
 * @file Timer.h
 * @brief Referencia temporal en milisegundos y ventana periodica de medicion.
 * @details Configuracion fija por compilacion; interfaz sin tipos del microcontrolador.
 * TIMER0 cuenta libre a 1 tick por TIMER_BASE_MS; MR0 interrumpe cada
 * TPI_CONTROL_PERIODO_MS sin reiniciar la cuenta.
 * @note ESW.2.1.17. Init y Start preceden a Read/ConsumirVentana; timer exclusivo.
 */
#ifndef TPI_TIMER_H
#define TPI_TIMER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup timer ESW.2.1.17 - Timer
 * @brief Inicializacion, arranque, consulta temporal y ventana de medicion.
 * @details Reloj informado correcto y periodo representable/exacto son precondiciones
 * del driver. Reconfigurar abre nueva epoca: descartar marcas anteriores.
 * @{
 */
/** @brief Milisegundos por cuenta; default1, configurable mediante opcion de compilacion. */
#ifndef TIMER_BASE_MS
#define TIMER_BASE_MS UINT32_C(1)
#endif
#if TIMER_BASE_MS<=0 || TIMER_BASE_MS>UINT32_MAX/UINT32_C(1000)
#error "TIMER_BASE_MS debe ser positivo y convertible a microsegundos uint32."
#endif

/**
 * @brief Configurar TIMER0 y la interrupcion periodica de ventana, dejandolo detenido.
 * @details Usa la biblioteca original: TIM_InitTimer(TIM_US, TIMER_BASE_MS*1000)
 * alimenta el periferico (PCONP.PCTIM0, UM10360 tabla 46), fija PCLK = CCLK/4
 * (tabla 40) y calcula PR desde SystemCoreClock (PR = 24999 con CCLK 100 MHz, 1 kHz).
 * TIM_ConfigMatch programa MR0 = TPI_CONTROL_PERIODO_MS/TIMER_BASE_MS con
 * interrupcion y sin reset ni stop (MCR = 0x001, cap. 21, tabla 430), sin accion
 * sobre pin externo. Reinicia el estado de ventana y habilita TIMER0_IRQn en el
 * NVIC con prioridad 1 (cap. 6, tabla 50).
 * @pre SystemInit ejecutado y SystemCoreClock = CCLK real. Timer exclusivo.
 * @note USW.2.1.17.1. Llamar desde el lazo privilegiado antes de Timer_Start;
 * no bloquea. Reinicializar abre una epoca nueva de marcas y ventanas.
 */
void Timer_Init(void);

/**
 * @brief Arrancar la cuenta previamente configurada mediante TIM_Enable.
 * @details Toma antes la referencia de SensorOptico_Read para que la primera
 * ventana cuente solo pulsos posteriores al arranque. No cambia el periodo.
 * @pre Timer_Init y SensorOptico_Init ejecutados; llamar una sola vez por epoca.
 * @note USW.2.1.17.14. Timer exclusivo, acceso privilegiado serializado.
 */
void Timer_Start(void);

/**
 * @brief Consultar TC mediante el driver y expresarlo en milisegundos.
 * @return Marca uint32 modular TC*TIMER_BASE_MS; cero puede ser una marca valida.
 * @pre Init y Start ejecutados, configuracion estable y acceso privilegiado serializado.
 * @note USW.2.1.17.2. No reinicia ni detiene el contador: TC corre libre porque
 * MR0 no lo reinicia. Restar marcas de la misma epoca con duracion real<2^31ms;
 * no acredita precision de reloj/suspension/debug.
 */
uint32_t Timer_Read(void);

/**
 * @brief Retirar, una sola vez, los pulsos de la ultima ventana completa.
 * @details El ISR de TIMER0 cierra una ventana cada TPI_CONTROL_PERIODO_MS (500 ms)
 * y guarda la diferencia modulo 2^32 del conteo del sensor optico. Si el lazo no
 * consulta a tiempo y se cierran dos o mas ventanas, solo se conserva la mas
 * reciente: las anteriores se descartan sin sumarse, de modo que el valor entregado
 * siempre corresponde a exactamente un periodo.
 * @param[out] pulsos Destino de los pulsos de la ventana (unidad: flancos contados
 * en TPI_CONTROL_PERIODO_MS). Solo se escribe cuando se retorna true.
 * @return true si habia una ventana nueva desde la consulta anterior; false si no
 * la habia o si pulsos es NULL (en ese caso no se consume la ventana pendiente).
 * @pre Timer_Init/Timer_Start ejecutados. Llamar solo desde el lazo principal.
 * @note USW.2.1.17.15. Sin enmascarar interrupciones: lee contador de ventanas,
 * pulsos y otra vez el contador (volatile, 32 bits), y repite si el ISR intervino;
 * como mucho una repeticion por ventana. No reentrante.
 */
bool Timer_ConsumirVentana(uint32_t *pulsos);

/** @} */
#endif
