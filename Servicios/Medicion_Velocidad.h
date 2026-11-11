/**
 * @file Medicion_Velocidad.h
 * @brief Velocidad del eje en milesimas de RPM, una medicion cada 500 ms.
 * @details Convierte los pulsos del sensor optico contados en cada ventana del
 * Timer. La llegada de una medicion nueva marca el periodo del lazo de control.
 * @note ESW.2.1.3. Usa los dispositivos Sensor_Optico y Timer.
 */
#ifndef TPI_MEDICION_VELOCIDAD_H
#define TPI_MEDICION_VELOCIDAD_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup medicion_velocidad ESW.2.1.3 - Medicion de velocidad
 * @brief Medicion periodica de la velocidad de giro.
 * @{
 */

/** @brief Unidades de salida por RPM; una unidad equivale a 0,001 RPM. */
#define MEDICION_VELOCIDAD_ESCALA_RPM 1000U

/**
 * @brief Poner en marcha el conteo de pulsos y las ventanas de 500 ms.
 * @details Llama SensorOptico_Init, Timer_Init y Timer_Start, en ese orden. La
 * primera ventana empieza en esta llamada.
 * @pre Sistema_Init ejecutado. Llamar una vez, al final de la inicializacion.
 * @note USW.2.1.3.3. Habilita la interrupcion de TIMER0.
 */
void Medicion_Velocidad_Init(void);

/**
 * @brief Entregar la velocidad de la ultima ventana cerrada, una sola vez.
 * @details Si Timer_ConsumirVentana informa una ventana nueva, calcula
 * pulsos * 60000 * 1000 / (TPI_MEDICION_PULSOS_POR_VUELTA * TPI_CONTROL_PERIODO_MS)
 * en 64 bits, sin redondeo, y satura en UINT32_MAX. Con 60 ranuras y 500 ms cada
 * pulso vale 2,000 RPM: 50 pulsos son 100,000 RPM.
 * @param[out] rpm_milesimas Destino de la velocidad en milesimas de RPM; solo se
 * escribe cuando se retorna true.
 * @return true una vez por cada ventana de 500 ms; false si todavia no cerro una
 * ventana nueva.
 * @pre Medicion_Velocidad_Init ejecutado; llamar desde el lazo principal.
 * @note USW.2.1.3.4. El true es el periodo del lazo de control. No recorta a 185 RPM.
 */
bool Medicion_Velocidad_Actualizar(uint32_t *rpm_milesimas);

/** @} */
#endif /* TPI_MEDICION_VELOCIDAD_H */
