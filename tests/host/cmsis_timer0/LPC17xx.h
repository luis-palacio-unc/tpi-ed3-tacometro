#ifndef TEST_CMSIS_TIMER0_H
#define TEST_CMSIS_TIMER0_H
#include <stdint.h>
typedef struct {
    volatile uint32_t IR,TCR,TC,PR,PC,MCR,MR0,MR1,MR2,MR3,CCR,CR0,CR1;
    uint32_t reservado0[2]; volatile uint32_t EMR; uint32_t reservado1[12];
    volatile uint32_t CTCR;
} LPC_TIM_TypeDef;
typedef struct { volatile uint32_t PCONP,PCLKSEL0,PCLKSEL1,PLL0CON,FLASHCFG; } LPC_SC_TypeDef;
extern LPC_TIM_TypeDef modelo_timer0,modelo_timer1;
extern LPC_SC_TypeDef modelo_sc;
#define LPC_TIM0 (&modelo_timer0)
#define LPC_TIM1 (&modelo_timer1)
#define LPC_SC (&modelo_sc)
#define TIMER0_IRQn 1
void NVIC_DisableIRQ(int irq);
void NVIC_ClearPendingIRQ(int irq);
void __DSB(void);
#endif
