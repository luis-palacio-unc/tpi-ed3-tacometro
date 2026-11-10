/**
 * @file Driver_Alarmas.h
 * @brief Coordinacion de solicitudes logicas hacia LED y buzzer reemplazables.
 * @details El llamador entrega dos funciones void(bool) y activar/desactivar.
 * El servicio valida la interfaz y despacha ambas solicitudes, sin diagnostico fisico.
 * @note ESW.2.1.6. Sin inicializacion fisica, estado global mutable, heap o esperas propias.
 */
#ifndef TPI_DRIVER_ALARMAS_H
#define TPI_DRIVER_ALARMAS_H

#include <stdbool.h>

/**
 * @defgroup driver_alarmas ESW.2.1.6 - Driver de alarmas
 * @brief Solicitudes coordinadas independientes de placa, pines y polaridad.
 * @details Cada funcion traduce actividad logica al dispositivo elegido. El resultado
 * informa validacion/despacho, no confirma nivel electrico ni accionamiento atomico.
 * El llamador serializa el acceso a las salidas; no hay sincronizacion interna.
 * @{
 */

/**
 * @brief Funcion de salida suministrada por el integrador, sin contexto ni retorno.
 * @param[in] activar true solicita activo logico; false, inactivo, sin imponer nivel electrico.
 * @pre La funcion debe retornar sin esperas activas y admitir ambas solicitudes.
 * Acceso serializado por el llamador, sin modificar la interfaz durante la operacion.
 * @note No proporciona diagnostico electrico ni un resultado de aceptacion ficticio.
 */
typedef void (*Driver_Alarmas_EscribirSalida)(bool activar);

/**
 * @brief Par de funciones por interfaz; el servicio no la modifica, almacena ni libera.
 * @details Ambas funciones deben existir para despachar una solicitud. El integrador
 * elige las salidas; no hay campos de contexto, registro global o configuracion fisica.
 */
typedef struct {
    Driver_Alarmas_EscribirSalida escribir_led; /**< Solicitud logica del LED. */
    Driver_Alarmas_EscribirSalida escribir_buzzer; /**< Solicitud logica del buzzer. */
} Driver_Alarmas_Salidas;

/** @brief Resultado de validacion/despacho de esta llamada, sin confirmacion electrica. */
typedef enum {
    DRIVER_ALARMAS_ACEPTADA = 0, /**< Ambas funciones fueron invocadas y retornaron. */
    DRIVER_ALARMAS_ERROR_ARGUMENTO /**< Interfaz NULL o funcion ausente; ninguna fue invocada. */
} Driver_Alarmas_Estado;

/**
 * @brief Solicitar el mismo estado logico a LED y buzzer, previa validacion completa.
 * @details Si la interfaz esta completa, llama LED y luego buzzer exactamente una
 * vez cada uno con activar. No cachea, reintenta, deshace ni conserva resultados.
 * No evalua temperatura/umbral ni decide ante una lectura ausente o erronea.
 * @param[in] salidas Interfaz legible/estable; NULL o funciones ausentes se rechazan.
 * @param[in] activar true solicita activar ambos canales; false solicita desactivar ambos.
 * @return ACEPTADA tras invocar ambas funciones y retornar de ellas; ERROR_ARGUMENTO
 * si la interfaz es NULL o incompleta, sin invocar ninguna salida.
 * @pre Si salidas no es NULL, debe ser legible y estable durante toda la llamada.
 * Funciones presentes conformes al contrato; acceso a salidas serializado externamente.
 * @note USW.2.1.6.1. Sin heap, globals mutables, reloj o esperas internas. Duracion
 * y pila total dependen de las salidas; no acredita estado fisico ni prioridad temporal.
 */
Driver_Alarmas_Estado Driver_Alarmas_Aplicar(const Driver_Alarmas_Salidas *salidas,
                                          bool activar);

/** @} */
#endif /* TPI_DRIVER_ALARMAS_H */
