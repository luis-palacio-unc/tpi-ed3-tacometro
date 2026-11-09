/**
 * @file Driver_Motor.h
 * @brief Solicitudes de mando de motor normalizado mediante una salida reemplazable.
 * @details Recibe unidades por mil y devuelve la aceptacion declarada por el proveedor.
 * No conoce modos, RPM, pines, frecuencia PWM ni etapa de potencia.
 * @note ESW.2.1.8. Sin inicializacion, heap, estado global mutable o esperas propias.
 */
#ifndef TPI_DRIVER_MOTOR_H
#define TPI_DRIVER_MOTOR_H

#include "Limites.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup driver_motor ESW.2.1.8 - Driver de motor
 * @brief Validacion y envio de mandos independientes del dispositivo fisico.
 * @details El proveedor traduce por mil a su salida fisica. Aceptacion no confirma
 * PWM, velocidad o detencion del eje. Serializacion y vida de contextos son externas.
 * @{
 */

/** @brief Maximo mando compartido con B01/B04: 1000 por mil es 100 por ciento. */
#define DRIVER_MOTOR_MANDO_MAX TPI_MANDO_MANUAL_MAX

/**
 * @brief Contrato de la funcion de salida suministrada por el integrador.
 * @param[in,out] contexto Estado opaco del proveedor; NULL si la funcion lo admite.
 * @param[in] mando_por_mil Solicitud 0..1000 inclusive; cero pide salida al 0 por ciento.
 * @return true si acepta la solicitud actual; false si declara fallo.
 * @pre Debe retornar sin espera activa. Contexto vivo/estable y acceso serializado;
 * no modificar la interfaz del servicio durante la llamada.
 */
typedef bool (*Driver_Motor_EscribirSalida)(void *contexto, uint16_t mando_por_mil);

/**
 * @brief Interfaz por llamada; el servicio no la modifica, retiene ni libera.
 * @details La funcion debe existir; contexto NULL depende de su contrato. El servicio
 * pasa el contexto exacto sin desreferenciarlo; el proveedor puede modificar su estado.
 */
typedef struct {
    Driver_Motor_EscribirSalida escribir; /**< Funcion externa que recibe el mando logico. */
    void *contexto; /**< Contexto opaco vivo/estable durante toda la llamada. */
} Driver_Motor_Salida;

/** @brief Resultado completo de esta solicitud, sin confirmar estado electrico o mecanico. */
typedef enum {
    DRIVER_MOTOR_ACEPTADA = 0, /**< El proveedor declaro aceptada la solicitud actual. */
    DRIVER_MOTOR_ERROR_ARGUMENTO, /**< Interfaz NULL/funcion ausente; ninguna llamada. */
    DRIVER_MOTOR_ERROR_RANGO, /**< Mando superior al maximo compartido; ninguna llamada. */
    DRIVER_MOTOR_ERROR_SALIDA /**< El proveedor fue invocado una vez y declaro fallo. */
} Driver_Motor_Estado;

/**
 * @brief Validar y solicitar un mando normalizado a una salida de motor.
 * @details Valida primero interfaz/funcion, luego rango. Un mando valido invoca
 * exactamente una vez al proveedor con el valor y contexto recibidos. No satura,
 * convierte RPM, cachea, reintenta, deshace ni guarda errores anteriores. Rechazar
 * no emite cero ni confirma que se conserve un estado fisico anterior.
 * @param[in] salida Interfaz legible/estable con funcion valida; NULL se rechaza.
 * @param[in] mando_por_mil uint16_t 0..1000 inclusive, 0..100 por ciento. Un origen
 * mas amplio/con signo debe validarse antes de convertir al tipo de este parametro.
 * @return ERROR_ARGUMENTO si interfaz NULL/incompleta; ERROR_RANGO si mando>maximo;
 * ACEPTADA si proveedor retorna true; ERROR_SALIDA si retorna false.
 * @pre Interfaz/contexto vivos y estables, acceso serializado, funcion conforme al
 * contrato. const no garantiza atomicidad ni detecta punteros invalidos no nulos.
 * @note USW.2.1.8.1. Tiempo/pila total dependen del proveedor. El llamador consume
 * el retorno actual; una solicitud posterior valida recupera si el proveedor acepta.
 */
Driver_Motor_Estado Driver_Motor_Aplicar(const Driver_Motor_Salida *salida,
                                      uint16_t mando_por_mil);

/**
 * @brief Solicitar explicitamente mando cero mediante el mismo contrato de salida.
 * @details Equivale a Aplicar con 0 por mil, incluso si ya se solicito cero antes.
 * No confirma detencion del eje ni realiza frenado o una secuencia de seguridad fisica.
 * @param[in] salida Interfaz legible/estable; NULL/funcion ausente se rechazan sin llamada.
 * @return ACEPTADA si el proveedor acepta cero; ERROR_SALIDA si falla;
 * ERROR_ARGUMENTO si la interfaz es invalida. No produce ERROR_RANGO.
 * @pre Vida util, serializacion y funcion conforme al contrato de Aplicar.
 * @note USW.2.1.8.2. Sin reintento, memoria propia ni inicializacion de hardware.
 */
Driver_Motor_Estado Driver_Motor_SolicitarCero(const Driver_Motor_Salida *salida);

/** @} */
#endif /* TPI_DRIVER_MOTOR_H */
