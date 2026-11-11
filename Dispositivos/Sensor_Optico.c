/**
 * @file Sensor_Optico.c
 * @brief Lectura de eventos de TIMER1 usando el driver existente.
 * @details Una consulta de TC, sin aritmetica, reinicios o control de ventanas.
 * @note ESW.2.1.10. No configura pinmux/reloj/contador ni modifica la biblioteca.
 */
#include "Sensor_Optico.h"
#include "lpc17xx_timer.h"

uint32_t SensorOptico_Read(void)
{
    return TIM_ReadTimer(LPC_TIM1);
}
