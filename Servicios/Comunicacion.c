/**
 * @file Comunicacion.c
 * @brief Parser ASCII y FIFO acotada de consignas recibidas.
 * @details Sin estado global mutable, libc, heap, UART o dependencias de aplicacion.
 * @note ESW.2.1.7. Contrato publico y precondiciones en Comunicacion.h.
 */
#include "Comunicacion.h"
#include <stddef.h>

/** @addtogroup comunicacion
 * @{
 */

/**
 * @brief Volver al comienzo de una trama sin modificar la FIFO ni el orden.
 * @details Los bytes anteriores quedan fuera de longitud y no vuelven a interpretarse.
 * @param[in,out] contexto Instancia valida no nula.
 * @note USW.2.1.7.4. No es necesario borrar memoria fuera de longitud.
 */
static void ReiniciarTrama(Comunicacion_Contexto *contexto)
{
    contexto->longitud = 0U;
    contexto->descartando = false;
}

/**
 * @brief Validar una trama sin LF y convertir sus 1..3 digitos decimales.
 * @details CR es opcional solo al final. El acumulador nunca supera 999; no usa
 * conversiones de C strings ni depende del locale. Solo escribe rpm si es valida.
 * @param[in] trama Buffer con longitud bytes validos, sin terminador NUL.
 * @param[in] longitud Cantidad de bytes antes del LF, de 0 a 8.
 * @param[out] rpm Memoria no nula para el entero validado.
 * @return OK, ERROR_FORMATO o ERROR_RANGO.
 * @note USW.2.1.7.5. Punteros internos validos, no expuesta al integrador.
 */
static Comunicacion_Resultado Decodificar(const uint8_t *trama, uint8_t longitud,
                                        uint32_t *rpm)
{
    if (longitud > 0U && trama[longitud - 1U] == (uint8_t)'\r') {
        --longitud;
    }
    if (longitud < 5U || longitud > 7U ||
        trama[0] != (uint8_t)'R' || trama[1] != (uint8_t)'P' ||
        trama[2] != (uint8_t)'M' || trama[3] != (uint8_t)' ') {
        return COMUNICACION_ERROR_FORMATO;
    }
    uint32_t valor = 0U;
    for (uint8_t i = 4U; i < longitud; ++i) {
        if (trama[i] < (uint8_t)'0' || trama[i] > (uint8_t)'9') {
            return COMUNICACION_ERROR_FORMATO;
        }
        valor = valor * 10U + (uint32_t)(trama[i] - (uint8_t)'0');
    }
    if (valor > COMUNICACION_RPM_MAX) {
        return COMUNICACION_ERROR_RANGO;
    }
    *rpm = valor;
    return COMUNICACION_OK;
}

/**
 * @brief Agregar un evento validado al final de la cola circular.
 * @details Cola llena rechaza el nuevo evento sin modificar los pendientes.
 * @param[in,out] contexto Instancia valida con metadatos inicializados.
 * @param[in] rpm Consigna ya validada en 0..185 RPM.
 * @param[in] orden Secuencia del LF final de recepcion.
 * @return ENCOLADA o ERROR_COLA_LLENA.
 * @note USW.2.1.7.6. No asigna memoria ni sobrescribe la entrada mas antigua.
 */
static Comunicacion_Resultado Encolar(Comunicacion_Contexto *contexto,
                                    uint32_t rpm, uint64_t orden)
{
    if (contexto->cantidad >= COMUNICACION_COLA_CAPACIDAD) {
        return COMUNICACION_ERROR_COLA_LLENA;
    }
    uint8_t indice = (uint8_t)((contexto->cabeza + contexto->cantidad) %
                              COMUNICACION_COLA_CAPACIDAD);
    contexto->cola[indice].rpm = rpm;
    contexto->cola[indice].orden_recepcion = orden;
    ++contexto->cantidad;
    return COMUNICACION_ENCOLADA;
}

bool Comunicacion_Inicializar(Comunicacion_Contexto *contexto, uint64_t orden_inicial)
{
    if (contexto == NULL) {
        return false;
    }
    contexto->ultimo_orden = orden_inicial;
    contexto->cabeza = 0U;
    contexto->cantidad = 0U;
    ReiniciarTrama(contexto);
    return true;
}

Comunicacion_Resultado Comunicacion_RecibirByte(Comunicacion_Contexto *contexto,
                                             uint8_t byte, uint64_t orden)
{
    if (contexto == NULL) {
        return COMUNICACION_ERROR_ARGUMENTO;
    }
    if (orden <= contexto->ultimo_orden) {
        contexto->longitud = 0U;
        contexto->descartando = byte != (uint8_t)'\n';
        return COMUNICACION_ERROR_ORDEN;
    }
    contexto->ultimo_orden = orden;
    if (contexto->descartando) {
        if (byte == (uint8_t)'\n') {
            ReiniciarTrama(contexto);
        }
        return COMUNICACION_OK;
    }
    if (byte == (uint8_t)'\n') {
        uint32_t rpm = 0U;
        Comunicacion_Resultado resultado = Decodificar(contexto->trama,
                                                      contexto->longitud, &rpm);
        ReiniciarTrama(contexto);
        if (resultado != COMUNICACION_OK) {
            return resultado;
        }
        return Encolar(contexto, rpm, orden);
    }
    if (contexto->longitud >= COMUNICACION_TRAMA_MAX) {
        contexto->longitud = 0U;
        contexto->descartando = true;
        return COMUNICACION_ERROR_LONGITUD;
    }
    contexto->trama[contexto->longitud] = byte;
    ++contexto->longitud;
    return COMUNICACION_OK;
}

Comunicacion_Resultado Comunicacion_ExtraerConsigna(Comunicacion_Contexto *contexto,
                                                 Comunicacion_Consigna *evento)
{
    if (evento != NULL) {
        evento->rpm = 0U;
        evento->orden_recepcion = 0U;
    }
    if (contexto == NULL || evento == NULL) {
        return COMUNICACION_ERROR_ARGUMENTO;
    }
    if (contexto->cantidad == 0U) {
        return COMUNICACION_VACIA;
    }
    evento->rpm = contexto->cola[contexto->cabeza].rpm;
    evento->orden_recepcion = contexto->cola[contexto->cabeza].orden_recepcion;
    contexto->cabeza = (uint8_t)((contexto->cabeza + 1U) % COMUNICACION_COLA_CAPACIDAD);
    --contexto->cantidad;
    return COMUNICACION_EXTRAIDA;
}

/** @} */
