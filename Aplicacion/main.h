/**
 * @file main.h
 * @brief Pasos del programa principal: inicializacion y una vuelta del lazo.
 * @details main() solo los encadena. Estan separados para poder ejecutarlos en
 * la PC con dispositivos simulados, sin depender del lazo infinito.
 * @note ESW.2.1.1.
 */
#ifndef TPI_MAIN_H
#define TPI_MAIN_H

#include <stdbool.h>

/**
 * @brief Inicializar la plataforma, los servicios y el estado de la aplicacion.
 * @details Orden: Sistema_Init (reloj y ADC/DMA); Driver_Motor_Init y mando cero;
 * Driver_Alarmas_Init y alarma apagada; Mandos_Init; Comunicacion_Init; modo
 * inicial segun la llave; regulador PI con las ganancias de Control.h;
 * Medicion_Velocidad_Init al final, para que la primera ventana de 500 ms empiece
 * con todo listo. Si Sistema_Init falla, deja el motor en cero, enciende la
 * alarma como aviso y no inicializa el resto.
 * @return true si el sistema quedo listo para iterar; false si fallo el arranque
 * de la adquisicion analogica.
 * @pre Llamar una sola vez, desde reset.
 * @note USW.2.1.1.2.
 */
bool Aplicacion_Inicializar(void);

/**
 * @brief Ejecutar una vuelta del lazo principal, sin esperas.
 * @details En cada vuelta, si el estado es MANUAL, aplica al motor la lectura del
 * potenciometro. Cuando el servicio de medicion entrega una velocidad nueva (una
 * vez cada 500 ms) ejecuta el periodo de control, en este orden:
 * -# Alarma: temperatura filtrada, decision con histeresis, LED y buzzer.
 * -# Modo: llave y consignas recibidas (vale la ultima). Al pasar de MANUAL a
 *    automatico descarta la recepcion pendiente y reinicia el PI.
 * -# Mando: MANUAL usa el potenciometro; AUTO_ESPERA aplica cero y reinicia el
 *    PI; AUTO_ACTIVO aplica la salida del regulador PI, o cero si este no la
 *    calculo.
 * -# Telemetria: una trama con velocidad, temperatura y estado.
 * @pre Aplicacion_Inicializar devolvio true.
 * @note USW.2.1.1.3.
 */
void Aplicacion_Iterar(void);

#endif /* TPI_MAIN_H */
