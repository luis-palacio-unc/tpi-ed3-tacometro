/**
 * @file Base_Tiempos_LPC1769.c
 * @brief Adaptador ARM de SysTick, SCB y PRIMASK con CMSIS existente.
 * @details Mantiene barreras explicitas del compilador: los intrinsecos CMSIS
 * antiguos no declaran clobber memory. No incorpora SystemInit/SystemCoreClock.
 * @note ESW.2.1.17. Se compila con CMSIS real para ARM; host usa dobles solo en test.
 */
#include "Base_Tiempos_Puerto.h"
#include "LPC17xx.h"
/** @addtogroup base_tiempos
 * @{
 */
/** @brief Impedir reordenamiento de accesos de memoria a traves de la seccion critica. */
#define BASE_TIEMPOS_BARRERA_COMPILADOR() __asm__ volatile ("" ::: "memory")
_Static_assert(__NVIC_PRIO_BITS == 5, "Revisar prioridad SysTick del LPC1769.");
_Static_assert(BASE_TIEMPOS_CONTROL_ACTIVO == (SysTick_CTRL_CLKSOURCE_Msk |
    SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk), "Revisar CTRL SysTick.");
_Static_assert(BASE_TIEMPOS_LIMPIAR_PENDIENTE == SCB_ICSR_PENDSTCLR_Msk, "Revisar PENDSTCLR.");

uint32_t Base_Tiempos_Puerto_EntrarCritica(void)
{
    BASE_TIEMPOS_BARRERA_COMPILADOR();
    const uint32_t anterior = __get_PRIMASK();
    __disable_irq();
    BASE_TIEMPOS_BARRERA_COMPILADOR();
    __DSB();
    __ISB();
    return anterior;
}

void Base_Tiempos_Puerto_SalirCritica(uint32_t anterior)
{
    BASE_TIEMPOS_BARRERA_COMPILADOR();
    __DSB();
    __set_PRIMASK(anterior);
    __ISB();
    BASE_TIEMPOS_BARRERA_COMPILADOR();
}

void Base_Tiempos_Puerto_Escribir(Base_Tiempos_Registro registro, uint32_t valor)
{
    switch (registro) {
    case BASE_TIEMPOS_REG_CONTROL:
        SysTick->CTRL = valor;
        break;
    case BASE_TIEMPOS_REG_RECARGA:
        SysTick->LOAD = valor;
        break;
    case BASE_TIEMPOS_REG_ACTUAL:
        SysTick->VAL = valor;
        break;
    case BASE_TIEMPOS_REG_PENDIENTE:
        SCB->ICSR = valor;
        break;
    case BASE_TIEMPOS_REG_PRIORIDAD:
        NVIC_SetPriority(SysTick_IRQn, valor);
        break;
    default:
        break;
    }
}
/** @} */
