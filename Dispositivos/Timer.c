/**
 * @file Timer.c
 * @brief TIMER0 como base de tiempos libre y ventana periodica de medicion.
 * @details El driver original configura prescaler, match e interrupcion; el ISR
 * toma el conteo del sensor optico y publica los pulsos de cada ventana.
 * @note ESW.2.1.17. No modifica la biblioteca; sin acceso directo a registros.
 */
#include "Timer.h"
#include "Control.h"
#include "Sensor_Optico.h"
#include "lpc17xx_timer.h"
#include <stddef.h>

#if (TPI_CONTROL_PERIODO_MS % TIMER_BASE_MS) != 0
#error "TPI_CONTROL_PERIODO_MS debe ser multiplo de TIMER_BASE_MS."
#endif

/** @addtogroup timer
 * @{
 */

/** @brief Cuentas de TC por ventana: 500 con base de 1 ms. */
#define TIMER_TICKS_VENTANA (TPI_CONTROL_PERIODO_MS / TIMER_BASE_MS)
/** @brief Prioridad NVIC de TIMER0, la mas alta de las tres ISR propias. */
#define TIMER_PRIORIDAD_IRQ 1U

/** @brief Proximo valor de MR0 en cuentas, modulo 2^32; solo Init y el ISR lo escriben. */
static volatile uint32_t Timer_ProximoMatch;
/** @brief Conteo del sensor al cerrar la ventana anterior (o al arrancar). */
static volatile uint32_t Timer_ConteoAnterior;
/** @brief Pulsos de la ultima ventana cerrada; escrito por el ISR. */
static volatile uint32_t Timer_PulsosVentana;
/** @brief Ventanas cerradas desde Init, modulo 2^32; escrito por el ISR tras los pulsos. */
static volatile uint32_t Timer_VentanasCerradas;
/** @brief Valor de Timer_VentanasCerradas ya entregado; solo lo usa el lazo principal. */
static uint32_t Timer_VentanasConsumidas;

void Timer_Init(void)
{
    const TIM_TIMERCFG_T config = {TIM_US, TIMER_BASE_MS * UINT32_C(1000)};
    const TIM_MATCHCFG_T ventana = {
        .channel = TIM_MATCH_0,
        .intEn = ENABLE,
        .stopEn = DISABLE,
        .resetEn = DISABLE,
        .extOpt = TIM_NOTHING,
        .matchValue = TIMER_TICKS_VENTANA
    };
    NVIC_DisableIRQ(TIMER0_IRQn);
    TIM_InitTimer(LPC_TIM0, &config);
    TIM_ConfigMatch(LPC_TIM0, &ventana);
    Timer_ProximoMatch = TIMER_TICKS_VENTANA;
    Timer_ConteoAnterior = 0U;
    Timer_PulsosVentana = 0U;
    Timer_VentanasCerradas = 0U;
    Timer_VentanasConsumidas = 0U;
    NVIC_SetPriority(TIMER0_IRQn, TIMER_PRIORIDAD_IRQ);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

void Timer_Start(void)
{
    Timer_ConteoAnterior = SensorOptico_Read();
    TIM_Enable(LPC_TIM0);
}

uint32_t Timer_Read(void)
{
    return TIM_ReadTimer(LPC_TIM0) * TIMER_BASE_MS;
}

bool Timer_ConsumirVentana(uint32_t *pulsos)
{
    if (pulsos == NULL) {
        return false;
    }
    uint32_t cerradas;
    uint32_t valor;
    do {
        cerradas = Timer_VentanasCerradas;
        valor = Timer_PulsosVentana;
    } while (cerradas != Timer_VentanasCerradas);
    if (cerradas == Timer_VentanasConsumidas) {
        return false;
    }
    Timer_VentanasConsumidas = cerradas;
    *pulsos = valor;
    return true;
}

/**
 * @brief ISR de TIMER0: cerrar la ventana de medicion en cada match de MR0.
 * @details Si el flag MR0 de T0IR esta activo (UM10360 tabla 427): toma
 * SensorOptico_Read, limpia el flag con TIM_ClearIntPending, reprograma MR0 a
 * MR0 + TPI_CONTROL_PERIODO_MS/TIMER_BASE_MS (modulo 2^32, igual que TC) con
 * TIM_UpdateMatchValue, y publica pulsos = conteo - conteo anterior (modulo 2^32)
 * antes de incrementar el contador de ventanas cerradas.
 * Una ventana no consumida se sobrescribe. Otras fuentes de T0IR no estan habilitadas.
 * @note USW.2.1.17.16. Nombre exigido por la tabla de vectores. Prioridad 1; sin
 * esperas ni lazos. Comparte Timer_PulsosVentana/Timer_VentanasCerradas (volatile)
 * con Timer_ConsumirVentana, que detecta una actualizacion concurrente releyendo
 * el contador.
 */
void TIMER0_IRQHandler(void)
{
    if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT) == SET) {
        const uint32_t conteo = SensorOptico_Read();
        const uint32_t proximo = Timer_ProximoMatch + TIMER_TICKS_VENTANA;
        TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
        TIM_UpdateMatchValue(LPC_TIM0, TIM_MATCH_0, proximo);
        Timer_ProximoMatch = proximo;
        Timer_PulsosVentana = conteo - Timer_ConteoAnterior;
        Timer_ConteoAnterior = conteo;
        Timer_VentanasCerradas = Timer_VentanasCerradas + 1U;
    }
}

/** @} */
