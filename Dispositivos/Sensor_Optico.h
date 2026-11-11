/**
 * @file Sensor_Optico.h
 * @brief Conteo acumulado de eventos del sensor optico mediante TIMER1.
 * @details Entrada CAP1.0 en P1.18, funcion11 de PINSEL3, contador de flancos
 * ascendentes preconfigurado. El sensor XD-51 del TPI no altera esta interfaz.
 * @note ESW.2.1.10. Configuracion fisica/arranque y ventanas se integraran al final.
 */
#ifndef TPI_SENSOR_OPTICO_H
#define TPI_SENSOR_OPTICO_H

#include <stdint.h>

/**
 * @brief Consultar el conteo de TIMER1 mediante TIM_ReadTimer original.
 * @details Entrega eventos acumulados, sin convertir a tiempo/RPM ni aplicar
 * factor de ranuras o limite de consignas. No reinicia/detiene el contador.
 * @return Conteo uint32_t modulo2^32; cero es valido, no un estado de error.
 * El consumidor resta muestras de la misma epoca con menos de2^32 eventos entre
 * ellas; tras reinicio/reconfiguracion debe adquirir una referencia nueva.
 * @pre TIMER1 exclusivo, configurado y arrancado como contador de flancos
 * ascendentes CAP1.0/P1.18, senal compatible. Bits CCR del canal en000 y ningun
 * match/captura que reinicie/detenga TC; configuracion estable y acceso privilegiado
 * serializado. TIMER0 conserva la base temporal independiente de B11.
 * @note USW.2.1.10.1. Sin contexto/estado propio/ISR/heap/esperas; no detecta
 * parada, vueltas completas omitidas, fallos electricos ni valida tiempos reales.
 */
uint32_t SensorOptico_Read(void);

#endif /* TPI_SENSOR_OPTICO_H */
