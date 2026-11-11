/**
 * @file Driver_Motor.c
 * @brief Mando del motor limitado a 1000 por mil.
 * @details Sin estado propio.
 * @note ESW.2.1.8. Contratos publicos en Driver_Motor.h.
 */
#include "Driver_Motor.h"
#include "Limites.h"
#include "Motor.h"

void Driver_Motor_Init(void)
{
    Motor_Init();
}

void Driver_Motor_Aplicar(uint16_t por_mil)
{
    (void)Motor_Write(por_mil > TPI_MANDO_MANUAL_MAX ? (uint16_t)TPI_MANDO_MANUAL_MAX : por_mil);
}
