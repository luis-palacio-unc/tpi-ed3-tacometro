#include "Modelo_Timer0.h"
#include <string.h>
LPC_TIM_TypeDef modelo_timer0,modelo_timer1,modelo_timer2,modelo_timer3;
LPC_SC_TypeDef modelo_sc;
unsigned modelo_cambios_pclk,modelo_lecturas_pclk;
Modelo_Evento modelo_eventos[32];
size_t modelo_cantidad;
bool modelo_irq_habilitada,modelo_irq_pendiente,modelo_error;
static void Registrar(char operacion)
{
    if(modelo_cantidad<32U) {
        modelo_eventos[modelo_cantidad++]=(Modelo_Evento){operacion,LPC_TIM0->TCR,LPC_TIM0->TC,
            LPC_TIM0->PC,LPC_TIM0->PR,LPC_TIM0->MCR,LPC_TIM0->CCR,LPC_TIM0->CTCR,LPC_TIM0->IR};
    }
}
void Modelo_Limpiar(void)
{
    memset(&modelo_timer0,0,sizeof(modelo_timer0)); memset(&modelo_timer1,0,sizeof(modelo_timer1));
    memset(&modelo_timer2,0,sizeof(modelo_timer2)); memset(&modelo_timer3,0,sizeof(modelo_timer3));
    modelo_cambios_pclk=0U; modelo_lecturas_pclk=0U;
    memset(&modelo_sc,0,sizeof(modelo_sc)); modelo_cantidad=0U;
    modelo_irq_habilitada=true; modelo_irq_pendiente=true; modelo_error=false;
}
void NVIC_DisableIRQ(int irq)
{
    if(irq!=TIMER0_IRQn) { modelo_error=true; }
    modelo_irq_habilitada=false; Registrar('D');
}
void NVIC_ClearPendingIRQ(int irq)
{
    if(irq!=TIMER0_IRQn || modelo_irq_habilitada) { modelo_error=true; }
    modelo_irq_pendiente=false; Registrar('C');
}
void __DSB(void) { Registrar('B'); }
void Modelo_AvanzarPclk(uint64_t ciclos)
{
    if((LPC_SC->PCONP&2U)==0U || (LPC_TIM0->TCR&1U)==0U) { return; }
    if((LPC_TIM0->TCR&2U)!=0U) { LPC_TIM0->TC=0U; LPC_TIM0->PC=0U; return; }
    if(LPC_TIM0->CTCR!=0U || LPC_TIM0->MCR!=0U || LPC_TIM0->CCR!=0U) { modelo_error=true; return; }
    const uint64_t divisor=(uint64_t)LPC_TIM0->PR+1U;
    const uint64_t total=(uint64_t)LPC_TIM0->PC+ciclos;
    LPC_TIM0->PC=(uint32_t)(total%divisor);
    LPC_TIM0->TC+=(uint32_t)(total/divisor);
}
