/**
 * @file Temperatura.h
 * @brief Lectura de temperatura fisica y disponibilidad de una fuente generica.
 * @details El proveedor entrega centesimas Celsius y validez. El servicio conserva
 * la magnitud o invalida la lectura sin conocer dispositivos ni parametros electricos.
 * @note ESW.2.1.4. Datos por valor, acceso serializado externo y sin deteccion de frescura.
 */
#ifndef TPI_TEMPERATURA_H
#define TPI_TEMPERATURA_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup temperatura ESW.2.1.4 - Temperatura
 * @brief Contrato de temperatura independiente del origen de la medicion.
 * @details Cada proveedor adapta sus lecturas y errores a Celsius/estado genericos.
 * La aplicacion consume solo lecturas VALIDAS. No se interpreta plausibilidad de
 * una fuente especifica ni se configura adquisicion o alarma.
 * @{
 */

/** @brief Unidades por grado Celsius; constante con signo para datos int32_t. */
#define TEMPERATURA_CENTESIMAS_POR_GRADO INT32_C(100)

/** @brief Disponibilidad normalizada del proveedor, sin errores especificos del origen. */
typedef enum {
    TEMPERATURA_FUENTE_SIN_DATOS = 0, /**< Todavia no hay una medicion disponible. */
    TEMPERATURA_FUENTE_VALIDA, /**< El proveedor valida la medicion fisica entregada. */
    TEMPERATURA_FUENTE_ERROR /**< El proveedor no puede entregar una medicion valida. */
} Temperatura_EstadoFuente;

/** @brief Estado generico de la lectura del servicio. */
typedef enum {
    TEMPERATURA_SIN_DATOS = 0, /**< No hay valor disponible; campo numerico cero. */
    TEMPERATURA_VALIDA, /**< Valor recibido de una fuente valida; puede ser cero o negativo. */
    TEMPERATURA_ERROR_ARGUMENTO, /**< Salida NULL o estado de fuente desconocido. */
    TEMPERATURA_ERROR_FUENTE /**< Fuente fallida; lectura invalidada. */
} Temperatura_Estado;

/**
 * @brief Lectura por instancia; consultar ambos campos sin modificarlos directamente.
 * @details Legible tras inicializar/actualizar. Cada fuente valida la precision/rango
 * que le corresponden; el servicio no impone los de un dispositivo particular.
 */
typedef struct {
    int32_t centesimas_c; /**< Temperatura en centesimas Celsius; cero si no VALIDA. */
    Temperatura_Estado estado; /**< Distingue valor valido, ausencia y error. */
} Temperatura_Lectura;

/**
 * @brief Inicializar o reiniciar una lectura a SIN_DATOS y cero.
 * @param[out] lectura Salida escribible con acceso exclusivo; NULL se rechaza.
 * @return true si se inicializo; false si lectura es NULL, sin acceso a memoria.
 * @note USW.2.1.4.1. No inicializa ni selecciona dispositivos.
 */
bool Temperatura_Inicializar(Temperatura_Lectura *lectura);

/**
 * @brief Reemplazar la lectura con una medicion fisica o un estado generico de fuente.
 * @details FUENTE_VALIDA conserva exactamente centesimas_c, sin conversiones o
 * limites de un sensor concreto. FUENTE_SIN_DATOS produce SIN_DATOS/cero;
 * FUENTE_ERROR produce ERROR_FUENTE/cero. Estado desconocido produce
 * ERROR_ARGUMENTO/cero si la salida es escribible. Una nueva fuente valida recupera.
 * No lee estado anterior ni exige inicializacion previa para esta escritura.
 * @param[out] lectura Salida escribible serializada; NULL devuelve ERROR_ARGUMENTO.
 * @param[in] centesimas_c Magnitud fisica int32_t; se ignora si la fuente no es valida.
 * @param[in] fuente Estado normalizado del proveedor; no contiene codigos de dispositivo.
 * @return VALIDA, SIN_DATOS, ERROR_FUENTE o ERROR_ARGUMENTO; tambien escrito en
 * lectura->estado si la salida no es NULL.
 * @pre Fuente adaptada a Celsius/estado genericos; serializar escritura/lectura.
 * Consumir el campo numerico solo si estado es VALIDA.
 * @note USW.2.1.4.2. Sin adquisicion, heap, flotantes, esperas o hardware especifico.
 */
Temperatura_Estado Temperatura_Actualizar(Temperatura_Lectura *lectura,
                                        int32_t centesimas_c,
                                        Temperatura_EstadoFuente fuente);

/** @} */
#endif /* TPI_TEMPERATURA_H */
