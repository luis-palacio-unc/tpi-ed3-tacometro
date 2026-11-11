/**
 * @file Motor.c
 * @brief Envio directo del mando a PWM1.1 con periodo fijo de 1000 ticks.
 * @details El driver original actualiza MR1/LER; MR0=1000 se configura al integrar.
 * @note ESW.2.1.9. Sin conversion, lectura de registros o inicializacion propias.
 */
#include "Motor.h"
#include "Limites.h"
#include "lpc17xx_pwm.h"

bool Motor_Write(uint16_t mando_por_mil)
{
    if (mando_por_mil > TPI_MANDO_MANUAL_MAX) {
        return false;
    }
    PWM_MatchUpdateSingle(PWM_MATCH_1, mando_por_mil);
    return true;
}
