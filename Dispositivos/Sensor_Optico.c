/**
 * @file Sensor_Optico.c
 * @brief Configuracion y lectura de eventos de TIMER1 usando el driver existente.
 * @details Contador de flancos de CAP1.0; la lectura es una consulta de TC, sin
 * aritmetica, reinicios o control de ventanas.
 * @note ESW.2.1.10. No modifica la biblioteca; sin acceso directo a registros.
 */
#include "Sensor_Optico.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"

void SensorOptico_Init(void)
{
    const TIM_COUNTERCFG_T contador = {.input = TIM_CAPTURE_0, .edge = TIM_CTR_RISING};
    const PINSEL_CFG_T pin = {PORT_1, PIN_18, PINSEL_FUNC_11, PINSEL_PULLUP, DISABLE};
    TIM_InitCounter(LPC_TIM1, &contador);
    PINSEL_ConfigPin(&pin);
    TIM_Enable(LPC_TIM1);
}

uint32_t SensorOptico_Read(void)
{
    return TIM_ReadTimer(LPC_TIM1);
}
