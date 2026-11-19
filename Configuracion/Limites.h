/**
 * @file Limites.h
 * @brief Limites de dominio compartidos por aplicacion y servicios.
 * @details Una fuente comun mantiene rangos iguales en parser, mandos y aplicacion.
 * @note R14: maximo acordado por el usuario (185 RPM hasta el 2026-11-16, 300 hasta
 * el 2026-11-19 y 700 desde entonces); no limita la medicion real del eje ni acredita que el motor lo alcance.
 */
#ifndef TPI_LIMITES_H
#define TPI_LIMITES_H

/** @brief Maxima consigna aceptada, en RPM inclusive. */
#define TPI_RPM_MAX 700U

/** @brief Maximo mando manual normalizado, en unidades por mil (100 por ciento). */
#define TPI_MANDO_MANUAL_MAX 1000U

#endif /* TPI_LIMITES_H */
