/**
 * @file Mandos.c
 * @brief Conversion entera del potenciometro a mando manual por mil.
 * @details Producto y redondeo en uint64_t: hasta 4 297 114 778 647, sin overflow.
 * Lectura<=escala y escala>0 garantizan salida<=1000, representable en uint16_t.
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

/** @} */
