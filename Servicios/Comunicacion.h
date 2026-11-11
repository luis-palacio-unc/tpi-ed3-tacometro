/**
 * @file Comunicacion.h
 * @brief Enlace con la PC: recepcion de consignas y envio de telemetria ASCII.
 * @details Consigna: "RPM n" terminada en LF o CRLF. Telemetria: una linea
 * "RPM=...;TEMP=...;ESTADO=..." por periodo.
 * @note ESW.2.1.7. Usa el dispositivo UART (UART0, 9600 8N1).
 */
#ifndef TPI_COMUNICACION_H
#define TPI_COMUNICACION_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup comunicacion ESW.2.1.7 - Comunicacion
 * @brief Consignas recibidas y telemetria enviada por UART.
 * @{
 */

/** @brief Bytes maximos de una consigna antes del LF: "RPM 185" mas CR opcional. */
#define COMUNICACION_LINEA_MAX 8U
/** @brief Bytes de la trama de telemetria mas larga posible, con su LF. */
#define COMUNICACION_TELEMETRIA_MAX 53U

/** @brief Estado de operacion informado en la telemetria. */
typedef enum {
    COMUNICACION_ESTADO_MANUAL = 0, /**< Se informa como MANUAL. */
    COMUNICACION_ESTADO_AUTO_ESPERA, /**< Se informa como AUTO_ESPERA. */
    COMUNICACION_ESTADO_AUTO_ACTIVO /**< Se informa como AUTO_ACTIVO. */
} Comunicacion_Estado;

/**
 * @brief Configurar la UART y vaciar la linea en recepcion.
 * @details Llama UART0_Init.
 * @pre Sistema_Init ejecutado.
 * @note USW.2.1.7.11. Llamar una vez antes de las demas funciones del servicio.
 */
void Comunicacion_Init(void);

/**
 * @brief Procesar los bytes recibidos y entregar la proxima consigna completa.
 * @details Retira bytes de UART_Read y arma la linea hasta el LF. Formato exacto:
 * "RPM", un espacio y de 1 a 3 digitos decimales, con CR opcional antes del LF;
 * el valor debe estar entre 0 y TPI_RPM_MAX (185). Una linea con otro formato,
 * fuera de rango o de mas de COMUNICACION_LINEA_MAX bytes se descarta entera
 * hasta su LF. Si llegaron varias consignas, entrega una por llamada, en orden.
 * @param[out] rpm Destino de la consigna en RPM; solo se escribe al retornar true.
 * @return true si se completo una consigna valida; false cuando no quedan bytes
 * recibidos por procesar.
 * @pre Comunicacion_Init ejecutado; llamar desde el lazo principal.
 * @note USW.2.1.7.12. Conserva la linea parcial entre llamadas. Llamar en un
 * while hasta false para vaciar la recepcion.
 */
bool Comunicacion_RecibirConsigna(uint32_t *rpm);

/**
 * @brief Olvidar todo lo recibido hasta este momento.
 * @details Vacia la linea parcial y retira sin interpretar los bytes pendientes
 * de la UART. La aplicacion la usa al entrar en automatico, para que solo valga
 * una consigna enviada despues del cambio de modo.
 * @pre Comunicacion_Init ejecutado; llamar desde el lazo principal.
 * @note USW.2.1.7.13.
 */
void Comunicacion_Descartar(void);

/**
 * @brief Enviar una trama de telemetria.
 * @details Formato fijo, sin espacios y terminado en LF:
 * RPM=n.ddd;TEMP=[-]n.dd;ESTADO=MANUAL|AUTO_ESPERA|AUTO_ACTIVO. La parte entera
 * no lleva ceros a la izquierda. La entrega a UART_Write, que no espera.
 * @param[in] rpm_milesimas Velocidad en milesimas de RPM (100000 se envia 100.000).
 * @param[in] temperatura_centesimas Temperatura en centesimas de grado Celsius
 * (2500 se envia 25.00; -50 se envia -0.50).
 * @param[in] estado Estado de operacion a informar.
 * @return true si la trama entro completa en el buffer de transmision; false si
 * no habia lugar suficiente y quedo truncada.
 * @pre Comunicacion_Init ejecutado; llamar desde el lazo principal.
 * @note USW.2.1.7.14. Una trama por periodo de 500 ms ocupa menos del 6 por
 * ciento de la capacidad del enlace a 9600 baudios.
 */
bool Comunicacion_EnviarTelemetria(uint32_t rpm_milesimas, int32_t temperatura_centesimas,
                                   Comunicacion_Estado estado);

/** @} */
#endif /* TPI_COMUNICACION_H */
