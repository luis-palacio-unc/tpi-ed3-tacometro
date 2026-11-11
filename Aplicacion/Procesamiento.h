/**
 * @file Procesamiento.h
 * @brief Gestion de modos y evaluacion logica de alarma de temperatura.
 * @details B01 selecciona la fuente de mando; B06 evalua temperatura/validez del
 * servicio generico contra un umbral configurable. B17 agrega PI separado; sin actuacion.
 * @note ESW.2.1.2. Funciones serializadas, sin heap ni acceso a perifericos.
 */
#ifndef TPI_PROCESAMIENTO_H
#define TPI_PROCESAMIENTO_H

#include "Limites.h"
#include "Temperatura.h"
#include "Medicion_Velocidad.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup procesamiento ESW.2.1.2 - Procesamiento
 * @brief Gestion de modos/fuente de mando y evaluacion termica independiente.
 * @details El orden de eventos se captura al recibirlos y se conserva en cola.
 * No representa milisegundos y no permite rollover. Reiniciar la numeracion
 * requiere inicializar el contexto y purgar colas externas.
 * @{
 */

/** @brief Mando manual maximo: 1000 por mil equivale a 100 por ciento. */
#define PROCESAMIENTO_MANDO_MAX TPI_MANDO_MANUAL_MAX

/** @brief Maxima consigna aceptada en RPM, inclusive; aclaracion del usuario. */
#define PROCESAMIENTO_RPM_MAX TPI_RPM_MAX

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
    uint32_t rpm; /**< Consigna en RPM: 0..185 inclusive; valores superiores se descartan. */
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
 * si el servicio la valida. Consignas mayores que 185 RPM se descartan aunque
 * valida sea true; no se saturan ni sustituyen la consigna vigente.
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

/** @brief Decision termica completa; solo ACTIVA/INACTIVA son evaluaciones disponibles. */
typedef enum {
    PROCESAMIENTO_ALARMA_SIN_DATOS = 0, /**< Lectura todavia no disponible. */
    PROCESAMIENTO_ALARMA_INACTIVA, /**< Lectura valida igual o inferior al umbral. */
    PROCESAMIENTO_ALARMA_ACTIVA, /**< Lectura valida estrictamente superior al umbral. */
    PROCESAMIENTO_ALARMA_ERROR_FUENTE, /**< Fuente fallida; no representa temperatura normal. */
    PROCESAMIENTO_ALARMA_ERROR_ARGUMENTO /**< NULL, estado desconocido o error de argumento del servicio. */
} Procesamiento_EstadoAlarma;

/**
 * @brief Evaluar alarma de temperatura usando lectura generica y umbral configurable.
 * @details Con TEMPERATURA_VALIDA retorna ACTIVA solo si centesimas_c>umbral;
 * igual/inferior retorna INACTIVA. SIN_DATOS, ERROR_FUENTE y ERROR_ARGUMENTO se
 * conservan como categorias de alarma distintas; estado desconocido es argumento
 * invalido. No usa el campo numerico de lecturas no validas ni guarda decision previa.
 * El llamador reemplaza su decision con el retorno; solo ACTIVA/INACTIVA permiten
 * interpretar la comparacion. Otros estados no ordenan apagar/encender actuadores.
 * @param[in] lectura Lectura del servicio legible/coherente; NULL produce ERROR_ARGUMENTO.
 * @param[in] umbral_centesimas_c Configuracion int32_t en centesimas Celsius,
 * sin valor por defecto. Puede cambiar por llamada y admite todo el dominio int32_t.
 * @return ACTIVA, INACTIVA, SIN_DATOS, ERROR_FUENTE o ERROR_ARGUMENTO segun lectura;
 * NULL/estado desconocido devuelven ERROR_ARGUMENTO sin evaluacion numerica.
 * @pre Serializar captura/escritura/lectura del registro; const no garantiza atomicidad.
 * No pasar memoria no inicializada ni un puntero invalido distinto de NULL.
 * @note USW.2.1.2.7. Sin estado mutable, heap, reloj, esperas o resta que pueda
 * desbordar. No modifica lectura ni B01/motor, no detecta frescura ni acciona alarma fisica.
 */
Procesamiento_EstadoAlarma Procesamiento_EvaluarAlarmaTemperatura(
    const Temperatura_Lectura *lectura, int32_t umbral_centesimas_c);

/** @brief Ganancias discretas por instancia; construir defaults con Control.h. */
typedef struct {
    uint32_t kp_num; /**< Numerador proporcional, 1..1000. */
    uint32_t kp_den; /**< Denominador proporcional, 1..1000. */
    uint32_t ki_num; /**< Numerador integral por paso de 500 ms, 1..1000. */
    uint32_t ki_den; /**< Denominador integral, 1..1000. */
    uint32_t max_periodos_sin_medicion; /**< Retenciones consecutivas admitidas, 1..65535. */
} Procesamiento_ConfigRegulador;

/**
 * @brief Memoria del PI, independiente de Procesamiento_Contexto.
 * @details Campos internos: no modificarlos. Inicializar incluso antes de reiniciar;
 * una inicializacion fallida tambien deja memoria definida pero deshabilitada.
 * No copiar sobre entradas/salida ni compartir sin serializacion externa.
 */
typedef struct {
    int64_t integral_num; /**< Integral por ki_den; entre 0 y mando_max*ki_den. */
    Procesamiento_ConfigRegulador config; /**< Copia privada de configuracion validada. */
    uint32_t ultima_valida; /**< Ultima velocidad valida completa, en milesimas RPM. */
    uint32_t periodos_invalidos; /**< Racha retenida, entre 0 y N, sin rollover. */
    bool configurado; /**< true solo tras inicializacion valida. */
    bool tiene_medicion; /**< Distingue historia ausente de velocidad cero valida. */
} Procesamiento_Regulador;

/** @brief Resultado actual del calculo, sin confirmar actuacion del motor. */
typedef enum {
    PROCESAMIENTO_REGULADOR_VALIDO = 0, /**< Uso de medicion actual valida. */
    PROCESAMIENTO_REGULADOR_RETENIENDO_MEDICION, /**< Usa historia sin integrar. */
    PROCESAMIENTO_REGULADOR_SIN_MEDICION, /**< Sin historia o limite superado; mando cero. */
    PROCESAMIENTO_REGULADOR_PARADO, /**< Consigna cero, dinamica reiniciada. */
    PROCESAMIENTO_REGULADOR_ERROR_ARGUMENTO, /**< Algun puntero NULL, prioridad maxima. */
    PROCESAMIENTO_REGULADOR_ERROR_CONFIGURACION, /**< Inicializacion previa fallida. */
    PROCESAMIENTO_REGULADOR_ERROR_CONSIGNA /**< Consigna mayor que TPI_RPM_MAX. */
} Procesamiento_EstadoRegulador;

/**
 * @brief Copiar ganancias validas y limpiar la dinamica de una instancia.
 * @details Componentes num/den 1..1000, retencion 1..65535. Ki ya esta discretizada
 * para 500 ms. Sin configuracion valida deja todo definido, deshabilitado y sin historia.
 * @param[out] r Instancia escribible; NULL devuelve false sin acceso.
 * @param[in] cfg Configuracion legible estable; NULL se rechaza. No se retiene el puntero.
 * @return true si ambos punteros y valores son validos; false en otro caso.
 * @pre Punteros vivos y no solapados; llamadas serializadas. Defaults no son sintonia real.
 * @note USW.2.1.2.8. No cambia B01/B06 ni inicializa perifericos.
 */
bool Procesamiento_InicializarRegulador(Procesamiento_Regulador *r,
                                      const Procesamiento_ConfigRegulador *cfg);

/**
 * @brief Calcular un paso PI en por mil, con saturacion y anti-windup.
 * @details Error en milesimas RPM, sin recortar medicion a 185. P=e*kp_num/kp_den,
 * I=integral_num/ki_den. Mando usa I anterior, limita 0..1000 y redondea una sola
 * vez al cercano/empate arriba. Luego integra solo con medicion VALIDA, excepto
 * si v>=1000/e>0 o v<=0/e<0; I se limita a 0..1000.
 * No VALIDA (incluido enum desconocido) ignora el campo numerico: reutiliza ultima
 * valida durante N periodos consecutivos, sin integrar. Sin historia o en N+1,
 * mando cero y reinicio de integral/historia/contador. Nueva valida corta la racha.
 * Prioridad: NULL, configuracion, consigna>185, consigna0, medicion.
 * Todo error pone mando escribible a cero y reinicia dinamica de r existente;
 * consigna0 devuelve PARADO/cero y tambien descarta historia.
 * @param[in,out] r Instancia inicializada (incluso por un intento fallido), serializada.
 * @param[in] consigna_rpm RPM enteras 0..TPI_RPM_MAX; exceso es error, no saturacion.
 * @param[in] medicion Instantanea coherente; NULL es error aun con consigna cero.
 * @param[out] mando_por_mil Salida escribible no solapada, 0..TPI_MANDO_MANUAL_MAX.
 * @return Estado actual; VALIDO y RETENIENDO_MEDICION permiten mando calculado.
 * @pre Llamador garantiza un paso por ventana nueva de 500 ms, sin recuperar pasos
 * con muestras viejas. Punteros vivos, memoria definida y acceso serializado externo.
 * @note USW.2.1.2.9. Productos int64_t: |n|<=4294968295000000, d<=1000000;
 * suma integral en modulo<=4294968295000. Sin float, heap, esperas o perifericos.
 * No detecta frescura, sensor desconectado, bloqueo ni fallo del actuador.
 */
Procesamiento_EstadoRegulador Procesamiento_Regular(
    Procesamiento_Regulador *r, uint32_t consigna_rpm,
    const Medicion_Velocidad_Lectura *medicion, uint16_t *mando_por_mil);

/**
 * @brief Borrar integral, medicion retenida y contador conservando configuracion.
 * @param[in,out] r Instancia definida tras inicializar (incluso si fallo); NULL se rechaza.
 * @return true si r existe; false para NULL, sin acceso.
 * @pre Serializar acceso. No repara configuracion fallida ni inicializa memoria arbitraria.
 * @note USW.2.1.2.10. Integrador externo llama al salir de automatico o ante fuente CERO.
 */
bool Procesamiento_ReiniciarRegulador(Procesamiento_Regulador *r);

/** @} */
#endif /* TPI_PROCESAMIENTO_H */
