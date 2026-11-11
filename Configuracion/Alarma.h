/**
 * @file Alarma.h
 * @brief Umbral e histeresis de la alarma de temperatura ambiente del TPI.
 * @details Decision del usuario (2026-11-11): la alarma se enciende por encima de
 * 45,00 grados Celsius y se apaga al bajar a 43,00 grados o menos.
 * @note B18/I2. Valores en centesimas de grado Celsius, la unidad del servicio
 * Temperatura. No acredita la exactitud del sensor.
 */
#ifndef TPI_ALARMA_H
#define TPI_ALARMA_H

/** @brief Temperatura que enciende la alarma al superarla, en centesimas de grado. */
#ifndef TPI_ALARMA_UMBRAL_CENTESIMAS
#define TPI_ALARMA_UMBRAL_CENTESIMAS 4500
#endif

/** @brief Descenso bajo el umbral que apaga la alarma, en centesimas de grado. */
#ifndef TPI_ALARMA_HISTERESIS_CENTESIMAS
#define TPI_ALARMA_HISTERESIS_CENTESIMAS 200
#endif

#if TPI_ALARMA_HISTERESIS_CENTESIMAS < 0 || \
    TPI_ALARMA_HISTERESIS_CENTESIMAS > TPI_ALARMA_UMBRAL_CENTESIMAS
#error "TPI_ALARMA_HISTERESIS_CENTESIMAS debe estar entre 0 y el umbral."
#endif

#endif /* TPI_ALARMA_H */
