/**
 * @file Driver_Alarmas.h
 * @brief Coordinacion de solicitudes logicas hacia salidas LED y buzzer reemplazables.
 * @details El llamador entrega funciones/contextos externos y activar/desactivar.
 * El servicio valida la interfaz, intenta ambas salidas y devuelve su aceptacion.
 * @note ESW.2.1.6. Sin inicializacion fisica, estado global mutable, heap o esperas propias.
 */
#ifndef TPI_DRIVER_ALARMAS_H
#define TPI_DRIVER_ALARMAS_H

#include <stdbool.h>

/**
 * @defgroup driver_alarmas ESW.2.1.6 - Driver de alarmas
 * @brief Solicitudes coordinadas independientes de placa, pines y polaridad.
 * @details El proveedor traduce activo logico a su dispositivo. El retorno acredita
 * aceptacion declarada por funciones, no estado electrico real ni escritura atomica.
 * Serializar operaciones sobre contextos compartidos; no se ofrece sincronizacion interna.
 * @{
 */

/**
 * @brief Contrato de una funcion de salida suministrada por el integrador.
 * @param[in,out] contexto Estado externo del proveedor; puede ser NULL si lo admite.
 * @param[in] activar true solicita activo logico; false, inactivo, sin imponer nivel electrico.
 * @return true si el proveedor acepta la operacion; false si declara fallo.
 * @pre La funcion debe retornar sin esperas activas. Contexto valido/vivo durante
 * llamada y acceso serializado por el llamador. No modificar la interfaz del servicio.
 */
typedef bool (*Driver_Alarmas_EscribirSalida)(void *contexto, bool activar);

/**
 * @brief Interfaz por instancia; el servicio no la modifica, almacena ni libera.
 * @details Ambas funciones deben existir antes de llamar. Contextos opacos propios
 * de cada proveedor, estables durante toda la operacion; NULL depende del proveedor.
 */
typedef struct {
    Driver_Alarmas_EscribirSalida escribir_led; /**< Funcion de solicitud logica del LED. */
    void *contexto_led; /**< Contexto externo del LED; nunca se desreferencia en el servicio. */
    Driver_Alarmas_EscribirSalida escribir_buzzer; /**< Funcion de solicitud logica del buzzer. */
    void *contexto_buzzer; /**< Contexto externo del buzzer; puede diferir del contexto LED. */
} Driver_Alarmas_Salidas;

/** @brief Resultado completo de una solicitud; no representa confirmacion electrica. */
typedef enum {
    DRIVER_ALARMAS_ACEPTADA = 0, /**< Ambos proveedores aceptaron la solicitud actual. */
    DRIVER_ALARMAS_ERROR_ARGUMENTO, /**< Interfaz NULL o funcion ausente; no se invoco ninguna. */
    DRIVER_ALARMAS_ERROR_LED, /**< LED declaro fallo; buzzer fue invocado y acepto. */
    DRIVER_ALARMAS_ERROR_BUZZER, /**< Buzzer declaro fallo; LED fue invocado y acepto. */
    DRIVER_ALARMAS_ERROR_AMBOS /**< Ambos fueron invocados y declararon fallo. */
} Driver_Alarmas_Estado;

/**
 * @brief Solicitar el mismo estado logico a LED y buzzer, validando y reportando fallos.
 * @details Valida toda la interfaz antes de invocar proveedores. Si es valida,
 * llama LED y luego buzzer exactamente una vez, incluso si LED falla. No cachea,
 * reintenta, deshace o conserva errores previos. Una solicitud posterior recupera
 * si ambos proveedores la aceptan. No evalua temperatura/umbral ni decide ante ausencia.
 * @param[in] salidas Interfaz legible/estable con funciones validas; NULL se rechaza.
 * @param[in] activar true solicita activar ambos canales; false solicita desactivar ambos.
 * @return ACEPTADA si ambos aceptan; ERROR_LED/ERROR_BUZZER/ERROR_AMBOS segun
 * fallos declarados; ERROR_ARGUMENTO si interfaz NULL/incompleta, sin llamadas.
 * @pre Configuracion/contextos vivos y estables, acceso serializado, proveedores
 * conformes al contrato. const no garantiza atomicidad ni que un contexto sea valido.
 * @note USW.2.1.6.1. Sin heap, globals mutables, reloj o esperas internas. Duracion
 * y pila total dependen de proveedores; no acredita estado fisico ni prioridad temporal.
 */
Driver_Alarmas_Estado Driver_Alarmas_Aplicar(const Driver_Alarmas_Salidas *salidas,
                                          bool activar);

/** @} */
#endif /* TPI_DRIVER_ALARMAS_H */
