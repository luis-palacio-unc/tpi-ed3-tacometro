/**
 * @file Selector.c
 * @brief Acceso al selector P2.6 mediante el driver GPIO original.
 * @details El driver lee el puerto y el dispositivo selecciona su bit activo por alto.
 * @note ESW.2.1.13. No modifica la biblioteca; sin acceso directo a registros.
 */
#include "Selector.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"

void Selector_Init(void)
{
    const PINSEL_CFG_T pin = {PORT_2, PIN_6, PINSEL_FUNC_00, PINSEL_PULLUP, DISABLE};
    PINSEL_ConfigPin(&pin);
    GPIO_SetDir(PORT_2, UINT32_C(1) << PIN_6, GPIO_INPUT);
}

bool Selector_Read(void)
{
    return (GPIO_ReadValue(PORT_2)&(UINT32_C(1)<<PIN_6))!=0U;
}
