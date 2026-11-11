/**
 * @file Procesamiento.c
 * @brief Maquina de modos, alarma con histeresis y regulador PI entero.
 * @details Sin perifericos ni memoria dinamica. El modo, la consigna, la alarma y
 * la integral del regulador viven en memoria estatica.
 * @note ESW.2.1.2. Contratos publicos en Procesamiento.h.
 */
#include "Procesamiento.h"
#include "Alarma.h"
#include "Control.h"

/** @addtogroup procesamiento
 * @{
 */

/** @brief Estado de operacion vigente. */
static Procesamiento_Estado Procesamiento_EstadoActual;
/** @brief Consigna vigente en RPM; cero fuera de AUTO_ACTIVO. */
static uint32_t Procesamiento_ConsignaRpm;
/** @brief Ultima decision de la alarma; apagada al arrancar. */
static bool Procesamiento_AlarmaActiva;
/** @brief Integral del PI multiplicada por TPI_CONTROL_KI_DEN; de 0 a 1000 * KI_DEN. */
static int64_t Procesamiento_IntegralNum;

Procesamiento_Estado Procesamiento_InicializarModo(Mandos_Modo modo)
{
    Procesamiento_EstadoActual = modo == MANDOS_MODO_MANUAL ?
                                 PROCESAMIENTO_MANUAL : PROCESAMIENTO_AUTO_ESPERA;
    Procesamiento_ConsignaRpm = 0U;
    Procesamiento_IntegralNum = 0;
    return Procesamiento_EstadoActual;
}

Procesamiento_Estado Procesamiento_ActualizarModo(Mandos_Modo modo, bool hay_consigna,
                                                  uint32_t consigna_rpm)
{
    if (modo == MANDOS_MODO_MANUAL) {
        Procesamiento_EstadoActual = PROCESAMIENTO_MANUAL;
        Procesamiento_ConsignaRpm = 0U;
        Procesamiento_IntegralNum = 0;
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

uint16_t Procesamiento_Regular(uint32_t consigna_rpm, uint32_t rpm_milesimas)
{
    uint16_t mando = 0U;
    if (consigna_rpm == 0U) {
        Procesamiento_IntegralNum = 0;
        return mando;
    }
    const int64_t error = (int64_t)consigna_rpm * (int64_t)MEDICION_VELOCIDAD_ESCALA_RPM -
                          (int64_t)rpm_milesimas;
    const int64_t d = (int64_t)TPI_CONTROL_KP_DEN * (int64_t)TPI_CONTROL_KI_DEN;
    const int64_t n = error * (int64_t)TPI_CONTROL_KP_NUM * (int64_t)TPI_CONTROL_KI_DEN +
                      Procesamiento_IntegralNum * (int64_t)TPI_CONTROL_KP_DEN;
    const int64_t limite = (int64_t)TPI_MANDO_MANUAL_MAX * d;
    if (n >= limite) {
        mando = (uint16_t)TPI_MANDO_MANUAL_MAX;
    } else if (n > 0) {
        mando = (uint16_t)((n + d / 2) / d);
    }
    const bool saturando = (n >= limite && error > 0) || (n <= 0 && error < 0);
    if (!saturando) {
        const int64_t siguiente = Procesamiento_IntegralNum + error * (int64_t)TPI_CONTROL_KI_NUM;
        const int64_t max_integral = (int64_t)TPI_MANDO_MANUAL_MAX * (int64_t)TPI_CONTROL_KI_DEN;
        Procesamiento_IntegralNum = siguiente < 0 ? 0 :
                                    (siguiente > max_integral ? max_integral : siguiente);
    }
    return mando;
}

/** @} */
