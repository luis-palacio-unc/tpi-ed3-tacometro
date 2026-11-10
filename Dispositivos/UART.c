/**
 * @file UART.c
 * @brief Wrappers de lectura/escritura UART0 sin esperas agregadas.
 * @details La biblioteca original resuelve FIFO y disponibilidad. El periferico
 * debe estar configurado para TX P0.2/RX P0.3 y los baudios elegidos previamente.
 * @note ESW.2.1.16. No modifica Drivers, pinmux, reloj, FIFO ni protocolo.
 */
#include "UART.h"
#include "lpc17xx_uart.h"
#include <stddef.h>

uint32_t UART_Read(uint8_t *destino, uint32_t capacidad)
{
    if (destino == NULL || capacidad == 0U) {
        return 0U;
    }
    return UART_Receive(UART0, destino, capacidad, NONE_BLOCKING);
}

uint32_t UART_Write(const uint8_t *datos, uint32_t longitud)
{
    if (datos == NULL || longitud == 0U) {
        return 0U;
    }
    return UART_Send(UART0, datos, longitud, NONE_BLOCKING);
}
