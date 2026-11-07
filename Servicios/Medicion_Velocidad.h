/**
 * @file Medicion_Velocidad.h
 * @brief Conversion generica de conteo y tiempo a velocidad de giro.
 * @details Factor pulsos/vuelta explicito, sin modelo de sensor o configuracion fisica.
 * Salida en milesimas de RPM, cercano/empate arriba, sin recortar a 185 RPM.
 * @note ESW.2.1.3. Sin captura, filtrado, timeout ni relacion de engranajes.
 */
#ifndef TPI_MEDICION_VELOCIDAD_H
#define TPI_MEDICION_VELOCIDAD_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup medicion_velocidad ESW.2.1.3 - Medicion de velocidad
 * @brief Velocidad y validez para conteo/tiempo/factor coherentes.
 * @details Resolucion numerica no acredita precision fisica. El llamador serializa
 * captura, configuracion, actualizacion y lectura; detecta overflow de adquisicion.
 * No compartir una lectura simultaneamente entre ISR y lazo principal.
 * @{
 */

/** @brief Unidades de salida por RPM; una unidad equivale a 0,001 RPM. */
#define MEDICION_VELOCIDAD_ESCALA_RPM 1000U

/** @brief Disponibilidad/validez de una lectura o error de la llamada. */
typedef enum {
    MEDICION_VELOCIDAD_SIN_DATOS = 0, /**< Inicializada, sin ventana procesada. */
    MEDICION_VELOCIDAD_VALIDA, /**< Ventana/factor positivos convertidos, incluido cero. */
    MEDICION_VELOCIDAD_ERROR_ARGUMENTO, /**< Salida NULL; no se accedio a memoria. */
    MEDICION_VELOCIDAD_ERROR_DURACION, /**< Duracion nula; lectura invalidada. */
    MEDICION_VELOCIDAD_ERROR_RANGO, /**< Resultado redondeado no cabe en uint32_t. */
    MEDICION_VELOCIDAD_ERROR_PULSOS_POR_VUELTA /**< Factor cero; lectura invalidada. */
} Medicion_Velocidad_Estado;

/**
 * @brief Salida por instancia; consultar ambos campos sin modificarlos directamente.
 * @details Legible tras inicializar/actualizar; sin marca temporal ni frescura.
 * Cada actualizacion reemplaza por completo la lectura anterior.
 */
typedef struct {
    uint32_t rpm_milesimas; /**< Milesimas de RPM; cero si estado no es VALIDA. */
    Medicion_Velocidad_Estado estado; /**< Validez explicita, independiente del valor. */
} Medicion_Velocidad_Lectura;

/**
 * @brief Inicializar o reiniciar una lectura a SIN_DATOS y cero.
 * @param[out] lectura Salida escribible sin acceso concurrente; NULL se rechaza.
 * @return true si se inicializo; false si lectura es NULL, sin acceso a memoria.
 * @note USW.2.1.3.1. No configura ni reinicia captura fisica.
 */
bool Medicion_Velocidad_Inicializar(Medicion_Velocidad_Lectura *lectura);

/**
 * @brief Reemplazar lectura con velocidad calculada o error explicito.
 * @details Milesimas = pulsos * 60000000000 / (duracion_us * pulsos_por_vuelta).
 * Cercano/empate arriba; error aritmetico <=0,0005 RPM, sin redondeo intermedio.
 * Resultado redondeado 0..UINT32_MAX, sin saturacion ni recorte de consigna.
 * Duracion/factor positivos con cero pulsos son cero valido. Prioridad: NULL,
 * duracion cero, factor cero y rango. Todo error escribible borra valor anterior;
 * la siguiente ventana valida recupera. No lee contenido anterior ni exige init.
 * Factor puede cambiar por llamada, manteniendo coherencia con el conteo recibido.
 * @param[out] lectura Salida escribible serializada; NULL devuelve ERROR_ARGUMENTO.
 * @param[in] pulsos Conteo uint32_t coherente, sin overflow ocurrido previamente.
 * @param[in] duracion_us Tiempo real transcurrido 1..UINT32_MAX us, no marca absoluta.
 * @param[in] pulsos_por_vuelta Factor 1..UINT32_MAX de la captura; cero es error.
 * @return VALIDA, ERROR_DURACION, ERROR_PULSOS_POR_VUELTA o ERROR_RANGO, tambien
 * escrito en lectura->estado; ERROR_ARGUMENTO si salida NULL. Nunca SIN_DATOS.
 * @pre Serializar captura/configuracion/acceso a salida; consumir solo VALIDA.
 * @note USW.2.1.3.2. Aritmetica uint64_t con seis pasos acotados; sin heap,
 * flotantes, esperas, dispositivos o dependencia de configuracion fisica.
 */
Medicion_Velocidad_Estado Medicion_Velocidad_Actualizar(
    Medicion_Velocidad_Lectura *lectura, uint32_t pulsos, uint32_t duracion_us,
    uint32_t pulsos_por_vuelta);

/** @} */
#endif /* TPI_MEDICION_VELOCIDAD_H */
