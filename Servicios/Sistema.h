/**
 * @file Sistema.h
 * @brief Arranque de la plataforma: reloj del sistema y adquisicion analogica.
 * @details Reune lo que debe estar en marcha antes de inicializar cualquier otro
 * servicio, para que la aplicacion no dependa de Dispositivos.
 * @note Soporte de plataforma B18/I2, sin ESW propio en el anexo B.
 */
#ifndef TPI_SISTEMA_H
#define TPI_SISTEMA_H

#include <stdbool.h>

/**
 * @brief Configurar el reloj a 100 MHz, arrancar ADC/DMA y esperar el primer par.
 * @details Llama Reloj_Init, despues Adquisicion_ADC_DMA_Init y consulta
 * Adquisicion_ADC_DMA_HayDatos hasta que el potenciometro y el LM35 tengan una
 * conversion real, con un limite fijo de consultas (el primer par tarda unos 2 ms).
 * @return true si la adquisicion quedo en marcha y con datos; false si el DMA
 * rechazo el canal o si se agoto el limite sin recibir el primer par.
 * @pre Llamar una sola vez, desde reset y como primera accion de la aplicacion.
 * @note Soporte de plataforma B18/I2. Espera acotada por cantidad de consultas,
 * sin base de tiempos; no acredita reloj ni conversiones en la placa.
 */
bool Sistema_Init(void);

#endif /* TPI_SISTEMA_H */
