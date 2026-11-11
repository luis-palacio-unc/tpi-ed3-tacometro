/**
 * @file Driver_Alarmas.h
 * @brief Senalizacion de la alarma: LED rojo y buzzer juntos.
 * @details Unico camino de la aplicacion hacia las salidas de alarma.
 * @note ESW.2.1.6. Usa los dispositivos LED y Buzzer.
 */
#ifndef TPI_DRIVER_ALARMAS_H
#define TPI_DRIVER_ALARMAS_H

#include <stdbool.h>

/**
 * @defgroup driver_alarmas ESW.2.1.6 - Driver de alarmas
 * @brief Inicializacion y accionamiento de la alarma visual y acustica.
 * @{
 */

/**
 * @brief Configurar el LED y el buzzer, que arrancan apagados.
 * @details Llama LED_Init y Buzzer_Init.
 * @pre Sistema_Init ejecutado.
 * @note USW.2.1.6.2. Llamar una vez antes de Driver_Alarmas_Aplicar.
 */
void Driver_Alarmas_Init(void);

/**
 * @brief Encender o apagar a la vez el LED rojo y el buzzer.
 * @details Entrega el mismo valor a LED_Write y a Buzzer_Write; cada dispositivo
 * resuelve su polaridad.
 * @param[in] activa true enciende ambas salidas; false las apaga.
 * @pre Driver_Alarmas_Init ejecutado.
 * @note USW.2.1.6.3. Sin estado propio; se puede repetir el mismo valor.
 */
void Driver_Alarmas_Aplicar(bool activa);

/** @} */
#endif /* TPI_DRIVER_ALARMAS_H */
