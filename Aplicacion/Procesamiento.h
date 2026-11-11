/**
 * @file Procesamiento.h
 * @brief Logica de la aplicacion: modos de operacion, alarma termica y regulador PI.
 * @details Decide con magnitudes y estados logicos; no accede a perifericos ni
 * llama servicios. main.c le entrega las lecturas y aplica sus decisiones.
 * @note ESW.2.1.2. Sin heap; llamar solo desde el lazo principal.
 */
#ifndef TPI_PROCESAMIENTO_H
#define TPI_PROCESAMIENTO_H

#include "Limites.h"
#include "Mandos.h"
#include "Medicion_Velocidad.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup procesamiento ESW.2.1.2 - Procesamiento
 * @brief Modos manual/automatico, alarma con histeresis y regulacion de velocidad.
 * @{
 */

/** @brief Estado de operacion del tacometro. */
typedef enum {
    PROCESAMIENTO_MANUAL = 0, /**< El potenciometro fija el mando del motor. */
    PROCESAMIENTO_AUTO_ESPERA = 1, /**< Automatico sin consigna: mando cero. */
    PROCESAMIENTO_AUTO_ACTIVO = 2 /**< Automatico con consigna: regula el PI. */
} Procesamiento_Estado;

/**
 * @brief Fijar el estado de arranque segun la llave de modo.
 * @details MANUAL arranca en MANUAL; AUTOMATICO arranca en AUTO_ESPERA, sin
 * consigna.
 * @param[in] modo Modo leido de la llave al encender.
 * @return Estado inicial.
 * @note USW.2.1.2.11. Borra la consigna anterior.
 */
Procesamiento_Estado Procesamiento_InicializarModo(Mandos_Modo modo);

/**
 * @brief Actualizar el estado con la llave y la consigna recibida en el periodo.
 * @details Reglas, en este orden:
 * -# Con la llave en MANUAL el estado es MANUAL y se borra la consigna.
 * -# Al pasar de MANUAL a AUTOMATICO el estado es AUTO_ESPERA y la consigna de
 *    esta misma llamada se descarta: solo vale una consigna posterior al cambio.
 * -# En AUTO_ESPERA o AUTO_ACTIVO, una consigna de 0 a TPI_RPM_MAX (185) pasa a
 *    AUTO_ACTIVO y reemplaza a la anterior; una mayor se ignora.
 * @param[in] modo Modo leido de la llave en este periodo.
 * @param[in] hay_consigna true si se recibio una consigna en este periodo.
 * @param[in] consigna_rpm Consigna recibida en RPM; se ignora si hay_consigna es false.
 * @return Estado resultante.
 * @pre Procesamiento_InicializarModo ejecutado.
 * @note USW.2.1.2.12. Una llamada por periodo de 500 ms. Si el retorno pasa de
 * MANUAL a AUTO_ESPERA, main descarta la recepcion pendiente y reinicia el PI.
 */
Procesamiento_Estado Procesamiento_ActualizarModo(Mandos_Modo modo, bool hay_consigna,
                                                  uint32_t consigna_rpm);

/**
 * @brief Consultar la consigna vigente.
 * @return Consigna en RPM, de 0 a TPI_RPM_MAX; cero fuera de AUTO_ACTIVO.
 * @note USW.2.1.2.13. Sin efectos laterales.
 */
uint32_t Procesamiento_Consigna(void);

/**
 * @brief Decidir la alarma de temperatura con histeresis.
 * @details Apagada, se enciende si la temperatura supera
 * TPI_ALARMA_UMBRAL_CENTESIMAS (45,00 grados). Encendida, se apaga si la
 * temperatura es menor o igual al umbral menos TPI_ALARMA_HISTERESIS_CENTESIMAS
 * (43,00 grados). Entre ambos valores conserva la decision anterior.
 * @param[in] temperatura_centesimas Temperatura en centesimas de grado Celsius.
 * @return true si la alarma debe estar encendida.
 * @note USW.2.1.2.14. Arranca apagada. Una llamada por periodo de 500 ms. No
 * detiene el motor ni depende del modo.
 */
bool Procesamiento_EvaluarAlarma(int32_t temperatura_centesimas);

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
