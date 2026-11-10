#include "Modelo_Timer0.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>
#include <stdlib.h>
void CLKPWR_ConfigPPWR(uint32_t type,FunctionalState enabled)
{
    if(enabled==ENABLE) { LPC_SC->PCONP|=type&CLKPWR_PCONP_BITMASK; }
    else if(enabled==DISABLE) { LPC_SC->PCONP&=(~type)&CLKPWR_PCONP_BITMASK; }
    else { abort(); }
}
void CLKPWR_SetPCLKDiv(uint32_t type,uint32_t divisor)
{
    ++modelo_cambios_pclk;
    const uint32_t bit=type<32U?type:type-32U;
    volatile uint32_t *const reg=type<32U?&LPC_SC->PCLKSEL0:&LPC_SC->PCLKSEL1;
    *reg=(*reg&~(UINT32_C(3)<<bit))|(divisor<<bit);
}
uint32_t CLKPWR_GetPCLK(uint32_t type)
{
    ++modelo_lecturas_pclk;
    if(type==CLKPWR_PCLKSEL_TIMER0) { return UINT32_C(25000000); }
    if(type==CLKPWR_PCLKSEL_TIMER1) { return UINT32_C(20000000); }
    if(type==CLKPWR_PCLKSEL_TIMER2) { return UINT32_C(10000000); }
    if(type==CLKPWR_PCLKSEL_TIMER3) { return UINT32_C(5000000); }
    abort();
}
void PINSEL_ConfigPin(const PINSEL_CFG_T *cfg)
{
    (void)cfg; fputs("PINSEL no pertenece a la base temporal\n",stderr); abort();
}
void check_failed(uint8_t *file,uint32_t line)
{
    fprintf(stderr,"CHECK_PARAM: %s:%u\n",(const char *)file,(unsigned)line); abort();
}
