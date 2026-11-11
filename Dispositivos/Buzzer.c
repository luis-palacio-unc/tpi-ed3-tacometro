/**
 * @file Buzzer.c
 * @brief Mando de buzzer P2.7 mediante el driver GPIO original.
 * @details Traduce activar logico a nivel alto sin reimplementar operaciones GPIO.
 * @note ESW.2.1.15. No modifica la biblioteca; sin acceso directo a registros.
 */
#include "Buzzer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"

void Buzzer_Init(void)
{
    const PINSEL_CFG_T pin = {PORT_2, PIN_7, PINSEL_FUNC_00, PINSEL_TRISTATE, DISABLE};
    PINSEL_ConfigPin(&pin);
    Buzzer_Write(false);
    GPIO_SetDir(PORT_2, UINT32_C(1) << PIN_7, GPIO_OUTPUT);
}

void Buzzer_Write(bool activar)
{
    GPIO_SetPinState(PORT_2,PIN_7,activar?SET:RESET);
}
