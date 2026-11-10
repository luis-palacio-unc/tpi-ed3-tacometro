/**
 * @file Mandos.c
 * @brief Normalizacion manual y selector estable con tiempo externo.
 * @details Producto y redondeo en uint64_t: hasta 4 297 114 778 647, sin overflow.
 * Lectura<=escala y escala>0 garantizan salida<=1000, representable en uint16_t.
 * B10 agrega estabilidad de selector por resta modular uint32_t sin esperas.
 * @note ESW.2.1.5. Contratos/USW en Mandos.h; libgcc ya provisto por el toolchain.
 */
#include "Mandos.h"
#include <stddef.h>

/** @addtogroup mandos
 * @{
 */

bool Mandos_Inicializar(Mandos_MandoManual *mando)
{
    if (mando == NULL) {
        return false;
    }
    mando->por_mil = 0U;
    mando->estado = MANDOS_SIN_DATOS;
    return true;
}

Mandos_Estado Mandos_NormalizarManual(Mandos_MandoManual *mando,
                                    uint32_t lectura, uint32_t fondo_escala)
{
    if (mando == NULL) {
        return MANDOS_ERROR_ARGUMENTO;
    }
    mando->por_mil = 0U;
    if (fondo_escala == 0U) {
        mando->estado = MANDOS_ERROR_ESCALA;
        return mando->estado;
    }
    if (lectura > fondo_escala) {
        mando->estado = MANDOS_ERROR_LECTURA;
        return mando->estado;
    }
    const uint64_t numerador = (uint64_t)lectura * MANDOS_MANDO_MAX;
    const uint64_t redondeado = (numerador + (uint64_t)fondo_escala / 2U) / fondo_escala;
    mando->por_mil = (uint16_t)redondeado;
    mando->estado = MANDOS_VALIDO;
    return mando->estado;
}

/**
 * @brief Invalidar modo confirmado/candidato y publicar un diagnostico completo.
 * @details No cambia intervalo ni anclaje temporal. El llamador decide cuando debe
 * descartar el anclaje; ningun modo anterior queda disponible tras invalidar.
 * @param[in,out] selector Contexto interno escribible, valido y no nulo.
 * @param[in] estado SIN_DATOS o error conocido, nunca VALIDO.
 * @return Estado asignado al contexto.
 * @pre Acceso exclusivo y estado conforme al conjunto indicado.
 * @note USW.2.1.5.5. Sin efectos en mando manual, aplicacion o hardware.
 */
static Mandos_EstadoSelector InvalidarSelector(Mandos_Selector *selector, Mandos_EstadoSelector estado)
{
    selector->modo = MANDOS_SELECTOR_MODO_NINGUNO;
    selector->candidato = MANDOS_SELECTOR_MODO_NINGUNO;
    selector->inicio_candidato_ms = 0U;
    selector->estado = estado;
    return estado;
}

bool Mandos_InicializarSelector(Mandos_Selector *selector, uint32_t intervalo_estable_ms)
{
    if (selector == NULL) {
        return false;
    }
    selector->intervalo_estable_ms = intervalo_estable_ms;
    selector->ultimo_tiempo_ms = 0U;
    selector->tiene_tiempo = false;
    (void)InvalidarSelector(selector, intervalo_estable_ms <= MANDOS_SELECTOR_INTERVALO_MAX_MS ?
        MANDOS_SELECTOR_SIN_DATOS : MANDOS_SELECTOR_ERROR_CONFIGURACION);
    return selector->estado == MANDOS_SELECTOR_SIN_DATOS;
}

Mandos_EstadoSelector Mandos_ActualizarSelector(Mandos_Selector *selector,
    Mandos_ModoSelector modo, Mandos_FuenteSelector fuente, uint32_t tiempo_ms)
{
    if (selector == NULL) {
        return MANDOS_SELECTOR_ERROR_ARGUMENTO;
    }
    if (selector->intervalo_estable_ms > MANDOS_SELECTOR_INTERVALO_MAX_MS) {
        return InvalidarSelector(selector, MANDOS_SELECTOR_ERROR_CONFIGURACION);
    }
    if (fuente != MANDOS_SELECTOR_FUENTE_SIN_DATOS &&
        fuente != MANDOS_SELECTOR_FUENTE_VALIDA && fuente != MANDOS_SELECTOR_FUENTE_ERROR) {
        return InvalidarSelector(selector, MANDOS_SELECTOR_ERROR_ARGUMENTO);
    }
    if (selector->tiene_tiempo &&
        (uint32_t)(tiempo_ms - selector->ultimo_tiempo_ms) > MANDOS_SELECTOR_INTERVALO_MAX_MS) {
        selector->tiene_tiempo = false;
        selector->ultimo_tiempo_ms = 0U;
        return InvalidarSelector(selector, MANDOS_SELECTOR_ERROR_TIEMPO);
    }
    selector->ultimo_tiempo_ms = tiempo_ms;
    selector->tiene_tiempo = true;
    if (fuente == MANDOS_SELECTOR_FUENTE_SIN_DATOS) {
        return InvalidarSelector(selector, MANDOS_SELECTOR_SIN_DATOS);
    }
    if (fuente == MANDOS_SELECTOR_FUENTE_ERROR) {
        return InvalidarSelector(selector, MANDOS_SELECTOR_ERROR_FUENTE);
    }
    if (modo != MANDOS_SELECTOR_MODO_MANUAL && modo != MANDOS_SELECTOR_MODO_AUTOMATICO) {
        return InvalidarSelector(selector, MANDOS_SELECTOR_ERROR_MODO);
    }
    if (selector->candidato != modo) {
        selector->candidato = modo;
        selector->inicio_candidato_ms = tiempo_ms;
    }
    if ((uint32_t)(tiempo_ms - selector->inicio_candidato_ms) >= selector->intervalo_estable_ms) {
        selector->modo = modo;
    }
    selector->estado = selector->modo == MANDOS_SELECTOR_MODO_NINGUNO ?
        MANDOS_SELECTOR_SIN_DATOS : MANDOS_SELECTOR_VALIDO;
    return selector->estado;
}

/** @} */
