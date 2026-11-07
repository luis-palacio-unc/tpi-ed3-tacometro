/**
 * @file Mandos.h
 * @brief Normalizacion logica de la lectura del potenciometro a mando manual.
 * @details Escala explicita, independiente de bits/VREF/canal del ADC. El mando
 * normalizado va de 0 a 1000 por mil, sin acceder a perifericos ni accionar PWM.
 * @note ESW.2.1.5. B04 no implementa selector, antirrebote, adquisicion o filtrado.
 */
#ifndef TPI_MANDOS_H
#define TPI_MANDOS_H

#include "Limites.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup mandos ESW.2.1.5 - Mandos
 * @brief Mando manual normalizado con validez explicita.
 * @details El integrador entrega lectura/escala coherentes y verifica errores de
 * adquisicion antes de llamar. Serializar escritura y lectura de cada salida;
 * no compartir directamente entre ISR y lazo. No se evalua frescura del dato.
 * @{
 */

/** @brief Maximo mando manual: 1000 por mil equivale al 100 por ciento. */
#define MANDOS_MANDO_MAX TPI_MANDO_MANUAL_MAX

/** @brief Disponibilidad/validez del mando o error explicito de la llamada. */
typedef enum {
    MANDOS_SIN_DATOS = 0, /**< Salida inicializada, sin conversion disponible. */
    MANDOS_VALIDO, /**< Conversion valida, incluido mando cero. */
    MANDOS_ERROR_ARGUMENTO, /**< Salida NULL, sin acceso a memoria. */
    MANDOS_ERROR_ESCALA, /**< Fondo de escala cero; salida invalidada. */
    MANDOS_ERROR_LECTURA /**< Lectura mayor que escala positiva; salida invalidada. */
} Mandos_Estado;

/**
 * @brief Salida de una instancia; consultar ambos campos, sin modificarlos directamente.
 * @details Legible tras inicializar o normalizar. Solo VALIDO permite consumir el
 * mando. No representa un PWM aplicado ni certifica la validez electrica del ADC.
 */
typedef struct {
    uint16_t por_mil; /**< Mando 0..1000; cero si estado no es VALIDO. */
    Mandos_Estado estado; /**< Distingue cero valido, ausencia y errores. */
} Mandos_MandoManual;

/**
 * @brief Inicializar o reiniciar el mando a SIN_DATOS y cero.
 * @param[out] mando Salida escribible valida y de acceso exclusivo; NULL se rechaza.
 * @return true si se inicializo; false si mando es NULL, sin acceso a memoria.
 * @note USW.2.1.5.1. No inicializa ADC, selector ni motor.
 */
bool Mandos_Inicializar(Mandos_MandoManual *mando);

/**
 * @brief Reemplazar el mando con la conversion lineal de lectura/escala o su error.
 * @details Mando = 1000 * lectura / fondo_escala, redondeado al mas cercano con
 * empate hacia arriba. Error aritmetico <=0,5 por mil, sin suponer precision del ADC.
 * Primero valida salida, luego escala y lectura. Escala cero o lectura excesiva
 * escriben cero/estado de error, sin saturar ni conservar el mando viejo como valido.
 * Una entrada valida posterior recupera la salida; puede cambiar la escala por llamada.
 * No lee contenido anterior y no requiere inicializacion previa para esta escritura.
 * @param[out] mando Salida escribible serializada; NULL devuelve ERROR_ARGUMENTO.
 * @param[in] lectura Conteo uint32_t coherente con la escala; debe ser <=fondo_escala.
 * @param[in] fondo_escala Valor maximo inclusivo, 1..UINT32_MAX cuentas; cero es error.
 * @return VALIDO, ERROR_ESCALA o ERROR_LECTURA, tambien escrito en mando->estado;
 * ERROR_ARGUMENTO si mando es NULL. Nunca devuelve SIN_DATOS.
 * @pre El llamador valida la adquisicion y serializa acceso a la salida. Leer el
 * valor para control solo cuando el estado es VALIDO; adaptar en la aplicacion.
 * @note USW.2.1.5.2. Sin dependencia hacia Aplicacion ni actualizacion del PWM.
 */
Mandos_Estado Mandos_NormalizarManual(Mandos_MandoManual *mando,
                                    uint32_t lectura, uint32_t fondo_escala);

/** @} */
#endif /* TPI_MANDOS_H */
