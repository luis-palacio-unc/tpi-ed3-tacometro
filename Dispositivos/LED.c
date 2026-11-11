/**
 * @file LED.c
 * @brief Mando del LED rojo P0.22 mediante el driver GPIO original.
 * @details Traduce activar logico al nivel bajo requerido por el LED de anodo comun.
 * @note ESW.2.1.14. Sin dependencia de la capa de servicios ni acceso directo a registros.
 */
#include "LED.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"

void LED_Init(void)
{
    const PINSEL_CFG_T pin = {PORT_0, PIN_22, PINSEL_FUNC_00, PINSEL_TRISTATE, DISABLE};
    PINSEL_ConfigPin(&pin);
    LED_Write(false);
    GPIO_SetDir(PORT_0, UINT32_C(1) << PIN_22, GPIO_OUTPUT);
}

void LED_Write(bool activar)
{
    GPIO_SetPinState(PORT_0,PIN_22,activar?RESET:SET);
}
