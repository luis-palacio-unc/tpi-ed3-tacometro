/**
 * @file Medicion_Velocidad.h
 * @brief Conversion logica de pulsos/tiempo a velocidad del eje portadisco.
 * @details Disco de 60 ranuras, un flanco por ranura. Salida en milesimas de RPM,
 * redondeada al mas cercano con empate hacia arriba. No limita lecturas a 185 RPM.
 * @note ESW.2.1.3. Sin captura fisica, filtrado, timeout ni relacion de engranajes.
 */
#ifndef TPI_MEDICION_VELOCIDAD_H
#define TPI_MEDICION_VELOCIDAD_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup medicion_velocidad ESW.2.1.3 - Medicion de velocidad
 * @brief Velocidad y validez explicitas para una ventana de captura coherente.
 * @details La resolucion numerica no acredita precision del sensor ni del reloj.
 * El llamador serializa captura, actualizacion y lectura; detecta desbordes de la
 * adquisicion antes de entregar pulsos/tiempo. No usar simultaneamente desde ISR/lazo.
 * @{
 */

/** @brief Pulsos por vuelta del disco: contar un solo flanco de cada ranura. */
#define MEDICION_VELOCIDAD_PULSOS_POR_VUELTA 60U
/** @brief Unidades de salida por RPM; cada unidad equivale a 0,001 RPM. */
#define MEDICION_VELOCIDAD_ESCALA_RPM 1000U

/** @brief Disponibilidad/validez de una lectura o error de la llamada. */
typedef enum {
    MEDICION_VELOCIDAD_SIN_DATOS = 0, /**< Inicializada, sin ventana procesada. */
    MEDICION_VELOCIDAD_VALIDA, /**< Ventana positiva convertida, incluso cero RPM. */
    MEDICION_VELOCIDAD_ERROR_ARGUMENTO, /**< Salida NULL; no se accedio a memoria. */
    MEDICION_VELOCIDAD_ERROR_DURACION, /**< Duracion nula; lectura invalidada. */
    MEDICION_VELOCIDAD_ERROR_RANGO /**< Resultado redondeado no cabe en uint32_t. */
} Medicion_Velocidad_Estado;

/**
 * @brief Salida por instancia; consultar ambos campos, sin modificarlos directamente.
 * @details Legible tras inicializar o actualizar. No contiene marca temporal ni
 * detecta envejecimiento; una ventana nueva reemplaza completamente la anterior.
 */
typedef struct {
    uint32_t rpm_milesimas; /**< Milesimas de RPM; cero si estado no es VALIDA. */
    Medicion_Velocidad_Estado estado; /**< Validez explicita, independiente del valor. */
} Medicion_Velocidad_Lectura;

/**
 * @brief Inicializar o reiniciar una lectura a SIN_DATOS y cero.
 * @param[out] lectura Memoria escribible valida, sin acceso concurrente; NULL se rechaza.
 * @return true si se inicializo; false si lectura es NULL, sin acceder a memoria.
 * @note USW.2.1.3.1. Reinicia disponibilidad; no configura ni reinicia la captura fisica.
 */
bool Medicion_Velocidad_Inicializar(Medicion_Velocidad_Lectura *lectura);

/**
 * @brief Reemplazar la lectura con la conversion de una ventana o con su error.
 * @details Para 60 pulsos/vuelta: milesimas = pulsos * 1 000 000 000 / duracion_us.
 * Redondeo al mas cercano, empate arriba; error numerico <=0,0005 RPM. El resultado
 * redondeado debe caber en uint32_t (hasta 4 294 967,295 RPM); no se satura.
 * Ventana positiva sin pulsos y valores pequenos que redondean a cero son validos.
 * Duracion cero o exceso reemplazan valor viejo por cero y estado de error.
 * La siguiente ventana valida recupera la lectura. No lee el contenido anterior.
 * @param[out] lectura Salida escribible serializada; NULL devuelve ERROR_ARGUMENTO.
 * @param[in] pulsos Conteo uint32_t coherente, sin overflow previo, un flanco/ranura.
 * @param[in] duracion_us Duracion real transcurrida, 1..UINT32_MAX microsegundos;
 * cero devuelve error. No es una marca absoluta ni se calcula rollover aqui.
 * @return VALIDA, ERROR_DURACION o ERROR_RANGO, tambien escrito en lectura->estado;
 * ERROR_ARGUMENTO si lectura es NULL. Nunca devuelve SIN_DATOS.
 * @pre Captura coherente y acceso exclusivo durante escritura y lectura de la salida.
 * @note USW.2.1.3.2. Solo aritmetica entera; no garantiza frescura ni tiempos fisicos.
 */
Medicion_Velocidad_Estado Medicion_Velocidad_Actualizar(
    Medicion_Velocidad_Lectura *lectura, uint32_t pulsos, uint32_t duracion_us);

/** @} */
#endif /* TPI_MEDICION_VELOCIDAD_H */
