/**
 * @file Selector.c
 * @brief Acceso al selector P2.6 mediante el driver GPIO original.
 * @details El driver lee el puerto y el dispositivo selecciona su bit activo por alto.
 * @note ESW.2.1.13. No configura perifericos ni modifica la biblioteca.
 */
#include "Selector.h"
#include "lpc17xx_gpio.h"

bool Selector_Read(void)
{
    return (GPIO_ReadValue(PORT_2)&(UINT32_C(1)<<PIN_6))!=0U;
}
