/**
 * @file Motor.c
 * @brief Configuracion de PWM1.1 y envio directo del mando con periodo de 1000 ticks.
 * @details El driver original configura PWM1 y actualiza MR1/LER; MR0=1000 fijo.
 * @note ESW.2.1.9. Sin conversion ni acceso directo a registros.
 */
#include "Motor.h"
#include "Limites.h"
#include "lpc17xx_pwm.h"

/** @brief Divisor del prescaler: PCLK_PWM1 25 MHz / 25 = 1 MHz (PR = 24). */
#define MOTOR_PWM_PRESCALER 25U
/** @brief Periodo en ticks de 1 us: MR0 = 1000, 1 kHz; iguala TPI_MANDO_MANUAL_MAX. */
#define MOTOR_PWM_PERIODO_TICKS 1000U

_Static_assert(MOTOR_PWM_PERIODO_TICKS == TPI_MANDO_MANUAL_MAX,
               "MR0 debe igualar el mando maximo para el envio directo a MR1.");

void Motor_Init(void)
{
    const PWM_TIMERCFG_T base = {PWM_TICK, MOTOR_PWM_PRESCALER};
    const PWM_MATCHCFG_T periodo = {
        .channel = PWM_MATCH_0,
        .intEn = DISABLE,
        .stopEn = DISABLE,
        .resetEn = ENABLE,
        .matchValue = MOTOR_PWM_PERIODO_TICKS
    };
    const PWM_MATCHCFG_T mando = {
        .channel = PWM_MATCH_1,
        .intEn = DISABLE,
        .stopEn = DISABLE,
        .resetEn = DISABLE,
        .matchValue = 0U
    };
    PWM_InitTimer(&base);
    PWM_ConfigMatch(&periodo);
    PWM_ConfigMatch(&mando);
    PWM_ChannelConfig(PWM_CHANNEL_1, PWM_SINGLE_EDGE);
    PWM_PinConfig(PWM1_P2_0);
    PWM_ChannelEnable(PWM_CHANNEL_1);
    PWM_CounterEnable();
    PWM_Enable();
}

bool Motor_Write(uint16_t mando_por_mil)
{
    if (mando_por_mil > TPI_MANDO_MANUAL_MAX) {
        return false;
    }
    PWM_MatchUpdateSingle(PWM_MATCH_1, mando_por_mil);
    return true;
}
