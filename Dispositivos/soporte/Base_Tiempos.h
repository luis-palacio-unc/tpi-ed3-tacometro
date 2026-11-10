/**
 * @file Base_Tiempos.h
 * @brief Base temporal por lectura del contador continuo de TIMER0.
 * @details Dispositivo singleton; periodo configurable, salida normalizada a ms.
 * No acumula tiempo por software ni utiliza una ISR. No configura el reloj PCLK.
 * @note ESW.2.1.17, extension local de soporte del TPI, dentro de Dispositivos.
 */
#ifndef TPI_BASE_TIEMPOS_H
#define TPI_BASE_TIEMPOS_H
#include <stdbool.h>
#include <stdint.h>
/**
 * @defgroup base_tiempos ESW.2.1.17 - Base temporal TC
 * @brief Propietario exclusivo de TIMER0, acceso serializado desde lazo privilegiado.
 * @details PCLK real/estable y periferico disponible son precondiciones externas.
 * No llamar desde ISR/NMI ni tareas concurrentes. Leer no detiene/reinicia el timer.
 * Reiniciar/reconfigurar abre nueva epoca; descartar marcas anteriores. No prometer
 * avance durante suspension/debug/clock gating ni precision fisica sin comprobarla.
 * @{
 */
/** @brief Maximo reloj PCLK admitido en Hz; no supone el reloj de la placa. */
#define BASE_TIEMPOS_PCLK_MAX_HZ UINT32_C(120000000)
/** @brief Periodo maximo ms compatible con contrato temporal de semirrango. */
#define BASE_TIEMPOS_PERIODO_MAX_MS ((uint32_t)INT32_MAX)
/** @brief Disponibilidad de la base o diagnostico de la llamada. */
typedef enum {
    BASE_TIEMPOS_SIN_DATOS=0, /**< Sin inicializar; no se consulta TC. */
    BASE_TIEMPOS_ACTIVA, /**< Timer habilitado; tiempo actual disponible. */
    BASE_TIEMPOS_DETENIDA, /**< Tiempo congelado, no reloj actual. */
    BASE_TIEMPOS_ERROR_ARGUMENTO, /**< Leer(NULL), sin efecto sobre la base. */
    BASE_TIEMPOS_ERROR_RELOJ, /**< PCLK cero o mayor al limite admitido. */
    BASE_TIEMPOS_ERROR_PERIODO, /**< Periodo cero o mayor a INT32_MAX ms. */
    BASE_TIEMPOS_ERROR_PRESCALER /**< Division inexacta o divisor fuera de 1..2^32. */
} Base_Tiempos_Estado;
/** @brief Instantanea de salida exclusiva del llamador; consumir actual solo ACTIVA. */
typedef struct {
    uint32_t milisegundos; /**< TC*periodo modulo 2^32; cero si ausencia/error. */
    Base_Tiempos_Estado estado; /**< Distingue cero activo, congelado y diagnosticos. */
} Base_Tiempos_Lectura;
/**
 * @brief Inicializar/reiniciar TIMER0 con PCLK y periodo explicitos.
 * @details N=PCLK*periodo/1000 exacto, 1..2^32; PR=N-1. Prioridad reloj, periodo,
 * prescaler. Todo intento prepara/deshabilita/resetea timer, incluso error.
 * Valido inicia TC/PC en cero, error deja base invalida/cero hasta reinicio valido.
 * @param[in] frecuencia_pclk_hz PCLK real/estable, 1..120000000 Hz, distinto de CPU.
 * @param[in] periodo_ms Unidad de cuenta entera 1..INT32_MAX ms, fija por epoca.
 * @return ACTIVA, ERROR_RELOJ, ERROR_PERIODO o ERROR_PRESCALER; estado conservado.
 * @pre Timer exclusivo; APIs y gestion de alimentacion PCONP serializadas desde lazo privilegiado.
 * @note USW.2.1.17.1. No modifica PLL/PCLKSEL/flash/pines. Cambiar epoca exige
 * reiniciar consumidores; aceptar configuracion no acredita precision electrica.
 */
Base_Tiempos_Estado Base_Tiempos_Inicializar(uint32_t frecuencia_pclk_hz,uint32_t periodo_ms);
/**
 * @brief Leer TC una vez y normalizar a milisegundos uint32_t modulares.
 * @details No modifica registros ni mantiene un contador software. ACTIVA y DETENIDA
 * consultan TC; SIN_DATOS/error entregan cero sin acceder al registro. Producto
 * modular intencional, no saturacion; mayor periodo significa menor resolucion.
 * @param[out] lectura Salida viva/escribible/exclusiva, no compartida con ISR.
 * @return Estado tambien escrito en lectura; ERROR_ARGUMENTO si NULL sin efectos propios.
 * @pre Lazo privilegiado serializado, periodo/configuracion estable y timer exclusivo.
 * @note USW.2.1.17.2. Una lectura alineada de 32 bits; solo ACTIVA habilita reloj actual.
 * No detecta vueltas completas omitidas ni que marcas pertenezcan a distintas epocas.
 */
Base_Tiempos_Estado Base_Tiempos_Leer(Base_Tiempos_Lectura *lectura);
/**
 * @brief Detener cuenta hardware conservando TC/periodo y diagnostico.
 * @details ACTIVA pasa a DETENIDA; los demas estados no cambian ni acceden al puerto.
 * Repetir parada es idempotente; solo reinicializar reanuda desde cero.
 * @return DETENIDA, SIN_DATOS o diagnostico de inicializacion anterior.
 * @pre Lazo privilegiado serializado y propiedad exclusiva del timer.
 * @note USW.2.1.17.3. No deshabilita reloj de otros perifericos ni toca IRQ globales.
 */
Base_Tiempos_Estado Base_Tiempos_Detener(void);
/**
 * @brief Proveer marca ms mediante firma generica bool(void*,uint32_t*).
 * @param[in] contexto Opaco ignorado, admite NULL; no se desreferencia ni retiene.
 * @param[out] milisegundos Destino escribible exclusivo; se limpia a cero si no disponible.
 * @return true solo con ACTIVA, incluso cero valido; false si NULL/no disponible.
 * @pre Condiciones de Leer; no compartir salida con ISR ni metadatos del dispositivo.
 * @note USW.2.1.17.9. Compatible con wrapper sin incluir contratos del servicio.
 * .17.4 a .17.8 del candidato SysTick quedan retiradas.
 */
bool Base_Tiempos_ProveerMilisegundos(void *contexto,uint32_t *milisegundos);
/** @} */
#endif
