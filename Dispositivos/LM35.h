/**
 * @file LM35.h
 * @brief Temperatura del LM35 en centesimas Celsius desde su muestra en memoria.
 * @details Modelo nominal 10 mV por grado; referencia configurable en Analogica.h.
 * @note ESW.2.1.11. AD0.1/P0.24; el soporte Adquisicion_ADC_DMA configura y arranca.
 */
#ifndef TPI_LM35_H
#define TPI_LM35_H

#include <stdint.h>

/**
 * @defgroup lm35 ESW.2.1.11 - LM35
 * @brief Consulta escalar de temperatura, sin estados propios del dispositivo.
 * @{
 */

/** @brief Centesimas por grado Celsius; 2500 unidades representan 25,00 grados. */
#define LM35_CENTESIMAS_POR_GRADO 100U

/**
 * @brief Consultar la muestra actual y devolver su temperatura en centesimas.
 * @return Temperatura int32_t no negativa; redondeo al cercano, empate arriba.
 * @pre ADC_DMA_LM35 contiene la muestra del canal LM35, transferida en una palabra
 * de 32 bits alineada. ADC_VREF_MV corresponde a la referencia utilizada. La primera
 * adquisicion debe estar completa, sin escritores CPU concurrentes en ejecucion.
 * @note USW.2.1.11.3. Una consulta volatile y conversion entera; no inicia ADC/DMA,
 * espera, valida flags/canal o modifica la muestra. Sin cache ni estados adicionales.
 * Durante actualizacion puede devolver la muestra anterior o nueva. No acredita edad,
 * coherencia multicanal, rango electrico o conexion del sensor. Cero no indica disponibilidad.
 * El error aritmetico del redondeo es <=0,005 grados, no la precision fisica.
 */
int32_t LM35_Read(void);

/** @} */
#endif /* TPI_LM35_H */
