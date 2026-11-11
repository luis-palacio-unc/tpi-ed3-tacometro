/**
 * @file lpc17xx.h
 * @brief Puente de nombre para compilar la biblioteca original en sistemas de
 * archivos que distinguen mayusculas.
 * @details lpc17xx_clkpwr.h y lpc17xx_pinsel.h de la biblioteca incluyen
 * "lpc17xx.h", pero el encabezado CMSIS se llama LPC17xx.h y Drivers no se modifica.
 * Este directorio se agrega como ruta de inclusion de sistema junto a la biblioteca.
 * @note Soporte de plataforma B18/I1, sin ESW del anteproyecto ni codigo ejecutable.
 */
#ifndef TPI_COMPAT_LPC17XX_H
#define TPI_COMPAT_LPC17XX_H

#include "LPC17xx.h"

#endif /* TPI_COMPAT_LPC17XX_H */
