/**
 * @file Base_Tiempos_LPC1769.c
 * @brief Accesos TIMER0/PCONP/NVIC confinados a la capa de dispositivos.
 * @details UM10360 21: timer continuo por PCLK y PR, sin match/IRQ ni contador software.
 * Conserva PCLKSEL por errata PCLKSELx.1; frecuencia efectiva es precondicion externa.
 * @note ESW.2.1.17. No configura PLL/flash/pines ni accede a SysTick.
 */
#include "Base_Tiempos_Puerto.h"
#include "LPC17xx.h"
#include <stddef.h>
/** @addtogroup base_tiempos
 * @{
 */
/** @brief PCONP bit1 alimenta TIMER0; otros bits conservados. */
#define BASE_TIEMPOS_PCTIM0 UINT32_C(2)
/** @brief Seis flags propios IR (MR0..MR3/CR0..CR1), escritura uno para limpiar. */
#define BASE_TIEMPOS_IR_LIMPIAR UINT32_C(0x3F)
_Static_assert(offsetof(LPC_TIM_TypeDef,TC)==8U,"Revisar registro TC del LPC1769.");
_Static_assert(TIMER0_IRQn==1,"Revisar IRQ exclusiva TIMER0.");

void Base_Tiempos_Puerto_Preparar(void)
{
    LPC_SC->PCONP|=BASE_TIEMPOS_PCTIM0;
    LPC_TIM0->TCR=0U;
    NVIC_DisableIRQ(TIMER0_IRQn);
    NVIC_ClearPendingIRQ(TIMER0_IRQn);
    LPC_TIM0->TCR=2U;
    LPC_TIM0->TC=0U;
    LPC_TIM0->PC=0U;
    LPC_TIM0->CTCR=0U;
    LPC_TIM0->MCR=0U;
    LPC_TIM0->CCR=0U;
    LPC_TIM0->EMR=0U;
    LPC_TIM0->IR=BASE_TIEMPOS_IR_LIMPIAR;
    __DSB();
}

void Base_Tiempos_Puerto_Iniciar(uint32_t prescaler)
{
    LPC_TIM0->PR=prescaler;
    LPC_TIM0->TCR=1U;
    __DSB();
}

uint32_t Base_Tiempos_Puerto_LeerTC(void)
{
    return LPC_TIM0->TC;
}

void Base_Tiempos_Puerto_Detener(void)
{
    LPC_TIM0->TCR=0U;
    __DSB();
}
/** @} */
