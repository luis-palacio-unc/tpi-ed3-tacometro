/**
 * @file Procesamiento.h
 * @brief Contrato de seleccion del mando y gestion de modos del tacometro.
 * @details B01 implementa la logica aislada de hardware. La integracion de servicios,
 * regulacion de velocidad y alarmas se incorporara en bloques posteriores.
 * @note ESW.2.1.2. Funciones serializadas, sin heap ni acceso a perifericos.
 */
#ifndef TPI_PROCESAMIENTO_H
#define TPI_PROCESAMIENTO_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup procesamiento ESW.2.1.2 - Procesamiento
 * @brief Gestion de modos y seleccion exclusiva de la fuente de mando.
 * @details El orden de eventos se captura al recibirlos y se conserva en cola.
 * No representa milisegundos y no permite rollover. Reiniciar la numeracion
 * requiere inicializar el contexto y purgar colas externas.
 * @{
 */

/** @brief Mando manual maximo: 1000 por mil equivale a 100 por ciento. */
#define PROCESAMIENTO_MANDO_MAX 1000U

/** @brief Modo logico entregado por el futuro servicio Mandos. */
typedef enum {
    PROCESAMIENTO_MODO_MANUAL = 0, /**< Seleccion del potenciometro. */
    PROCESAMIENTO_MODO_AUTOMATICO = 1 /**< Seleccion de consigna de RPM. */
} Procesamiento_Modo;

/** @brief Estados observables de la seleccion del mando. */
typedef enum {
    PROCESAMIENTO_MANUAL = 0, /**< Ruta manual habilitada. */
    PROCESAMIENTO_AUTO_ESPERA_CONSIGNA = 1, /**< Espera con salida cero. */
    PROCESAMIENTO_AUTO_ACTIVO = 2 /**< Consigna vigente para el regulador futuro. */
} Procesamiento_Estado;

/** @brief Una unica fuente seleccionada por cada actualizacion. */
typedef enum {
    PROCESAMIENTO_FUENTE_CERO = 0, /**< El integrador debe aplicar PWM cero. */
    PROCESAMIENTO_FUENTE_MANUAL = 1, /**< Usar exclusivamente el mando manual. */
    PROCESAMIENTO_FUENTE_AUTOMATICA = 2 /**< Habilitar el regulador futuro. */
} Procesamiento_Fuente;

/** @brief Evento de consigna recibido; no cambiar su marca al retirarlo de cola. */
typedef struct {
    uint32_t rpm; /**< Consigna en RPM; no fija el rango fisico del motor. */
    uint64_t orden_recepcion; /**< Secuencia global capturada al recibir el mensaje. */
    bool valida; /**< Validacion externa de formato y rango; false descarta el evento. */
} Procesamiento_Consigna;

/** @brief Instantanea de entradas estables durante una llamada serializada. */
typedef struct {
    Procesamiento_Modo modo; /**< Seleccion logica, sin antirrebote en B01. */
    uint16_t mando_manual_por_mil; /**< 0..1000; se ignora por completo en automatico. */
    uint64_t orden_actual; /**< Orden no regresivo; engloba los eventos observados. */
    const Procesamiento_Consigna *consigna; /**< NULL si no hay evento; ignorado en manual. */
} Procesamiento_Entradas;

/** @brief Resultado logico; no acciona hardware ni calcula PWM automatico. */
typedef struct {
    Procesamiento_Estado estado; /**< Estado resultante de la maquina. */
    Procesamiento_Fuente fuente; /**< Unica fuente autorizada. */
    uint16_t mando_manual_por_mil; /**< Solo significativo en MANUAL; cero en otra fuente. */
    uint32_t consigna_rpm; /**< Solo significativa en AUTOMATICA; cero en otra fuente. */
} Procesamiento_Salida;

/**
 * @brief Memoria de una instancia; sus campos son internos, no modificarlos directamente.
 * @details Inicializar antes de usar. No copiar sobre entradas/salida ni compartir entre
 * interrupcion y lazo principal sin serializacion externa. No consulta el reloj del MCU.
 */
typedef struct {
    Procesamiento_Estado estado; /**< Estado interno vigente. */
    uint64_t orden_actual; /**< Ultima secuencia confiable observada. */
    uint64_t orden_entrada_auto; /**< Barrera que una consigna nueva debe superar. */
    uint64_t orden_ultima_consigna; /**< Ultima consigna aceptada, evita repeticion/regresion. */
    uint32_t consigna_rpm; /**< RPM retenidas para la ruta automatica. */
    bool requiere_entrada_auto; /**< Un error exige establecer otra barrera de entrada. */
} Procesamiento_Contexto;

/**
 * @brief Inicializar una instancia con modo y orden de partida.
 * @details Manual habilita su ruta en la primera actualizacion. Automatico inicia en
 * espera. Un modo invalido deja espera y exige una entrada posterior valida.
 * @param[out] contexto Memoria escribible, o NULL para devolver error sin acceso.
 * @param[in] modo Modo inicial; solo MANUAL o AUTOMATICO.
 * @param[in] orden Secuencia inicial; consignas anteriores/iguales se descartan.
 * @return true si contexto y modo son validos; false en otro caso.
 * @pre Al reiniciar, purgar las colas externas antes de reutilizar la secuencia.
 * @note USW.2.1.2.1. No es reentrante sobre un mismo contexto.
 */
bool Procesamiento_Inicializar(Procesamiento_Contexto *contexto,
                              Procesamiento_Modo modo, uint64_t orden);

/**
 * @brief Actualizar el modo y seleccionar una fuente de mando.
 * @details En automatico, aceptar solo eventos validos con orden mayor que entrada
 * y ultima consigna, y no mayor que orden_actual. Invalidos/antiguos/duplicados/futuros
 * no sustituyen la consigna activa. Un cambio de modo tiene precedencia: un evento
 * de la misma actualizacion no habilita automatico. RPM cero es una consigna valida
 * si el servicio la valida; UINT32_MAX se transporta, no habilita ese rango fisico.
 * @param[in,out] contexto Instancia inicializada; NULL devuelve error.
 * @param[in] entradas Datos estables y no solapados; NULL devuelve error.
 * @param[out] salida Resultado escribible no solapado; NULL devuelve error.
 * @return true para actualizacion valida (incluso si descarta consigna); false ante
 * modo/orden/mando manual invalido o parametro NULL. Si salida existe, un error
 * la deja en fuente CERO; contexto existente invalida su consigna y exige nueva
 * entrada automatica. La regresion de orden conserva el ultimo orden confiable.
 * @pre Serializar llamadas y captura de ordenes. No usar contexto sin inicializar.
 * @note USW.2.1.2.2. El integrador debe aplicar salida CERO y comprobar el retorno.
 * La fuente AUTOMATICA habilita un controlador futuro, no proporciona su PWM.
 */
bool Procesamiento_Actualizar(Procesamiento_Contexto *contexto,
                             const Procesamiento_Entradas *entradas,
                             Procesamiento_Salida *salida);

/** @} */
#endif /* TPI_PROCESAMIENTO_H */
