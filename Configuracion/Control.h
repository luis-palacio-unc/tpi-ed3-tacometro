/**
 * @file Control.h
 * @brief Ganancias discretas del regulador PI del TPI.
 * @details Las ganancias son por mil de mando por milesima de RPM; Ki ya incluye el
 * paso de 500 ms. Los valores por defecto salen del ensayo de escalon del 2026-11-19
 * con el motor y el disco de 60 ranuras: constante de tiempo de unos 0,14 s y pendiente
 * maxima de la curva estatica de unos 1,3 RPM por cada por mil de mando.
 *
 * Procesamiento_Regular calcula el mando con la integral del paso anterior, de modo
 * que equivale a un PI de libro (integral actualizada antes de calcular) con
 * proporcional Kp - Ki. Con Kp = 1/2000 y Ki = 1/2500 queda un proporcional de
 * 0,1 por mil por RPM, una integral de 0,4 por mil por RPM en cada paso y
 * Ti = 0,1 * 0,5 s / 0,4 = 0,125 s, cercano a la constante de tiempo del motor.
 * @note Las usa Procesamiento_Regular. No configura Timer ni establece la cadencia real.
 */
#ifndef TPI_CONTROL_H
#define TPI_CONTROL_H

#include <stdint.h>

/** @brief Numerador proporcional configurable, 1..1000. */
#ifndef TPI_CONTROL_KP_NUM
#define TPI_CONTROL_KP_NUM 1U
#endif
#if TPI_CONTROL_KP_NUM < 1 || TPI_CONTROL_KP_NUM > 1000
#error "TPI_CONTROL_KP_NUM debe pertenecer a 1..1000."
#endif

/** @brief Denominador proporcional configurable, 1..10000. */
#ifndef TPI_CONTROL_KP_DEN
#define TPI_CONTROL_KP_DEN 2000U
#endif
#if TPI_CONTROL_KP_DEN < 1 || TPI_CONTROL_KP_DEN > 10000
#error "TPI_CONTROL_KP_DEN debe pertenecer a 1..10000."
#endif

/** @brief Numerador integral por paso configurable, 1..1000. */
#ifndef TPI_CONTROL_KI_NUM
#define TPI_CONTROL_KI_NUM 1U
#endif
#if TPI_CONTROL_KI_NUM < 1 || TPI_CONTROL_KI_NUM > 1000
#error "TPI_CONTROL_KI_NUM debe pertenecer a 1..1000."
#endif

/** @brief Denominador integral configurable, 1..10000. */
#ifndef TPI_CONTROL_KI_DEN
#define TPI_CONTROL_KI_DEN 2500U
#endif
#if TPI_CONTROL_KI_DEN < 1 || TPI_CONTROL_KI_DEN > 10000
#error "TPI_CONTROL_KI_DEN debe pertenecer a 1..10000."
#endif

/*
 * Procesamiento_Regular multiplica el error (hasta 2^32 * 1000 milesimas de RPM) por
 * KP_NUM * KI_DEN en 64 bits: el producto no puede pasar de un millon.
 */
#if TPI_CONTROL_KP_NUM * TPI_CONTROL_KI_DEN > 1000000
#error "TPI_CONTROL_KP_NUM por TPI_CONTROL_KI_DEN no debe superar 1000000."
#endif

/**
 * @brief Constante de contrato: un paso y una ventana representan 500 ms.
 * @details El guard admite solo 500, incluso con definicion del compilador.
 * No es un parametro ajustable. Cambiar periodo exige recalcular ganancias
 * discretizadas y revisar el contrato. No programa ni lee el Timer.
 */
#ifndef TPI_CONTROL_PERIODO_MS
#define TPI_CONTROL_PERIODO_MS 500U
#endif
#if TPI_CONTROL_PERIODO_MS != 500
#error "TPI_CONTROL_PERIODO_MS es constante contractual de 500 ms."
#endif

#endif /* TPI_CONTROL_H */
