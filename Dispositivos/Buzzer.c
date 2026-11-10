/**
 * @file Buzzer.c
 * @brief Mando de buzzer P2.7 mediante el driver GPIO original.
 * @details Traduce activar logico a nivel alto sin reimplementar operaciones GPIO.
 * @note ESW.2.1.15. No configura perifericos ni modifica la biblioteca.
 */
#include "Buzzer.h"
#include "lpc17xx_gpio.h"

void Buzzer_Write(bool activar)
{
    GPIO_SetPinState(PORT_2,PIN_7,activar?SET:RESET);
}
