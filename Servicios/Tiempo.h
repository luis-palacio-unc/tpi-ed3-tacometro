/**
 * @file Tiempo.h
 * @brief Wrapper temporal independiente de microcontrolador y dispositivo.
 * @details Lee marcas uint32_t ms de un proveedor reemplazable y calcula intervalos.
 * No configura hardware ni interpreta cuentas, frecuencias o prescalers.
 * @note ESW.2.1.18, extension local del TPI; dos USW sin init/estado global/heap.
 */
#ifndef TPI_TIEMPO_H
#define TPI_TIEMPO_H
#include <stdbool.h>
#include <stdint.h>
/**
 * @defgroup tiempo ESW.2.1.18 - Tiempo
 * @brief Consultas de tiempo con estados propios y proveedor intercambiable.
 * @details Interfaz/contextos/destinos vivos, estables, serializados y no solapados.
 * El proveedor debe retornar; tiempo de llamada depende de el. No comparte memoria
 * con ISR ni retiene interfaz, valores o marcas de consumidores.
 * @{
 */
/** @brief Maximo intervalo disponible; exige duracion real menor a 2^31 ms. */
#define TIEMPO_INTERVALO_MAX_MS ((uint32_t)INT32_MAX)
/**
 * @brief Contrato del proveedor de marcas de una epoca temporal.
 * @param[in,out] contexto Estado opaco externo; NULL si el proveedor lo admite.
 * @param[out] milisegundos Destino local valido; proveedor debe escribir marca si retorna true.
 * @return true con marca ms valida modulo 2^32; false si no disponible (valor ignorado).
 * @pre Configuracion estable/misma epoca y retorno sin espera activa del proveedor.
 */
typedef bool (*Tiempo_ConsultarFuente)(void *contexto,uint32_t *milisegundos);
/** @brief Interfaz por llamada; el wrapper no modifica, registra ni libera sus campos. */
typedef struct {
    Tiempo_ConsultarFuente leer; /**< Funcion que produce marca en ms, no cuentas hardware. */
    void *contexto; /**< Contexto opaco pasado exactamente al proveedor. */
} Tiempo_Fuente;
/** @brief Disponibilidad de salida o diagnostico propio de la llamada. */
typedef enum {
    TIEMPO_NO_DISPONIBLE=0, /**< Proveedor no ofrece tiempo actual. */
    TIEMPO_VALIDO, /**< Marca/intervalo valido, incluido cero. */
    TIEMPO_ERROR_ARGUMENTO, /**< Destino/fuente/funcion ausentes; proveedor no invocado. */
    TIEMPO_ERROR_INTERVALO /**< Diferencia modular mayor que INT32_MAX ms. */
} Tiempo_Estado;
/** @brief Salida completa; consumir milisegundos solo con VALIDO. */
typedef struct {
    uint32_t milisegundos; /**< Marca o intervalo segun funcion; cero si no valido. */
    Tiempo_Estado estado; /**< Estado de esta llamada; no acredita precision fisica. */
} Tiempo_Lectura;
/**
 * @brief Consultar una marca temporal mediante un proveedor reemplazable.
 * @param[in] fuente Interfaz legible/estable; NULL/funcion ausente se rechazan.
 * @param[out] lectura Destino escribible exclusivo; NULL tiene primera prioridad.
 * @return VALIDO si proveedor retorna true; NO_DISPONIBLE si false, ignorando valor;
 * ERROR_ARGUMENTO si destino/interfaz/funcion invalidos, sin invocar proveedor.
 * @details Siempre reemplaza salida escribible. Interfaz valida invoca exactamente
 * una vez con contexto exacto; salidas no validas se limpian a cero. No requiere init.
 * @note USW.2.1.18.1. Sin dependencia de dispositivos, configuracion fisica o aplicacion.
 */
Tiempo_Estado Tiempo_Leer(const Tiempo_Fuente *fuente,Tiempo_Lectura *lectura);
/**
 * @brief Consultar tiempo y calcular intervalo modular desde una marca propia.
 * @param[in] fuente Proveedor del mismo origen/epoca que inicio_ms.
 * @param[in] inicio_ms Marca inicial uint32_t ms del consumidor, no modificada.
 * @param[out] intervalo Destino exclusivo; cero si ausencia/error, estado explicito.
 * @return Estados de Leer con misma prioridad; ERROR_INTERVALO/cero si delta>INT32_MAX,
 * VALIDO con delta en 0..INT32_MAX. Fuente/argumento se evaluan antes del intervalo.
 * @pre Misma epoca y duracion real <2^31 ms, sin vueltas completas omitidas. Reinicio
 * de fuente exige nuevas marcas; no se pueden detectar todas las violaciones.
 * @note USW.2.1.18.2. Lee proveedor una vez, no reinicia fuente ni guarda marcas.
 */
Tiempo_Estado Tiempo_Transcurrido(const Tiempo_Fuente *fuente,uint32_t inicio_ms,Tiempo_Lectura *intervalo);
/** @} */
#endif
