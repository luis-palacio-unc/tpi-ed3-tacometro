#ifndef TEST_CMSIS_LPC17XX_H
#define TEST_CMSIS_LPC17XX_H
#include <stdint.h>
typedef struct { volatile uint32_t CTRL,LOAD,VAL,CALIB; } SysTick_Type;
typedef struct { volatile uint32_t ICSR,ajeno; volatile uint8_t SHP[12]; } SCB_Type;
extern SysTick_Type cmsis_systick;
extern SCB_Type cmsis_scb;
#define SysTick (&cmsis_systick)
#define SCB (&cmsis_scb)
#define __NVIC_PRIO_BITS 5
#define SysTick_IRQn (-1)
#define SysTick_CTRL_ENABLE_Msk UINT32_C(1)
#define SysTick_CTRL_TICKINT_Msk UINT32_C(2)
#define SysTick_CTRL_CLKSOURCE_Msk UINT32_C(4)
#define SCB_ICSR_PENDSTCLR_Msk UINT32_C(0x02000000)
uint32_t __get_PRIMASK(void);
void __disable_irq(void);
void __set_PRIMASK(uint32_t mascara);
void __DSB(void);
void __ISB(void);
void NVIC_SetPriority(int irq,uint32_t prioridad);
#endif
