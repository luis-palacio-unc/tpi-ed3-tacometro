/**
 * @file Timer.c
 * @brief Tres wrappers del driver original de TIMER0 para la referencia temporal.
 * @details El driver configura el prescaler y accede a TC; salida normalizada a ms.
 * @note ESW.2.1.17. Sin ISR ni contador software; no modifica la biblioteca.
 */
#include "Timer.h"
#include "lpc17xx_timer.h"

void Timer_Init(void)
{
    const TIM_TIMERCFG_T config={TIM_US,TIMER_BASE_MS*UINT32_C(1000)};
    TIM_InitTimer(LPC_TIM0,&config);
}

void Timer_Start(void)
{
    TIM_Enable(LPC_TIM0);
}

uint32_t Timer_Read(void)
{
    return TIM_ReadTimer(LPC_TIM0)*TIMER_BASE_MS;
}
