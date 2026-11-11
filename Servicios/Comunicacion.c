/**
 * @file Comunicacion.c
 * @brief Lectura de consignas "RPM n" y armado de la trama de telemetria.
 * @details La linea en recepcion vive en memoria estatica. Sin libc ni heap.
 * @note ESW.2.1.7. Contratos publicos en Comunicacion.h.
 */
#include "Comunicacion.h"
#include "Limites.h"
#include "UART.h"
#include <stddef.h>

/** @addtogroup comunicacion
 * @{
 */

/** @brief Bytes de la linea en recepcion, sin el LF. */
static uint8_t Comunicacion_Linea[COMUNICACION_LINEA_MAX];
/**
 * @brief Bytes recibidos de la linea actual.
 * @details Cuenta hasta COMUNICACION_LINEA_MAX + 1: ese valor marca una linea
 * demasiado larga, que se descarta al llegar su LF.
 */
static uint8_t Comunicacion_Longitud;

/**
 * @brief Validar una linea sin LF y convertir sus digitos.
 * @details Acepta "RPM", un espacio y de 1 a 3 digitos, con CR final opcional, y
 * un valor de 0 a TPI_RPM_MAX. Admite ceros a la izquierda.
 * @param[in] linea Bytes recibidos antes del LF.
 * @param[in] longitud Cantidad de bytes, de 0 a COMUNICACION_LINEA_MAX.
 * @param[out] rpm Consigna convertida; solo se escribe al retornar true.
 * @return true si la linea es una consigna valida.
 * @note USW.2.1.7.15. Unidad interna, sin efectos laterales.
 */
static bool Comunicacion_Decodificar(const uint8_t *linea, uint8_t longitud, uint32_t *rpm)
{
    if (longitud > 0U && linea[longitud - 1U] == (uint8_t)'\r') {
        --longitud;
    }
    if (longitud < 5U || longitud > 7U ||
        linea[0] != (uint8_t)'R' || linea[1] != (uint8_t)'P' ||
        linea[2] != (uint8_t)'M' || linea[3] != (uint8_t)' ') {
        return false;
    }
    uint32_t valor = 0U;
    for (uint8_t i = 4U; i < longitud; ++i) {
        if (linea[i] < (uint8_t)'0' || linea[i] > (uint8_t)'9') {
            return false;
        }
        valor = valor * 10U + (uint32_t)(linea[i] - (uint8_t)'0');
    }
    if (valor > TPI_RPM_MAX) {
        return false;
    }
    *rpm = valor;
    return true;
}

/**
 * @brief Agregar un literal del formato al final de la trama en preparacion.
 * @param[in,out] trama Buffer de COMUNICACION_TELEMETRIA_MAX bytes.
 * @param[in,out] posicion Indice del proximo byte libre.
 * @param[in] texto Literal interno terminado en NUL.
 * @note USW.2.1.7.8. Unidad interna; el formato garantiza la capacidad.
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
 * @brief Agregar un entero decimal sin ceros a la izquierda; cero ocupa un digito.
 * @param[in,out] trama Buffer de COMUNICACION_TELEMETRIA_MAX bytes.
 * @param[in,out] posicion Indice del proximo byte libre.
 * @param[in] valor Entero a convertir, hasta 10 digitos.
 * @note USW.2.1.7.9. Unidad interna; sin libc ni flotantes.
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
 * @brief Agregar una magnitud con punto decimal fijo de dos o tres cifras.
 * @param[in,out] trama Buffer de COMUNICACION_TELEMETRIA_MAX bytes.
 * @param[in,out] posicion Indice del proximo byte libre.
 * @param[in] valor Magnitud expresada en unidades de la escala.
 * @param[in] escala 100 para centesimas o 1000 para milesimas.
 * @note USW.2.1.7.10. Unidad interna; conserva los ceros de la fraccion.
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

void Comunicacion_Init(void)
{
    UART0_Init();
    Comunicacion_Longitud = 0U;
}

bool Comunicacion_RecibirConsigna(uint32_t *rpm)
{
    uint8_t byte;
    while (UART_Read(&byte, 1U) == 1U) {
        if (byte == (uint8_t)'\n') {
            const uint8_t longitud = Comunicacion_Longitud;
            Comunicacion_Longitud = 0U;
            if (longitud <= COMUNICACION_LINEA_MAX &&
                Comunicacion_Decodificar(Comunicacion_Linea, longitud, rpm)) {
                return true;
            }
        } else if (Comunicacion_Longitud < COMUNICACION_LINEA_MAX) {
            Comunicacion_Linea[Comunicacion_Longitud] = byte;
            ++Comunicacion_Longitud;
        } else {
            Comunicacion_Longitud = COMUNICACION_LINEA_MAX + 1U;
        }
    }
    return false;
}

void Comunicacion_Descartar(void)
{
    uint8_t byte;
    while (UART_Read(&byte, 1U) == 1U) {
    }
    Comunicacion_Longitud = 0U;
}

bool Comunicacion_EnviarTelemetria(uint32_t rpm_milesimas, int32_t temperatura_centesimas,
                                   Comunicacion_Estado estado)
{
    char trama[COMUNICACION_TELEMETRIA_MAX];
    size_t posicion = 0U;
    AgregarTexto(trama, &posicion, "RPM=");
    AgregarFijo(trama, &posicion, rpm_milesimas, 1000U);
    AgregarTexto(trama, &posicion, ";TEMP=");
    if (temperatura_centesimas < 0) {
        AgregarTexto(trama, &posicion, "-");
        AgregarFijo(trama, &posicion, 0U - (uint32_t)temperatura_centesimas, 100U);
    } else {
        AgregarFijo(trama, &posicion, (uint32_t)temperatura_centesimas, 100U);
    }
    AgregarTexto(trama, &posicion, ";ESTADO=");
    AgregarTexto(trama, &posicion, estado == COMUNICACION_ESTADO_MANUAL ? "MANUAL" :
                 (estado == COMUNICACION_ESTADO_AUTO_ESPERA ? "AUTO_ESPERA" : "AUTO_ACTIVO"));
    AgregarTexto(trama, &posicion, "\n");
    return UART_Write((const uint8_t *)trama, (uint32_t)posicion) == (uint32_t)posicion;
}

/** @} */
