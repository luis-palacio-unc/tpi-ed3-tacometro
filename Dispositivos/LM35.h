/**
 * @file LM35.h
 * @brief Conversion numerica de muestras ADC del LM35 a centesimas de grado Celsius.
 * @details Modelo ideal 10 mV por grado, sin offset. Escala y referencia explicitas;
 * no fija resolucion ni VREF reales, no adquiere muestras ni decide alarmas.
 * @note ESW.2.1.11. La validez numerica no acredita rango fisico, sensor o conexion.
 */
#ifndef TPI_LM35_H
#define TPI_LM35_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup lm35 ESW.2.1.11 - LM35
 * @brief Conversion logica del LM35 con disponibilidad y errores explicitos.
 * @details Escala ADC significa cuentas por referencia, no ultimo codigo inclusivo.
 * Por ejemplo, escala 4096 admite muestra 0..4095. El integrador valida adquisicion,
 * coherencia de muestra/configuracion y acceso serializado a cada salida.
 * No compartir directamente entre ISR y lazo; no se evalua frescura ni plausibilidad fisica.
 * @{
 */

/** @brief Unidades de salida por grado Celsius; una unidad equivale a 0,01 grados. */
#define LM35_CENTESIMAS_POR_GRADO 100U

/** @brief Disponibilidad/validez numerica de la lectura o error de la llamada. */
typedef enum {
    LM35_SIN_DATOS = 0, /**< Inicializada, sin conversion disponible. */
    LM35_VALIDA, /**< Conversion valida en el modelo, incluido cero ideal. */
    LM35_ERROR_ARGUMENTO, /**< Salida NULL; no se accedio a memoria. */
    LM35_ERROR_ESCALA, /**< Escala ADC cero; lectura invalidada. */
    LM35_ERROR_REFERENCIA, /**< Referencia de tension cero; lectura invalidada. */
    LM35_ERROR_MUESTRA /**< Muestra >= escala positiva; lectura invalidada. */
} LM35_Estado;

/**
 * @brief Salida por instancia; consultar ambos campos, sin modificarlos directamente.
 * @details Legible tras inicializar o actualizar; consumir solo VALIDA. Modelo no
 * negativo sin offset: no se implementan acondicionamiento, calibracion ni alarma.
 */
typedef struct {
    uint32_t centesimas_c; /**< Centesimas Celsius, 0..42949673 en dominio valido; cero si error/sin datos. */
    LM35_Estado estado; /**< Distingue cero ideal valido, ausencia y errores. */
} LM35_Lectura;

/**
 * @brief Inicializar o reiniciar una lectura a SIN_DATOS y cero.
 * @param[out] lectura Salida escribible con acceso exclusivo; NULL se rechaza.
 * @return true si se inicializo; false si lectura es NULL, sin acceder a memoria.
 * @note USW.2.1.11.1. No inicializa ADC, sensor ni subsistema de alarmas.
 */
bool LM35_InicializarLectura(LM35_Lectura *lectura);

/**
 * @brief Reemplazar la lectura con la conversion completa o con un error explicito.
 * @details Centesimas = muestra * referencia_uv / (escala_adc * 100). Redondeo al
 * mas cercano, empate arriba; error aritmetico <=0,005 grados Celsius. No redondea
 * primero a voltios/milivoltios/microvoltios enteros. No agrega media cuenta ADC.
 * La prioridad de errores es argumento, escala, referencia y muestra. Todo error
 * escribible borra valor anterior; una entrada valida posterior recupera la salida.
 * No lee contenido anterior ni exige inicializacion previa para esta escritura.
 * Cambiar escala/referencia por llamada esta permitido; no se impone rango fisico
 * de una variante no confirmada. VALIDA solo acredita la conversion numerica.
 * @param[out] lectura Salida escribible serializada; NULL devuelve ERROR_ARGUMENTO.
 * @param[in] muestra Codigo ADC uint32_t; debe pertenecer a 0..escala_adc-1.
 * @param[in] escala_adc Cuentas por referencia, 1..UINT32_MAX; cero es error.
 * @param[in] referencia_uv Referencia positiva en microvoltios, 1..UINT32_MAX; cero es error.
 * @return VALIDA, ERROR_ESCALA, ERROR_REFERENCIA o ERROR_MUESTRA, tambien escrito
 * en lectura->estado; ERROR_ARGUMENTO si salida NULL. Nunca devuelve SIN_DATOS.
 * @pre Captura/configuracion coherentes, adquisicion validada y acceso exclusivo
 * durante escritura/lectura. Solo consumir valor cuando estado sea VALIDA.
 * @note USW.2.1.11.2. Sin heap, flotantes, espera ni dependencia hacia Aplicacion.
 */
LM35_Estado LM35_ConvertirMuestra(LM35_Lectura *lectura,
                                        uint32_t muestra, uint32_t escala_adc,
                                        uint32_t referencia_uv);

/** @} */
#endif /* TPI_LM35_H */
