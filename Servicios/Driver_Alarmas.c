/**
 * @file Driver_Alarmas.c
 * @brief LED rojo y buzzer accionados juntos.
 * @details Sin estado propio.
 * @note ESW.2.1.6. Contratos publicos en Driver_Alarmas.h.
 */
#include "Driver_Alarmas.h"
#include "Buzzer.h"
#include "LED.h"

void Driver_Alarmas_Init(void)
{
    LED_Init();
    Buzzer_Init();
}

void Driver_Alarmas_Aplicar(bool activa)
{
    LED_Write(activa);
    Buzzer_Write(activa);
}
