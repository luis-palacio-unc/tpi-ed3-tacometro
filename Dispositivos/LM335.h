/**
 * @file LM335.h
 * @brief Temperatura del LM335 en centesimas Celsius desde su muestra en memoria.
 * @details Modelo nominal 10 mV por kelvin (2,7315 V a 0 grados Celsius); referencia
 * y ajuste de calibracion configurables en Analogica.h.
 * @note ESW.2.1.11. AD0.1/P0.24; el soporte Adquisicion_ADC_DMA configura y arranca.
 * Reemplaza al LM35 del anteproyecto, que no se consiguio.
 */
#ifndef TPI_LM335_H
#define TPI_LM335_H

#include <stdint.h>

/**
 * @defgroup lm335 ESW.2.1.11 - LM335
 * @brief Consulta escalar de temperatura, sin estados propios del dispositivo.
 * @{
 */

/** @brief Centesimas por grado Celsius; 2500 unidades representan 25,00 grados. */
#define LM335_CENTESIMAS_POR_GRADO 100U

/**
 * @brief Consultar la muestra actual y devolver su temperatura en centesimas.
 * @return Temperatura int32_t con signo; redondeo al cercano, empate arriba. Sin
 * ajuste va de -27315 (0 V) a la que corresponde al fondo de escala: 5677 con la
 * referencia de 3300 mV.
 * @pre ADC_DMA_LM335 contiene la muestra del canal LM335, transferida en una palabra
 * de 32 bits alineada. ADC_VREF_MV corresponde a la referencia utilizada. La primera
 * adquisicion debe estar completa, sin escritores CPU concurrentes en ejecucion.
 * @note El nombre no menciona el modelo del sensor: quien la llama (Temperatura.c)
 * no cambia si se reemplaza el dispositivo por otro que entregue centesimas Celsius.
 * USW.2.1.11.1. Una consulta volatile y conversion entera; no inicia ADC/DMA,
 * espera, valida flags/canal o modifica la muestra. Sin cache ni estados adicionales.
 * Durante actualizacion puede devolver la muestra anterior o nueva. No acredita edad,
 * coherencia multicanal, rango electrico o conexion del sensor: una entrada a 0 V
 * (sensor sin polarizar o en corto) se informa como -273,15 grados y una entrada
 * saturada como el fondo de escala. Ningun valor indica disponibilidad.
 * El error aritmetico del redondeo es <=0,005 grados, no la precision fisica; el
 * LM335 sin calibrar se aparta varios grados y se corrige con LM335_AJUSTE_CENTESIMAS.
 */
int32_t getTemperatura(void);

/** @} */
#endif /* TPI_LM335_H */
