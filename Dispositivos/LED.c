/**
 * @file LED.c
 * @brief Mando del LED rojo P0.22 mediante el driver GPIO original.
 * @details Traduce activar logico al nivel bajo requerido por el LED de anodo comun.
 * @note ESW.2.1.14. Sin configuracion GPIO ni dependencia de la capa de servicios.
 */
#include "LED.h"
#include "lpc17xx_gpio.h"

bool LED_Write(void *contexto,bool activar)
{
    (void)contexto;
    GPIO_SetPinState(PORT_0,PIN_22,activar?RESET:SET);
    return true;
}
