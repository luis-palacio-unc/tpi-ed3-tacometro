/**
 * @file Mandos.h
 * @brief Mando manual normalizado y seleccion estable de modo, independientes de hardware.
 * @details Escala explicita, independiente de bits/VREF/canal del ADC. El mando
 * normalizado va de 0 a 1000 por mil, sin acceder a perifericos ni accionar PWM.
 * @note ESW.2.1.5. B10 filtra selector logico con tiempo externo; adquisicion/PWM pendientes.
 */
#ifndef TPI_MANDOS_H
#define TPI_MANDOS_H

#include "Limites.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup mandos ESW.2.1.5 - Mandos
 * @brief Mando manual y selector logico con validez explicita.
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

/** @brief Mayor intervalo/avance temporal admitido, en ms: estrictamente menor que 2^31. */
#define MANDOS_SELECTOR_INTERVALO_MAX_MS ((uint32_t)INT32_MAX)

/** @brief Modo logico independiente del nivel/polaridad del selector fisico. */
typedef enum {
    MANDOS_SELECTOR_MODO_NINGUNO = 0, /**< Salida sin modo utilizable; no es una muestra valida. */
    MANDOS_SELECTOR_MODO_MANUAL, /**< Seleccion logica manual. */
    MANDOS_SELECTOR_MODO_AUTOMATICO /**< Seleccion logica automatica. */
} Mandos_ModoSelector;

/** @brief Disponibilidad de la muestra, adaptada externamente desde su origen. */
typedef enum {
    MANDOS_SELECTOR_FUENTE_SIN_DATOS = 0, /**< No hay muestra disponible; ignorar modo. */
    MANDOS_SELECTOR_FUENTE_VALIDA, /**< Evaluar modo y estabilidad observada. */
    MANDOS_SELECTOR_FUENTE_ERROR /**< Fuente fallida; ignorar modo y perder confirmacion. */
} Mandos_FuenteSelector;

/** @brief Resultado completo de selector; solo VALIDO permite consumir modo. */
typedef enum {
    MANDOS_SELECTOR_SIN_DATOS = 0, /**< Sin muestra o candidato aun no confirmado. */
    MANDOS_SELECTOR_VALIDO, /**< Modo confirmado disponible, incluso al calificar otro. */
    MANDOS_SELECTOR_ERROR_ARGUMENTO, /**< Selector NULL o enum de fuente desconocido. */
    MANDOS_SELECTOR_ERROR_CONFIGURACION, /**< Intervalo superior al maximo; reinicializar. */
    MANDOS_SELECTOR_ERROR_FUENTE, /**< Fuente declaro fallo; modo/candidato invalidados. */
    MANDOS_SELECTOR_ERROR_MODO, /**< Fuente valida con modo distinto de MANUAL/AUTOMATICO. */
    MANDOS_SELECTOR_ERROR_TIEMPO /**< Avance modular fuera de contrato; se descarta anclaje. */
} Mandos_EstadoSelector;

/**
 * @brief Contexto de una instancia de selector; requiere inicializacion explicita.
 * @details El consumidor lee estado/modo sin modificarlos; demas campos son internos.
 * Memoria viva y acceso serializado entre actualizaciones/lecturas; sin proteccion ISR.
 * No compartir ni solapar con otros registros o modificar configuracion directamente.
 */
typedef struct {
    uint32_t intervalo_estable_ms; /**< Configuracion interna, 0..INT32_MAX ms; cambia al reiniciar. */
    uint32_t ultimo_tiempo_ms; /**< Ultima marca temporal aceptada; no es orden de eventos B01. */
    uint32_t inicio_candidato_ms; /**< Primera muestra del candidato actual, modulo uint32_t. */
    Mandos_ModoSelector modo; /**< Salida: consumir solo con estado VALIDO; NINGUNO en otro caso. */
    Mandos_ModoSelector candidato; /**< Modo interno en calificacion, o NINGUNO. */
    Mandos_EstadoSelector estado; /**< Disponibilidad/diagnostico actual, tambien devuelto al actualizar. */
    bool tiene_tiempo; /**< Hay anclaje temporal confiable para comparar proxima llamada. */
} Mandos_Selector;

/**
 * @brief Inicializar/reiniciar selector sin suponer modo fisico ni reloj inicial.
 * @details Borra confirmacion/candidato/anclaje. Intervalo valido deja SIN_DATOS y
 * modo NINGUNO; cero permite confirmar la primera muestra valida inmediatamente.
 * Intervalo excesivo deja objeto inicializado con ERROR_CONFIGURACION/NINGUNO;
 * actualizarlo conserva ese error hasta reinicializar con un intervalo valido.
 * @param[out] selector Contexto escribible exclusivo; NULL devuelve false sin acceso.
 * @param[in] intervalo_estable_ms Configuracion 0..INT32_MAX ms, sin valor TPI por defecto.
 * @return true si contexto/configuracion validos; false ante NULL o intervalo excesivo.
 * @pre Memoria valida no solapada con otros registros; acceso serializado.
 * @note USW.2.1.5.3. No inicializa GPIO/timer, ni modifica mando manual o motor.
 */
bool Mandos_InicializarSelector(Mandos_Selector *selector, uint32_t intervalo_estable_ms);

/**
 * @brief Actualizar disponibilidad y confirmar modo tras estabilidad observada.
 * @details Cada cambio de candidato reinicia su fecha; conserva modo confirmado
 * durante otro candidato pendiente. Antes de primera confirmacion retorna SIN_DATOS.
 * Igualar el intervalo ya confirma. Fuente ausente/fallida o error invalida modo y
 * candidato, exige nueva estabilidad y no elige un modo alternativo ni acciona motor.
 * Valida NULL, configuracion, enum de fuente, tiempo, disponibilidad y modo en ese
 * orden. Fuentes SIN_DATOS/ERROR ignoran modo pero registran timestamp aceptado;
 * modo invalido tambien conserva ese tiempo. Fuente desconocida no acepta tiempo.
 * ERROR_TIEMPO descarta anclaje; proxima llamada aceptada inicia nueva epoca y
 * candidato sin restaurar un modo anterior. Reinicializar tambien descarta la epoca.
 * @param[in,out] selector Contexto inicializado, vivo/estable y serializado; NULL es error.
 * @param[in] modo MANUAL/AUTOMATICO logicos; ignorado si fuente no es VALIDA.
 * @param[in] fuente Disponibilidad de muestra; enum desconocido es ERROR_ARGUMENTO.
 * @param[in] tiempo_ms Marca uint32_t externa en ms, con avance modulo 2^32 menor
 * que 2^31 desde la ultima aceptada. Iguales y rollover permitidos; primera libre.
 * @return Estado actual tambien escrito en selector->estado si existe; NULL retorna
 * ERROR_ARGUMENTO. Modo queda NINGUNO salvo VALIDO; leer ambos campos conjuntamente.
 * @pre Capturar tiempo/muestra coherentes y serializar escrituras/lecturas. Llamar
 * con suficiente cadencia: no detecta multiples vueltas omitidas, ausencia de llamadas
 * o rebotes entre muestras; uint32_t no hace atomico al contexto completo.
 * @note USW.2.1.5.4. Sin heap, reloj, esperas, globals mutables o dependencia de
 * Aplicacion. Politica fisica ante indisponibilidad y ensamblado de main pendientes.
 */
Mandos_EstadoSelector Mandos_ActualizarSelector(Mandos_Selector *selector,
    Mandos_ModoSelector modo, Mandos_FuenteSelector fuente, uint32_t tiempo_ms);

/** @} */
#endif /* TPI_MANDOS_H */
