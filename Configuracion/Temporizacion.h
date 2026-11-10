/**
 * @file Temporizacion.h
 * @brief Periodo inicial del contador temporal del TPI, en milisegundos enteros.
 * @details Valor elegido 1 ms, configurable al compilar o mediante parametro de
 * inicializacion del dispositivo. No fija PCLK ni introduce hardware en servicios.
 * @note R10/R15, B11. La composicion de arranque suministrara el valor al dispositivo.
 */
#ifndef TPI_CONFIGURACION_TEMPORIZACION_H
#define TPI_CONFIGURACION_TEMPORIZACION_H
#include <stdint.h>
/** @brief Periodo predeterminado TPI; override de build permitido, 1..INT32_MAX ms. */
#ifndef TPI_BASE_TEMPORAL_MS
#define TPI_BASE_TEMPORAL_MS UINT32_C(1)
#endif
_Static_assert(TPI_BASE_TEMPORAL_MS>0 && TPI_BASE_TEMPORAL_MS<=INT32_MAX,
               "TPI_BASE_TEMPORAL_MS debe ser entero positivo <= INT32_MAX.");
#endif
