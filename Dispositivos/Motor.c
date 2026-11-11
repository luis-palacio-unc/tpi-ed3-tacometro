/**
 * @file Motor.c
 * @brief Conversion acotada de mando por mil y actualizacion de PWM1.1.
 * @details Lee MR0 de PWM1 y usa uint64_t para convertir sin overflow.
 * La escritura de match/LER pertenece al driver original.
 * @note ESW.2.1.9. Sin configuracion de perifericos ni estado propio.
 */
#include "Motor.h"
#include "Limites.h"
#include "lpc17xx_pwm.h"

bool Motor_Write(uint16_t mando_por_mil)
{
    if (mando_por_mil > TPI_MANDO_MANUAL_MAX) {
        return false;
    }
    const uint32_t periodo_ticks = LPC_PWM1->MR0;
    if (periodo_ticks == 0U || periodo_ticks == UINT32_MAX) {
        return false;
    }
    const uint64_t producto = (uint64_t)mando_por_mil * periodo_ticks;
    const uint32_t activos = (uint32_t)((producto + TPI_MANDO_MANUAL_MAX / 2U) /
                                      TPI_MANDO_MANUAL_MAX);
    const uint32_t match = activos == periodo_ticks ? periodo_ticks + 1U : activos;
    PWM_MatchUpdateSingle(PWM_MATCH_1, match);
    return true;
}
