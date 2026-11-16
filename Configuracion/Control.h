/**
 * @file Control.h
 * @brief Ganancias discretas del regulador PI del TPI.
 * @details Defaults provisionales para el modelo, no sintonia del motor real.
 * Las ganancias son por mil por milesima RPM; Ki ya incluye el paso de 1 s.
 * @note Las usa Procesamiento_Regular. No configura Timer ni establece la cadencia real.
 */
#ifndef TPI_CONTROL_H
#define TPI_CONTROL_H

#include <stdint.h>

/** @brief Numerador proporcional configurable, 1..1000. */
#ifndef TPI_CONTROL_KP_NUM
#define TPI_CONTROL_KP_NUM 4U
#endif
#if TPI_CONTROL_KP_NUM < 1 || TPI_CONTROL_KP_NUM > 1000
#error "TPI_CONTROL_KP_NUM debe pertenecer a 1..1000."
#endif

/** @brief Denominador proporcional configurable, 1..1000. */
#ifndef TPI_CONTROL_KP_DEN
#define TPI_CONTROL_KP_DEN 1000U
#endif
#if TPI_CONTROL_KP_DEN < 1 || TPI_CONTROL_KP_DEN > 1000
#error "TPI_CONTROL_KP_DEN debe pertenecer a 1..1000."
#endif

/** @brief Numerador integral por paso configurable, 1..1000. */
#ifndef TPI_CONTROL_KI_NUM
#define TPI_CONTROL_KI_NUM 1U
#endif
#if TPI_CONTROL_KI_NUM < 1 || TPI_CONTROL_KI_NUM > 1000
#error "TPI_CONTROL_KI_NUM debe pertenecer a 1..1000."
#endif

/** @brief Denominador integral configurable, 1..1000. */
#ifndef TPI_CONTROL_KI_DEN
#define TPI_CONTROL_KI_DEN 1000U
#endif
#if TPI_CONTROL_KI_DEN < 1 || TPI_CONTROL_KI_DEN > 1000
#error "TPI_CONTROL_KI_DEN debe pertenecer a 1..1000."
#endif

/**
 * @brief Constante de contrato: un paso y una ventana representan 1 s (1000 ms).
 * @details El guard admite solo 1000, incluso con definicion del compilador.
 * Hasta el 2026-11-16 fue de 500 ms; se paso a 1 s para contar los pulsos de las
 * RPM en una ventana mas larga (1 RPM por pulso con 60 ranuras). Las ganancias por
 * paso no se cambiaron: la respuesta en pasos es la misma y en tiempo es el doble.
 * No es un parametro ajustable. Cambiar periodo exige recalcular ganancias
 * discretizadas y revisar el contrato. No programa ni lee el Timer.
 */
#ifndef TPI_CONTROL_PERIODO_MS
#define TPI_CONTROL_PERIODO_MS 1000U
#endif
#if TPI_CONTROL_PERIODO_MS != 1000
#error "TPI_CONTROL_PERIODO_MS es constante contractual de 1000 ms."
#endif

#endif /* TPI_CONTROL_H */
