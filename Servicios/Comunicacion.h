/**
 * @file Comunicacion.h
 * @brief Recepcion de consignas y codificacion de telemetria ASCII acotadas.
 * @details B02 recibe consignas; B09 prepara telemetria sin transmitir. Integrador entrega bytes
 * y orden global de recepcion; eventos propios del servicio mantienen esa marca en cola.
 * @note ESW.2.1.7. Sin heap; contexto inicializado, llamadas serializadas y sin solapamiento.
 */
#ifndef TPI_COMUNICACION_H
#define TPI_COMUNICACION_H

#include "Limites.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup comunicacion ESW.2.1.7 - Comunicacion
 * @brief Consignas RX y telemetria logica sin UART concreta.
 * @details Gramatica exacta: RPM, un espacio, 1..3 digitos, LF o CRLF. Admite ceros
 * iniciales, sin signos ni espacios extra. El orden del LF es el del evento completo.
 * La PC no asigna ese orden; se captura al recibir, nunca al retirar de la cola.
 * @{
 */

/** @brief Cantidad maxima de bytes antes de LF; incluye CR, no agrega NUL. */
#define COMUNICACION_TRAMA_MAX 8U
/** @brief Capacidad fija de la cola FIFO de consignas. */
#define COMUNICACION_COLA_CAPACIDAD 4U
/** @brief Maximo RPM del servicio, alias de la configuracion comun. */
#define COMUNICACION_RPM_MAX TPI_RPM_MAX

/** @brief Resultado explicito; el integrador debe observar los errores de recepcion. */
typedef enum {
    COMUNICACION_OK = 0, /**< Byte parcial o descartado hasta LF; sin evento nuevo. */
    COMUNICACION_ENCOLADA, /**< Trama valida agregada a la cola. */
    COMUNICACION_EXTRAIDA, /**< Se entrego exactamente un evento FIFO. */
    COMUNICACION_VACIA, /**< No hay eventos para extraer. */
    COMUNICACION_ERROR_ARGUMENTO, /**< Parametro requerido NULL. */
    COMUNICACION_ERROR_ORDEN, /**< Secuencia repetida o regresiva; trama invalidada. */
    COMUNICACION_ERROR_LONGITUD, /**< Buffer excedido; descartar hasta LF. */
    COMUNICACION_ERROR_FORMATO, /**< Trama completa no cumple la gramatica ASCII. */
    COMUNICACION_ERROR_RANGO, /**< Entero decimal mayor que el maximo RPM. */
    COMUNICACION_ERROR_COLA_LLENA /**< Trama nueva rechazada sin sobrescribir la FIFO. */
} Comunicacion_Resultado;

/** @brief Evento propio del servicio; la aplicacion lo adapta a su contrato de modos. */
typedef struct {
    uint32_t rpm; /**< Consigna validada, de 0 a 185 RPM inclusive. */
    uint64_t orden_recepcion; /**< Orden capturado al recibir el LF que completa la trama. */
} Comunicacion_Consigna;

/**
 * @brief Estado de una instancia; campos internos, no modificarlos directamente.
 * @details Inicializar antes de usar y serializar recepcion/extraccion. No es seguro
 * compartirlo entre ISR y lazo sin sincronizacion externa, incluida la captura de 64 bits.
 */
typedef struct {
    Comunicacion_Consigna cola[COMUNICACION_COLA_CAPACIDAD]; /**< Entradas ocupadas de FIFO. */
    uint64_t ultimo_orden; /**< Ultimo orden confiable recibido; no permite rollover. */
    uint8_t trama[COMUNICACION_TRAMA_MAX]; /**< Bytes parciales; nunca es un C string. */
    uint8_t longitud; /**< Cantidad de bytes validos de trama. */
    uint8_t cabeza; /**< Indice de la entrada FIFO mas antigua. */
    uint8_t cantidad; /**< Cantidad de entradas ocupadas, de 0 a 4. */
    bool descartando; /**< Ignorar resto de trama invalida hasta LF. */
} Comunicacion_Contexto;

/**
 * @brief Inicializar contexto y descartar toda trama/cola anterior.
 * @details Los bytes posteriores deben tener orden mayor que orden_inicial.
 * Reiniciar la numeracion requiere coordinar nueva sesion y purga con la aplicacion.
 * @param[out] contexto Memoria escribible; NULL no se accede.
 * @param[in] orden_inicial Ultima secuencia global anterior a esta instancia/sesion.
 * @return true si se inicializo; false ante contexto NULL.
 * @note USW.2.1.7.1. No configura UART ni captura ordenes por si sola.
 */
bool Comunicacion_Inicializar(Comunicacion_Contexto *contexto, uint64_t orden_inicial);

/**
 * @brief Incorporar un byte y validar/encolar cuando completa una trama.
 * @details Orden estrictamente creciente por byte. Una regresion/repeticion conserva
 * el ultimo orden confiable y la FIFO, invalida la trama y descarta hasta LF (incluido
 * el byte rechazado si es LF). Exceso de buffer informa una vez y descarta hasta LF.
 * Errores de formato/rango o cola llena se informan al LF; la trama siguiente puede
 * procesarse. Cola llena descarta la nueva consigna, conservando las anteriores.
 * @param[in,out] contexto Instancia inicializada y serializada, o NULL para error.
 * @param[in] byte Byte recibido; puede contener cualquier valor uint8_t.
 * @param[in] orden Secuencia global asignada al recibir; no es tiempo ni dato de PC.
 * @return OK si no hay evento/error nuevo; ENCOLADA si se agrego un evento;
 * ERROR_ARGUMENTO, ERROR_ORDEN, ERROR_LONGITUD, ERROR_FORMATO, ERROR_RANGO o
 * ERROR_COLA_LLENA segun la causa. Nunca EXTRAIDA o VACIA.
 * @pre Memoria valida y contexto inicializado. No permitir rollover del orden.
 * @note USW.2.1.7.2. Sin esperas bloqueantes ni acceso a perifericos.
 */
Comunicacion_Resultado Comunicacion_RecibirByte(Comunicacion_Contexto *contexto,
                                             uint8_t byte, uint64_t orden);

/**
 * @brief Extraer el evento FIFO mas antiguo conservando su orden de recepcion.
 * @details No genera marcas nuevas. Salida escribible se limpia si cola vacia o
 * contexto NULL. Un evento NULL devuelve error sin consumir nada.
 * @param[in,out] contexto Instancia inicializada, sin acceso concurrente; NULL es error.
 * @param[out] evento Memoria escribible valida que no solapa contexto; NULL es error.
 * @return EXTRAIDA, VACIA o ERROR_ARGUMENTO. No se consume ante vacio/error.
 * @note USW.2.1.7.3. Adaptar el evento en aplicacion, sin dependencia hacia ella.
 */
Comunicacion_Resultado Comunicacion_ExtraerConsigna(Comunicacion_Contexto *contexto,
                                                 Comunicacion_Consigna *evento);

/** @brief Almacenamiento maximo de telemetria: 53 bytes de trama mas NUL. */
#define COMUNICACION_TELEMETRIA_CAPACIDAD 54U

/** @brief Disponibilidad propia del protocolo, independiente del productor de la lectura. */
typedef enum {
    COMUNICACION_DATO_VALIDO = 0, /**< Consumir y codificar la magnitud, incluido cero. */
    COMUNICACION_DATO_SIN_DATOS, /**< Emitir ND, sin interpretar la magnitud. */
    COMUNICACION_DATO_ERROR /**< Emitir ERR, sin interpretar la magnitud. */
} Comunicacion_ValidezDato;

/** @brief Estado logico del sistema adaptado externamente al protocolo de telemetria. */
typedef enum {
    COMUNICACION_OPERACION_MANUAL = 0, /**< Texto MANUAL. */
    COMUNICACION_OPERACION_AUTO_ESPERA, /**< Texto AUTO_ESPERA, sin consigna habilitada. */
    COMUNICACION_OPERACION_AUTO_ACTIVO /**< Texto AUTO_ACTIVO, consigna habilitada. */
} Comunicacion_EstadoOperacion;

/**
 * @brief Instantanea logica propia de Comunicacion; el integrador adapta sus productores.
 * @details No contiene tipos de Aplicacion, parametros electricos o modelo de sensor.
 * Magnitudes expresadas en unidades enteras; no representa captura atomica o frescura.
 */
typedef struct {
    uint32_t rpm_milesimas; /**< Milesimas RPM, todo uint32_t si valido; no limitado a 185 RPM. */
    int32_t temperatura_centesimas_c; /**< Centesimas Celsius, todo int32_t si valido. */
    Comunicacion_ValidezDato validez_rpm; /**< Validez de velocidad; ND/ERR ignoran la magnitud. */
    Comunicacion_ValidezDato validez_temperatura; /**< Validez termica independiente de velocidad. */
    Comunicacion_EstadoOperacion estado_operacion; /**< MANUAL/AUTO_ESPERA/AUTO_ACTIVO propios del servicio. */
} Comunicacion_Telemetria;

/** @brief Resultado completo de codificacion, distinto de los resultados RX existentes. */
typedef enum {
    COMUNICACION_TELEMETRIA_OK = 0, /**< Trama completa con LF y NUL disponible en buffer. */
    COMUNICACION_TELEMETRIA_ERROR_ARGUMENTO, /**< Datos/buffer/longitud NULL. */
    COMUNICACION_TELEMETRIA_ERROR_ESTADO, /**< Validez u operacion fuera de sus enums publicos. */
    COMUNICACION_TELEMETRIA_ERROR_CAPACIDAD /**< No entra trama mas NUL, incluida capacidad cero. */
} Comunicacion_ResultadoTelemetria;

/**
 * @brief Codificar RPM, temperatura y estado como una trama ASCII completa.
 * @details Formato RPM=valor;TEMP=valor;ESTADO=texto seguido de LF, sin espacios o CR.
 * Velocidad valida lleva tres decimales; temperatura valida, dos y signo si negativa.
 * SIN_DATOS/ERROR generan ND/ERR sin consumir magnitudes. No recorta mediciones a
 * 185 RPM ni redondea datos ya normalizados. NUL adicional sirve al consumidor C.
 * Valida primero punteros, luego enums y finalmente capacidad. En cualquier error
 * pone longitud a cero si existe y buffer[0] a NUL si buffer existe/capacidad>0;
 * no modifica otros bytes de destino ni deja trama parcial utilizable. Puede recuperar
 * en siguiente solicitud valida, sin retener estado o necesitar inicializacion RX.
 * @param[in] datos Instantanea legible, coherente, estable y no solapada; NULL se rechaza.
 * @param[out] buffer Memoria escribible para capacidad bytes; NULL se rechaza incluso con cero.
 * @param[in] capacidad Espacio real disponible; cero no permite tocar buffer. 54 bytes
 * bastan para cualquier entrada valida; tramas menores admiten capacidad exacta con NUL.
 * @param[out] longitud Memoria escribible no solapada; recibe bytes incluyendo LF y
 * excluyendo NUL en exito, cero en error si existe. NULL produce ERROR_ARGUMENTO.
 * @return TELEMETRIA_OK o TELEMETRIA_ERROR_ARGUMENTO/ESTADO/CAPACIDAD segun prioridad.
 * @pre Datos, buffer y longitud vivos, sin solaparse y con acceso serializado; const
 * no garantiza atomicidad ni valida punteros no nulos. Integrador adapta estados sin
 * depender el servicio de Aplicacion y valida la captura de magnitudes previamente.
 * @note USW.2.1.7.7. Sin heap, flotantes, esperas, UART, temporizacion o estado global
 * mutable. No modifica datos ni contexto RX, no transmite y no confirma recepcion en PC.
 */
Comunicacion_ResultadoTelemetria Comunicacion_CodificarTelemetria(
    const Comunicacion_Telemetria *datos, char *buffer, size_t capacidad, size_t *longitud);

/** @} */
#endif /* TPI_COMUNICACION_H */
