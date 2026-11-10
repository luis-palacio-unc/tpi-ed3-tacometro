#include "Modelo_Timer0.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"
#include <stdio.h>
#define CHECK(e) do { if(!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); return false; } } while(0)
static bool FreeRunning(void)
{
    LPC_TIM_TypeDef *const timers[]={LPC_TIM0,LPC_TIM1,LPC_TIM2,LPC_TIM3};
    const uint32_t powers[]={CLKPWR_PCONP_PCTIM0,CLKPWR_PCONP_PCTIM1,
                            CLKPWR_PCONP_PCTIM2,CLKPWR_PCONP_PCTIM3};
    for(size_t i=0U;i<4U;++i) {
        Modelo_Limpiar();
        LPC_SC->PCONP=UINT32_C(0x80000000);
        LPC_SC->PCLKSEL0=UINT32_C(0x12345678); LPC_SC->PCLKSEL1=UINT32_C(0xABCDEF01);
        LPC_TIM_TypeDef *const t=timers[i];
        t->TCR=3U; t->TC=11U; t->PC=22U; t->CTCR=3U;
        t->MCR=UINT32_MAX; t->CCR=UINT32_MAX; t->EMR=UINT32_MAX; t->MR0=123U;
        TIM_InitFreeRunning(t,UINT32_MAX);
        CHECK(t->TCR==0U && t->TC==0U && t->PC==0U && t->PR==UINT32_MAX);
        CHECK(t->CTCR==0U && t->MCR==0U && t->CCR==0U && t->EMR==0U);
        CHECK(t->IR==63U && t->MR0==123U);
        CHECK(LPC_SC->PCONP==(UINT32_C(0x80000000)|powers[i]));
        CHECK(LPC_SC->PCLKSEL0==UINT32_C(0x12345678) && LPC_SC->PCLKSEL1==UINT32_C(0xABCDEF01));
        CHECK(modelo_cambios_pclk==0U && modelo_lecturas_pclk==0U);
        TIM_Enable(t); CHECK(t->TCR==1U);
        t->TC=19U; CHECK(TIM_ReadTimer(t)==19U && t->TC==19U);
        TIM_Disable(t); CHECK(t->TCR==0U && t->TC==19U);
        TIM_InitFreeRunning(t,0U); CHECK(t->PR==0U && t->TC==0U && t->TCR==0U);
    }
    return true;
}
static bool LegacyTick(void)
{
    CHECK(CLKPWR_PCLKSEL_BITMASK(0U)==UINT32_C(3));
    CHECK(CLKPWR_PCLKSEL_BITMASK(30U)==UINT32_C(0xC0000000));
    LPC_TIM_TypeDef *const timers[]={LPC_TIM0,LPC_TIM1,LPC_TIM2,LPC_TIM3};
    for(size_t i=0U;i<4U;++i) {
        Modelo_Limpiar();
        const TIM_TIMERCFG_T cfg={TIM_TICK,25000U};
        TIM_InitTimer(timers[i],&cfg);
        CHECK(timers[i]->PR==24999U && timers[i]->TCR==0U);
        CHECK(modelo_cambios_pclk==1U && modelo_lecturas_pclk==0U);
    }
    return true;
}
static bool LegacyMicroseconds(void)
{
    LPC_TIM_TypeDef *const timers[]={LPC_TIM0,LPC_TIM1,LPC_TIM2,LPC_TIM3};
    for(size_t i=0U;i<4U;++i) {
        Modelo_Limpiar();
        const TIM_TIMERCFG_T cfg={TIM_US,1000U};
        const uint32_t expected[]={24999U,19999U,9999U,4999U};
        TIM_InitTimer(timers[i],&cfg);
        CHECK(timers[i]->PR==expected[i] && timers[i]->TCR==0U);
        CHECK(modelo_cambios_pclk==1U && modelo_lecturas_pclk==1U);
    }
    return true;
}
int main(void)
{
    if(!FreeRunning() || !LegacyTick() || !LegacyMicroseconds()) { return 1; }
    puts("PASS timer driver: 3 casos, 4 perifericos; driver real, CLK/PINSEL modelados.");
    return 0;
}
