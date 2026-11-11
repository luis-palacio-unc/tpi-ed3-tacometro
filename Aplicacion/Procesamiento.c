/**
 * @file Procesamiento.c
 * @brief Maquina de modos, alarma con histeresis y regulador PI entero.
 * @details Sin perifericos ni memoria dinamica. El modo, la consigna y la alarma
 * viven en memoria estatica; el regulador usa la instancia que recibe.
 * @note ESW.2.1.2. Contratos publicos en Procesamiento.h.
 */
#include "Procesamiento.h"
#include "Alarma.h"
#include "Control.h"
#include <stddef.h>

/** @addtogroup procesamiento
 * @{
 */

/** @brief Estado de operacion vigente. */
static Procesamiento_Estado Procesamiento_EstadoActual;
/** @brief Consigna vigente en RPM; cero fuera de AUTO_ACTIVO. */
static uint32_t Procesamiento_ConsignaRpm;
/** @brief Ultima decision de la alarma; apagada al arrancar. */
static bool Procesamiento_AlarmaActiva;

Procesamiento_Estado Procesamiento_InicializarModo(Mandos_Modo modo)
{
    Procesamiento_EstadoActual = modo == MANDOS_MODO_MANUAL ?
                                 PROCESAMIENTO_MANUAL : PROCESAMIENTO_AUTO_ESPERA;
    Procesamiento_ConsignaRpm = 0U;
    return Procesamiento_EstadoActual;
}

Procesamiento_Estado Procesamiento_ActualizarModo(Mandos_Modo modo, bool hay_consigna,
                                                  uint32_t consigna_rpm)
{
    if (modo == MANDOS_MODO_MANUAL) {
        Procesamiento_EstadoActual = PROCESAMIENTO_MANUAL;
        Procesamiento_ConsignaRpm = 0U;
    } else if (Procesamiento_EstadoActual == PROCESAMIENTO_MANUAL) {
        Procesamiento_EstadoActual = PROCESAMIENTO_AUTO_ESPERA;
    } else if (hay_consigna && consigna_rpm <= TPI_RPM_MAX) {
        Procesamiento_EstadoActual = PROCESAMIENTO_AUTO_ACTIVO;
        Procesamiento_ConsignaRpm = consigna_rpm;
    }
    return Procesamiento_EstadoActual;
}

uint32_t Procesamiento_Consigna(void)
{
    return Procesamiento_ConsignaRpm;
}

bool Procesamiento_EvaluarAlarma(int32_t temperatura_centesimas)
{
    if (temperatura_centesimas > TPI_ALARMA_UMBRAL_CENTESIMAS) {
        Procesamiento_AlarmaActiva = true;
    } else if (temperatura_centesimas <=
               TPI_ALARMA_UMBRAL_CENTESIMAS - TPI_ALARMA_HISTERESIS_CENTESIMAS) {
        Procesamiento_AlarmaActiva = false;
    }
    return Procesamiento_AlarmaActiva;
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
