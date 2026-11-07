/**
 * @file Procesamiento.c
 * @brief Implementacion de la seleccion de mando y transiciones logicas.
 * @details Sin perifericos, memoria dinamica, reloj o estado global mutable.
 * @note ESW.2.1.2. Contratos publicos en Procesamiento.h.
 */
#include "Procesamiento.h"
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

/** @} */
