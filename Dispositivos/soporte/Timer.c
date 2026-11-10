/**
 * @file Timer.c
 * @brief Validacion de periodo/PCLK y lectura normalizada del contador hardware.
 * @details Solo guarda periodo/estado, nunca acumula tiempo. Configuracion usa
 * uint64_t acotado para admitir PR=UINT32_MAX sin overflow intermedio.
 * @note ESW.2.1.17. No incluye ni enlaza servicios/aplicacion; no ISR ni esperas.
 */
#include "Timer.h"
#include "lpc17xx_timer.h"
#include <stddef.h>
_Static_assert(offsetof(LPC_TIM_TypeDef,TC)==8U,"Revisar registro TC del LPC1769.");
_Static_assert(TIMER0_IRQn==1,"Revisar IRQ exclusiva TIMER0.");
/** @addtogroup base_tiempos
 * @{
 */
/** @brief Periodo vigente ms, cero si sin configurar/error; solo lazo serializado. */
static uint32_t periodo_base_ms;
/** @brief Estado unico, SIN_DATOS por inicializacion estatica; nunca mutado por ISR. */
static Timer_Estado estado_base;

Timer_Estado Timer_Init(uint32_t frecuencia_pclk_hz,uint32_t periodo_ms)
{
    uint32_t prescaler=0U;
    periodo_base_ms=0U;
    estado_base=TIMER_ERROR_RELOJ;
    if(frecuencia_pclk_hz>0U && frecuencia_pclk_hz<=TIMER_PCLK_MAX_HZ) {
        estado_base=TIMER_ERROR_PERIODO;
        if(periodo_ms>0U && periodo_ms<=TIMER_PERIODO_MAX_MS) {
            estado_base=TIMER_ERROR_PRESCALER;
            const uint64_t producto=(uint64_t)frecuencia_pclk_hz*periodo_ms;
            const uint64_t divisor=producto/UINT64_C(1000);
            if(producto%UINT64_C(1000)==0U && divisor>0U && divisor<=UINT64_C(0x100000000)) {
                prescaler=(uint32_t)(divisor-1U);
                periodo_base_ms=periodo_ms;
                estado_base=TIMER_ACTIVA;
            }
        }
    }
    NVIC_DisableIRQ(TIMER0_IRQn);
    TIM_InitFreeRunning(LPC_TIM0,prescaler);
    NVIC_ClearPendingIRQ(TIMER0_IRQn);
    __DSB();
    if(estado_base==TIMER_ACTIVA) {
        TIM_Enable(LPC_TIM0);
        __DSB();
    }
    return estado_base;
}

Timer_Estado Timer_Read(Timer_Lectura *lectura)
{
    if(lectura==NULL) {
        return TIMER_ERROR_ARGUMENTO;
    }
    lectura->milisegundos=0U;
    if(estado_base==TIMER_ACTIVA || estado_base==TIMER_DETENIDA) {
        const uint32_t conteos=TIM_ReadTimer(LPC_TIM0);
        lectura->milisegundos=(uint32_t)(conteos*periodo_base_ms);
    }
    lectura->estado=estado_base;
    return estado_base;
}

Timer_Estado Timer_Stop(void)
{
    if(estado_base==TIMER_ACTIVA) {
        TIM_Disable(LPC_TIM0);
        __DSB();
        estado_base=TIMER_DETENIDA;
    }
    return estado_base;
}

bool Timer_Source(void *contexto,uint32_t *milisegundos)
{
    (void)contexto;
    if(milisegundos==NULL) {
        return false;
    }
    *milisegundos=0U;
    Timer_Lectura lectura;
    if(Timer_Read(&lectura)!=TIMER_ACTIVA) {
        return false;
    }
    *milisegundos=lectura.milisegundos;
    return true;
}
/** @} */
