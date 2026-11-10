/**
 * @file UART.h
 * @brief Transferencia de bytes mediante UART0 y el driver original.
 * @details Asignacion fija: TXD0 en P0.2 y RXD0 en P0.3, PINSEL0 funcion01,
 * segun UM10360 rev4.1 tabla80/p118. Pinmux/baudios se aplicaran en integracion final.
 * @note ESW.2.1.16. Sin inicializacion, buffers propios, ISR o protocolo de mensajes.
 */
#ifndef TPI_UART_H
#define TPI_UART_H

#include <stdint.h>

/**
 * @brief Baudios solicitados al inicializar UART0; default115200, configurable al compilar.
 * @details Parametro preparado para UART_CFG_T.baudRate de la integracion final;
 * no configura el periferico al incluir este header ni al leer/escribir bytes.
 * La representabilidad del entero no acredita divisores/precision con el reloj real.
 */
#ifndef UART_BAUDRATE
#define UART_BAUDRATE UINT32_C(115200)
#endif
#if UART_BAUDRATE<=0 || UART_BAUDRATE>UINT32_MAX
#error "UART_BAUDRATE debe ser positivo y representable en uint32_t."
#endif

/**
 * @brief Leer los bytes disponibles mediante UART_Receive con NONE_BLOCKING.
 * @details Delega una vez una peticion valida sin esperar completar la capacidad.
 * No agrega terminador ni conserva el buffer; duracion dependiente de los bytes pedidos.
 * @param[out] destino Buffer de capacidad bytes; NULL devuelve0 sin acceder al driver.
 * @param[in] capacidad Maximo de bytes; cero devuelve0 sin tocar memoria ni driver.
 * @return Numero recibido,0..capacidad; cero tambien indica ausencia de datos.
 * No distingue errores de linea. Solo el prefijo recibido puede ser modificado.
 * @pre Con destino no NULL y capacidad positiva, buffer escribible del tamano
 * indicado. Para operaciones validas, UART0 preconfigurada: reloj/baudios/formato, RX/TX/FIFO habilitados, DLAB=0,
 * P0.2/TXD0 y P0.3/RXD0 funcion01; uso privilegiado serializado, sin otro lector ISR/DMA.
 * @note USW.2.1.16.1. Sin estado/heap/esperas propias ni confirmacion fisica.
 */
uint32_t UART_Read(uint8_t *destino, uint32_t capacidad);

/**
 * @brief Entregar bytes al transmisor mediante UART_Send con NONE_BLOCKING.
 * @details Delega una vez una peticion valida y conserva el origen, sin esperar fin
 * fisico. No agrega terminador ni reintenta; el consumidor mantiene el resto/offset.
 * @param[in] datos Buffer legible de longitud bytes; NULL devuelve0 sin llamar al driver.
 * @param[in] longitud Cantidad solicitada; cero devuelve0 sin tocar memoria ni driver.
 * @return Numero entregado al transmisor/FIFO,0..longitud; cero indica sin progreso.
 * No confirma recepcion por PC, vaciado del transmisor ni errores electricos/de linea.
 * @pre Con datos no NULL y longitud positiva, buffer legible/estable del tamano
 * indicado. Para operaciones validas, UART0 preconfigurada: reloj/baudios/formato, RX/TX/FIFO habilitados, DLAB=0,
 * P0.2/TXD0 y P0.3/RXD0 funcion01; acceso privilegiado serializado, sin otro escritor ISR/DMA.
 * @note USW.2.1.16.2. Sin estado/heap/esperas propias; duracion depende del tamano.
 */
uint32_t UART_Write(const uint8_t *datos, uint32_t longitud);

#endif /* TPI_UART_H */
