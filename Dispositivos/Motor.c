/**
 * @file Motor.c
 * @brief Conversion acotada de mando por mil y actualizacion de PWM1.1.
 * @details Intermedio uint64_t evita desbordar al multiplicar el periodo de
 * 32 bits por el mando. La escritura de match/LER pertenece al driver original.
 * @note ESW.2.1.9. Sin acceso propio a registros ni configuracion de perifericos.
 */
#include "Motor.h"
#include "Limites.h"
#include "lpc17xx_pwm.h"

bool Motor_Write(uint16_t mando_por_mil, uint32_t periodo_ticks)
{
    if (mando_por_mil > TPI_MANDO_MANUAL_MAX || periodo_ticks == 0U ||
        periodo_ticks == UINT32_MAX) {
        return false;
    }
    const uint64_t producto = (uint64_t)mando_por_mil * periodo_ticks;
    const uint32_t activos = (uint32_t)((producto + TPI_MANDO_MANUAL_MAX / 2U) /
                                      TPI_MANDO_MANUAL_MAX);
    const uint32_t match = activos == periodo_ticks ? periodo_ticks + 1U : activos;
    PWM_MatchUpdateSingle(PWM_MATCH_1, match);
    return true;
}
