/**
 * @file Temperatura.h
 * @brief Temperatura ambiente filtrada, en centesimas de grado Celsius.
 * @details Promedia las ultimas lecturas del LM35 para que el ruido del ADC no
 * haga oscilar la alarma.
 * @note ESW.2.1.4. Usa el dispositivo LM35.
 */
#ifndef TPI_TEMPERATURA_H
#define TPI_TEMPERATURA_H

#include <stdint.h>

/**
 * @defgroup temperatura ESW.2.1.4 - Temperatura
 * @brief Lectura filtrada de la temperatura ambiente.
 * @{
 */

/** @brief Cantidad de lecturas promediadas. */
#define TEMPERATURA_MUESTRAS 4U

/**
 * @brief Tomar una lectura del LM35 y devolver el promedio de las ultimas cuatro.
 * @details Guarda LM35_Read en una ventana circular de TEMPERATURA_MUESTRAS
 * lecturas y devuelve su promedio entero. Las primeras llamadas promedian solo
 * las lecturas ya tomadas.
 * @return Temperatura en centesimas de grado Celsius (2500 = 25,00 grados).
 * @pre Sistema_Init devolvio true: ya hay una conversion real del LM35.
 * @note USW.2.1.4.3. Una llamada por periodo de 500 ms da un filtro de 2 s.
 * Guarda estado propio; llamar solo desde el lazo principal.
 */
int32_t Temperatura_Leer(void);

/** @} */
#endif /* TPI_TEMPERATURA_H */
