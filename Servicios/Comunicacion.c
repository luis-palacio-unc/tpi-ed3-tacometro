/**
 * @file Comunicacion.c
 * @brief Parser/FIFO de consignas y codificacion acotada de telemetria ASCII.
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

/**
 * @brief Agregar un literal interno sin NUL al final de la trama en preparacion.
 * @details La longitud de los literales depende del formato cerrado; no busca un
 * terminador previo en destino ni admite textos externos.
 * @param[in,out] trama Buffer local con espacio suficiente segun cota interna de 54 bytes.
 * @param[in,out] posicion Indice siguiente, mantenido dentro de la cota de trama.
 * @param[in] texto Literal interno terminado en NUL; no procede de una entrada externa.
 * @pre Punteros validos, texto acotado por el formato y almacenamiento no solapado.
 * @note USW.2.1.7.8. Helper privado; no aplica a textos arbitrarios del llamador.
 */
static void AgregarTexto(char *trama, size_t *posicion, const char *texto)
{
    while (*texto != '\0') {
        trama[*posicion] = *texto;
        ++*posicion;
        ++texto;
    }
}

/**
 * @brief Agregar entero decimal sin signo ni ceros iniciales, con cero como un digito.
 * @details Obtiene restos decimales y luego invierte los digitos; cero produce un
 * digito. Almacenamiento temporal fijo para todo uint32_t.
 * @param[in,out] trama Buffer local con espacio para hasta 10 digitos adicionales.
 * @param[in,out] posicion Indice siguiente, dentro de la cota del formato.
 * @param[in] valor Entero uint32_t a convertir, sin multiplicaciones ni libc.
 * @pre Memoria interna valida/no solapada y capacidad garantizada por el formato.
 * @note USW.2.1.7.9. Dos recorridos de hasta 10 digitos; sin flotantes/heap.
 */
static void AgregarEntero(char *trama, size_t *posicion, uint32_t valor)
{
    char digitos[10];
    size_t cantidad = 0U;
    do {
        digitos[cantidad] = (char)('0' + valor % 10U);
        ++cantidad;
        valor /= 10U;
    } while (valor != 0U);
    while (cantidad > 0U) {
        --cantidad;
        trama[*posicion] = digitos[cantidad];
        ++*posicion;
    }
}

/**
 * @brief Agregar magnitud sin signo con fraccion fija de dos o tres decimales.
 * @details Separa parte entera/fraccion y recorre las posiciones de escala; conserva
 * ceros decimales sin conversiones flotantes o escalas del hardware.
 * @param[in,out] trama Buffer local con espacio segun la cota interna del formato.
 * @param[in,out] posicion Indice siguiente valido.
 * @param[in] valor Magnitud uint32_t ya expresada en unidades de escala.
 * @param[in] escala Constante interna 100 o 1000; nunca cero ni escala externa arbitraria.
 * @pre Punteros validos/no solapados y escala conforme a ese conjunto cerrado.
 * @note USW.2.1.7.10. Conserva ceros de fraccion; no redondea ni cambia unidades de origen.
 */
static void AgregarFijo(char *trama, size_t *posicion, uint32_t valor, uint32_t escala)
{
    AgregarEntero(trama, posicion, valor / escala);
    trama[*posicion] = '.';
    ++*posicion;
    const uint32_t resto = valor % escala;
    for (uint32_t factor = escala / 10U; factor > 0U; factor /= 10U) {
        trama[*posicion] = (char)('0' + (resto / factor) % 10U);
        ++*posicion;
    }
}

Comunicacion_ResultadoTelemetria Comunicacion_CodificarTelemetria(
    const Comunicacion_Telemetria *datos, char *buffer, size_t capacidad, size_t *longitud)
{
    if (longitud != NULL) {
        *longitud = 0U;
    }
    if (buffer != NULL && capacidad > 0U) {
        buffer[0] = '\0';
    }
    if (datos == NULL || buffer == NULL || longitud == NULL) {
        return COMUNICACION_TELEMETRIA_ERROR_ARGUMENTO;
    }
    if ((datos->validez_rpm != COMUNICACION_DATO_VALIDO &&
         datos->validez_rpm != COMUNICACION_DATO_SIN_DATOS &&
         datos->validez_rpm != COMUNICACION_DATO_ERROR) ||
        (datos->validez_temperatura != COMUNICACION_DATO_VALIDO &&
         datos->validez_temperatura != COMUNICACION_DATO_SIN_DATOS &&
         datos->validez_temperatura != COMUNICACION_DATO_ERROR)) {
        return COMUNICACION_TELEMETRIA_ERROR_ESTADO;
    }
    const char *estado;
    switch (datos->estado_operacion) {
        case COMUNICACION_OPERACION_MANUAL: estado = "MANUAL"; break;
        case COMUNICACION_OPERACION_AUTO_ESPERA: estado = "AUTO_ESPERA"; break;
        case COMUNICACION_OPERACION_AUTO_ACTIVO: estado = "AUTO_ACTIVO"; break;
        default: return COMUNICACION_TELEMETRIA_ERROR_ESTADO;
    }
    char trama[COMUNICACION_TELEMETRIA_CAPACIDAD];
    size_t posicion = 0U;
    AgregarTexto(trama, &posicion, "RPM=");
    if (datos->validez_rpm == COMUNICACION_DATO_VALIDO) {
        AgregarFijo(trama, &posicion, datos->rpm_milesimas, 1000U);
    } else {
        AgregarTexto(trama, &posicion, datos->validez_rpm == COMUNICACION_DATO_SIN_DATOS ? "ND" : "ERR");
    }
    AgregarTexto(trama, &posicion, ";TEMP=");
    if (datos->validez_temperatura == COMUNICACION_DATO_VALIDO) {
        const int32_t temperatura = datos->temperatura_centesimas_c;
        const uint32_t magnitud = temperatura < 0 ? 0U - (uint32_t)temperatura : (uint32_t)temperatura;
        if (temperatura < 0) {
            trama[posicion] = '-';
            ++posicion;
        }
        AgregarFijo(trama, &posicion, magnitud, 100U);
    } else {
        AgregarTexto(trama, &posicion, datos->validez_temperatura == COMUNICACION_DATO_SIN_DATOS ? "ND" : "ERR");
    }
    AgregarTexto(trama, &posicion, ";ESTADO=");
    AgregarTexto(trama, &posicion, estado);
    trama[posicion] = '\n';
    ++posicion;
    trama[posicion] = '\0';
    if (capacidad <= posicion) {
        return COMUNICACION_TELEMETRIA_ERROR_CAPACIDAD;
    }
    for (size_t i = 0U; i <= posicion; ++i) {
        buffer[i] = trama[i];
    }
    *longitud = posicion;
    return COMUNICACION_TELEMETRIA_OK;
}

/** @} */
