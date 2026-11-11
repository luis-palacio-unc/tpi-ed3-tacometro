/**
 * @file Procesamiento.c
 * @brief Seleccion de mando, transiciones y decision termica sin actuacion fisica.
 * @details Sin perifericos, memoria dinamica, reloj o estado global mutable.
 * @note ESW.2.1.2. Contratos publicos en Procesamiento.h.
 */
#include "Procesamiento.h"
#include "Control.h"
#include <stddef.h>

/** @addtogroup procesamiento
 * @{
 */

/**
 * @brief Comprobar el dominio del selector logico.
 * @details Rechaza cualquier valor que no represente uno de los dos modos.
 * @param[in] modo Valor recibido del servicio de mandos.
 * @return true solo para manual o automatico.
 * @note USW.2.1.2.3. Sin efectos laterales.
 */
static bool ModoValido(Procesamiento_Modo modo)
{
    return modo == PROCESAMIENTO_MODO_MANUAL || modo == PROCESAMIENTO_MODO_AUTOMATICO;
}

/**
 * @brief Comprobar el estado interno de una instancia inicializada.
 * @details Valida el dominio antes de decidir transiciones y habilitar mandos.
 * @param[in] estado Estado a validar.
 * @return true si pertenece a los tres estados de B01.
 * @note USW.2.1.2.4. No acredita validez de memoria no inicializada.
 */
static bool EstadoValido(Procesamiento_Estado estado)
{
    return estado == PROCESAMIENTO_MANUAL ||
           estado == PROCESAMIENTO_AUTO_ESPERA_CONSIGNA ||
           estado == PROCESAMIENTO_AUTO_ACTIVO;
}

/**
 * @brief Invalidar consigna y establecer una barrera de recuperacion.
 * @details Borra las RPM retenidas, reinicia las marcas y exige una nueva entrada.
 * @param[in,out] contexto Instancia escribible no nula.
 * @param[in] orden Ultima secuencia confiable, sin incremento ni rollover.
 * @pre contexto no es NULL.
 * @note USW.2.1.2.5. La siguiente entrada automatica establece otra barrera.
 */
static void Invalidar(Procesamiento_Contexto *contexto, uint64_t orden)
{
    contexto->estado = PROCESAMIENTO_AUTO_ESPERA_CONSIGNA;
    contexto->orden_actual = orden;
    contexto->orden_entrada_auto = orden;
    contexto->orden_ultima_consigna = orden;
    contexto->consigna_rpm = 0U;
    contexto->requiere_entrada_auto = true;
}

/**
 * @brief Dejar una salida completa en espera y fuente cero.
 * @details Escribe todos los campos logicos antes de validar entradas o devolver error.
 * @param[out] salida Memoria escribible no nula.
 * @pre salida no es NULL.
 * @note USW.2.1.2.6. Evita reutilizar campos de una salida anterior.
 */
static void LimpiarSalida(Procesamiento_Salida *salida)
{
    salida->estado = PROCESAMIENTO_AUTO_ESPERA_CONSIGNA;
    salida->fuente = PROCESAMIENTO_FUENTE_CERO;
    salida->mando_manual_por_mil = 0U;
    salida->consigna_rpm = 0U;
}

bool Procesamiento_Inicializar(Procesamiento_Contexto *contexto,
                              Procesamiento_Modo modo, uint64_t orden)
{
    if (contexto == NULL) {
        return false;
    }
    Invalidar(contexto, orden);
    if (!ModoValido(modo)) {
        return false;
    }
    contexto->requiere_entrada_auto = false;
    if (modo == PROCESAMIENTO_MODO_MANUAL) {
        contexto->estado = PROCESAMIENTO_MANUAL;
    }
    return true;
}

bool Procesamiento_Actualizar(Procesamiento_Contexto *contexto,
                             const Procesamiento_Entradas *entradas,
                             Procesamiento_Salida *salida)
{
    if (salida != NULL) {
        LimpiarSalida(salida);
    }
    if (contexto == NULL) {
        return false;
    }
    if (entradas == NULL || salida == NULL) {
        Invalidar(contexto, contexto->orden_actual);
        return false;
    }
    uint64_t orden_confiable = entradas->orden_actual > contexto->orden_actual ?
                              entradas->orden_actual : contexto->orden_actual;
    if (!ModoValido(entradas->modo) || !EstadoValido(contexto->estado) ||
        entradas->orden_actual < contexto->orden_actual) {
        Invalidar(contexto, orden_confiable);
        return false;
    }
    contexto->orden_actual = entradas->orden_actual;
    if (entradas->modo == PROCESAMIENTO_MODO_MANUAL) {
        if (entradas->mando_manual_por_mil > PROCESAMIENTO_MANDO_MAX) {
            Invalidar(contexto, orden_confiable);
            return false;
        }
        Invalidar(contexto, orden_confiable);
        contexto->estado = PROCESAMIENTO_MANUAL;
        contexto->requiere_entrada_auto = false;
        salida->estado = PROCESAMIENTO_MANUAL;
        salida->fuente = PROCESAMIENTO_FUENTE_MANUAL;
        salida->mando_manual_por_mil = entradas->mando_manual_por_mil;
        return true;
    }
    if (contexto->estado == PROCESAMIENTO_MANUAL || contexto->requiere_entrada_auto) {
        Invalidar(contexto, orden_confiable);
        contexto->requiere_entrada_auto = false;
    }
    const Procesamiento_Consigna *consigna = entradas->consigna;
    if (consigna != NULL && consigna->valida &&
        consigna->rpm <= PROCESAMIENTO_RPM_MAX &&
        consigna->orden_recepcion > contexto->orden_entrada_auto &&
        consigna->orden_recepcion > contexto->orden_ultima_consigna &&
        consigna->orden_recepcion <= contexto->orden_actual) {
        contexto->consigna_rpm = consigna->rpm;
        contexto->orden_ultima_consigna = consigna->orden_recepcion;
        contexto->estado = PROCESAMIENTO_AUTO_ACTIVO;
    }
    salida->estado = contexto->estado;
    if (contexto->estado == PROCESAMIENTO_AUTO_ACTIVO) {
        salida->fuente = PROCESAMIENTO_FUENTE_AUTOMATICA;
        salida->consigna_rpm = contexto->consigna_rpm;
    }
    return true;
}

Procesamiento_EstadoAlarma Procesamiento_EvaluarAlarmaTemperatura(
    const Temperatura_Lectura *lectura, int32_t umbral_centesimas_c)
{
    if (lectura == NULL) {
        return PROCESAMIENTO_ALARMA_ERROR_ARGUMENTO;
    }
    switch (lectura->estado) {
        case TEMPERATURA_VALIDA:
            return lectura->centesimas_c > umbral_centesimas_c ?
                   PROCESAMIENTO_ALARMA_ACTIVA : PROCESAMIENTO_ALARMA_INACTIVA;
        case TEMPERATURA_SIN_DATOS:
            return PROCESAMIENTO_ALARMA_SIN_DATOS;
        case TEMPERATURA_ERROR_FUENTE:
            return PROCESAMIENTO_ALARMA_ERROR_FUENTE;
        case TEMPERATURA_ERROR_ARGUMENTO:
        default:
            return PROCESAMIENTO_ALARMA_ERROR_ARGUMENTO;
    }
}

bool Procesamiento_ReiniciarRegulador(Procesamiento_Regulador *r)
{
    if (r == NULL) {
        return false;
    }
    r->integral_num = 0;
    r->ultima_valida = 0U;
    r->periodos_invalidos = 0U;
    r->tiene_medicion = false;
    return true;
}

bool Procesamiento_InicializarRegulador(Procesamiento_Regulador *r,
                                      const Procesamiento_ConfigRegulador *cfg)
{
    if (r == NULL) {
        return false;
    }
    r->configurado = false;
    r->config = (Procesamiento_ConfigRegulador){0U, 0U, 0U, 0U, 0U};
    (void)Procesamiento_ReiniciarRegulador(r);
    if (cfg == NULL || cfg->kp_num < 1U || cfg->kp_num > 1000U ||
        cfg->kp_den < 1U || cfg->kp_den > 1000U ||
        cfg->ki_num < 1U || cfg->ki_num > 1000U ||
        cfg->ki_den < 1U || cfg->ki_den > 1000U ||
        cfg->max_periodos_sin_medicion < 1U || cfg->max_periodos_sin_medicion > 65535U) {
        return false;
    }
    r->config = *cfg;
    r->configurado = true;
    return true;
}

Procesamiento_EstadoRegulador Procesamiento_Regular(
    Procesamiento_Regulador *r, uint32_t consigna_rpm,
    const Medicion_Velocidad_Lectura *medicion, uint16_t *mando_por_mil)
{
    if (mando_por_mil != NULL) {
        *mando_por_mil = 0U;
    }
    if (r == NULL || medicion == NULL || mando_por_mil == NULL) {
        (void)Procesamiento_ReiniciarRegulador(r);
        return PROCESAMIENTO_REGULADOR_ERROR_ARGUMENTO;
    }
    if (!r->configurado) {
        (void)Procesamiento_ReiniciarRegulador(r);
        return PROCESAMIENTO_REGULADOR_ERROR_CONFIGURACION;
    }
    if (consigna_rpm > TPI_RPM_MAX) {
        (void)Procesamiento_ReiniciarRegulador(r);
        return PROCESAMIENTO_REGULADOR_ERROR_CONSIGNA;
    }
    if (consigna_rpm == 0U) {
        (void)Procesamiento_ReiniciarRegulador(r);
        return PROCESAMIENTO_REGULADOR_PARADO;
    }
    const bool valida = medicion->estado == MEDICION_VELOCIDAD_VALIDA;
    if (valida) {
        r->ultima_valida = medicion->rpm_milesimas;
        r->tiene_medicion = true;
        r->periodos_invalidos = 0U;
    } else if (r->tiene_medicion &&
               r->periodos_invalidos < r->config.max_periodos_sin_medicion) {
        ++r->periodos_invalidos;
    } else {
        (void)Procesamiento_ReiniciarRegulador(r);
        return PROCESAMIENTO_REGULADOR_SIN_MEDICION;
    }
    const int64_t error = (int64_t)consigna_rpm * (int64_t)MEDICION_VELOCIDAD_ESCALA_RPM -
                          (int64_t)r->ultima_valida;
    const int64_t d = (int64_t)r->config.kp_den * (int64_t)r->config.ki_den;
    const int64_t n = error * (int64_t)r->config.kp_num * (int64_t)r->config.ki_den +
                      r->integral_num * (int64_t)r->config.kp_den;
    const int64_t limite = (int64_t)TPI_MANDO_MANUAL_MAX * d;
    if (n >= limite) {
        *mando_por_mil = (uint16_t)TPI_MANDO_MANUAL_MAX;
    } else if (n > 0) {
        *mando_por_mil = (uint16_t)((n + d / 2) / d);
    }
    const bool saturando = (n >= limite && error > 0) || (n <= 0 && error < 0);
    if (valida && !saturando) {
        const int64_t siguiente = r->integral_num + error * (int64_t)r->config.ki_num;
        const int64_t max_integral = (int64_t)TPI_MANDO_MANUAL_MAX * (int64_t)r->config.ki_den;
        r->integral_num = siguiente < 0 ? 0 : (siguiente > max_integral ? max_integral : siguiente);
    }
    return valida ? PROCESAMIENTO_REGULADOR_VALIDO : PROCESAMIENTO_REGULADOR_RETENIENDO_MEDICION;
}

/** @} */
