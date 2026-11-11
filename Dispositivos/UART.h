/**
 * @file UART.h
 * @brief Transferencia de bytes por UART0 con buffers circulares e interrupciones.
 * @details Asignacion fija: TXD0 en P0.2 y RXD0 en P0.3, PINSEL0 funcion01,
 * segun UM10360 rev4.1 tabla80/p118. Formato 8N1 a UART_BAUDRATE.
 * @note ESW.2.1.16. Sin protocolo de mensajes; usa la biblioteca original.
 */
#ifndef TPI_UART_H
#define TPI_UART_H

#include <stdint.h>

/**
 * @brief Baudios aplicados por UART0_Init; default 9600, configurable al compilar.
 * @details Se entrega a UART_CFG_T.baudRate de la biblioteca, que calcula DLL/DLM y
 * el divisor fraccional desde PCLK_UART0. Con PCLK 25 MHz y 9600 baudios su
 * algoritmo obtiene DLL = 93, DLM = 0 y U0FDR = 0x43 (DIVADDVAL 3, MULVAL 4):
 * 25 MHz/(16*93*(1+3/4)) = 9600,6 baudios (UM10360 tablas 274, 275 y 286).
 * La representabilidad del entero no acredita divisores/precision con el reloj real.
 */
#ifndef UART_BAUDRATE
#define UART_BAUDRATE UINT32_C(9600)
#endif
#if UART_BAUDRATE<=0 || UART_BAUDRATE>UINT32_MAX
#error "UART_BAUDRATE debe ser positivo y representable en uint32_t."
#endif

/** @brief Capacidad en bytes del buffer circular de recepcion (potencia de dos). */
#define UART_BUFFER_RX_BYTES 64U
/** @brief Capacidad en bytes del buffer circular de transmision (potencia de dos). */
#define UART_BUFFER_TX_BYTES 128U

/**
 * @brief Configurar UART0, sus pines y sus interrupciones de recepcion/transmision.
 * @details Con la biblioteca original y en este orden: UART_Init alimenta UART0
 * (PCONP.PCUART0, UM10360 tabla 46) y programa divisores y 8N1 (U0LCR = 0x03,
 * tabla 280) para UART_BAUDRATE; UART_FIFOConfig habilita y vacia las FIFO con
 * disparo de RX en 1 caracter (U0FCR = 0x07, tabla 279); PINSEL_ConfigPin asigna
 * P0.2 = TXD0 y P0.3 = RXD0, funcion 01 (tabla 80), TX sin resistencias y RX con
 * pull-up para reposo definido sin conversor conectado; UART_TxEnable repone
 * U0TER.TXEN (tabla 288) que UART_Init deja en 0; UART_IntConfig habilita RBR
 * (incluye time-out de caracter) y THRE (U0IER = 0x03, tabla 276); el NVIC queda
 * con UART0_IRQn en prioridad 2. Vacia los buffers circulares.
 * @pre SystemInit ejecutado y SystemCoreClock = CCLK real (PCLK_UART0 = CCLK/4).
 * UART0 exclusiva. Llamar desde el lazo privilegiado antes de UART_Read/UART_Write.
 * @note USW.2.1.16.3. El nombre evita la colision con UART_Init de la biblioteca.
 * La espera de THRE interna de UART_Init de la biblioteca ocurre solo al iniciar.
 */
void UART0_Init(void);

/**
 * @brief Retirar bytes ya recibidos del buffer circular de recepcion.
 * @details No accede al periferico ni espera: copia lo que el ISR de UART0 guardo,
 * en orden de llegada. No agrega terminador. Si el buffer de 64 bytes se llena, el
 * ISR descarta los bytes nuevos; los errores de linea no se informan.
 * @param[out] destino Buffer de capacidad bytes; NULL devuelve0 sin efectos.
 * @param[in] capacidad Maximo de bytes; cero devuelve0 sin tocar memoria.
 * @return Numero copiado,0..min(capacidad,UART_BUFFER_RX_BYTES); cero tambien indica
 * ausencia de datos. Solo el prefijo copiado puede ser modificado.
 * @pre UART0_Init ejecutado; con destino no NULL y capacidad positiva, buffer
 * escribible del tamano indicado. Unico lector, desde el lazo principal.
 * @note USW.2.1.16.1. Sin seccion critica: el ISR solo avanza el indice de
 * escritura y esta funcion solo el de lectura (cargas/escrituras de 32 bits).
 */
uint32_t UART_Read(uint8_t *destino, uint32_t capacidad);

/**
 * @brief Encolar bytes para transmitir por interrupcion, sin esperar.
 * @details Copia al buffer circular de transmision lo que entre y arranca el envio
 * si el transmisor esta ocioso; el ISR de UART0 continua hasta vaciar el buffer.
 * No agrega terminador ni reintenta; el consumidor mantiene el resto/offset.
 * @param[in] datos Buffer legible de longitud bytes; NULL devuelve0 sin efectos.
 * @param[in] longitud Cantidad solicitada; cero devuelve0 sin tocar memoria.
 * @return Numero encolado,0..longitud; menor que longitud si el buffer de
 * UART_BUFFER_TX_BYTES no tenia lugar, cero si estaba lleno. Con el buffer vacio una
 * trama de hasta 128 bytes (p. ej. 53) se encola completa. No confirma recepcion
 * por la PC, vaciado del transmisor ni errores electricos/de linea.
 * @pre UART0_Init ejecutado; con datos no NULL y longitud positiva, buffer legible
 * y estable durante la llamada. Unico escritor, desde el lazo principal (no ISR).
 * @note USW.2.1.16.2. Cebado sin seccion critica: tras publicar los bytes pide la
 * interrupcion de UART0 con NVIC_SetPendingIRQ; el ISR, unico que escribe U0THR,
 * carga la FIFO si U0LSR.THRE = 1. Asi no depende de que el periferico genere
 * THRE al habilitarla con el transmisor ya vacio (UM10360 seccion 14.4.5).
 */
uint32_t UART_Write(const uint8_t *datos, uint32_t longitud);

#endif /* TPI_UART_H */
