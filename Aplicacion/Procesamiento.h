/**
 * @file Procesamiento.h
 * @brief Logica de la aplicacion: modos de operacion, alarma termica y regulador PI.
 * @details Decide con magnitudes y estados logicos; no accede a perifericos ni
 * llama servicios. main.c le entrega las lecturas y aplica sus decisiones.
 * @note ESW.2.1.2. Sin heap; llamar solo desde el lazo principal.
 */
#ifndef TPI_PROCESAMIENTO_H
#define TPI_PROCESAMIENTO_H

#include "Limites.h"
#include "Mandos.h"
#include "Medicion_Velocidad.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup procesamiento ESW.2.1.2 - Procesamiento
 * @brief Modos manual/automatico, alarma con histeresis y regulacion de velocidad.
 * @{
 */

/** @brief Estado de operacion del tacometro. */
typedef enum {
    PROCESAMIENTO_MANUAL = 0, /**< El potenciometro fija el mando del motor. */
    PROCESAMIENTO_AUTO_ESPERA = 1, /**< Automatico sin consigna: mando cero. */
    PROCESAMIENTO_AUTO_ACTIVO = 2 /**< Automatico con consigna: regula el PI. */
} Procesamiento_Estado;

/**
 * @brief Fijar el estado de arranque segun la llave de modo.
 * @details MANUAL arranca en MANUAL; AUTOMATICO arranca en AUTO_ESPERA, sin
 * consigna.
 * @param[in] modo Modo leido de la llave al encender.
 * @return Estado inicial.
 * @note USW.2.1.2.11. Borra la consigna anterior y la integral del regulador.
 */
Procesamiento_Estado Procesamiento_InicializarModo(Mandos_Modo modo);

/**
 * @brief Actualizar el estado con la llave y la consigna recibida en el periodo.
 * @details Reglas, en este orden:
 * -# Con la llave en MANUAL el estado es MANUAL y se borran la consigna y la
 *    integral del regulador.
 * -# Al pasar de MANUAL a AUTOMATICO el estado es AUTO_ESPERA y la consigna de
 *    esta misma llamada se descarta: solo vale una consigna posterior al cambio.
 * -# En AUTO_ESPERA o AUTO_ACTIVO, una consigna de 0 a TPI_RPM_MAX (185) pasa a
 *    AUTO_ACTIVO y reemplaza a la anterior; una mayor se ignora.
 * @param[in] modo Modo leido de la llave en este periodo.
 * @param[in] hay_consigna true si se recibio una consigna en este periodo.
 * @param[in] consigna_rpm Consigna recibida en RPM; se ignora si hay_consigna es false.
 * @return Estado resultante.
 * @pre Procesamiento_InicializarModo ejecutado.
 * @note USW.2.1.2.12. Una llamada por periodo de 500 ms. Si el retorno pasa de
 * MANUAL a AUTO_ESPERA, main descarta la recepcion pendiente.
 */
Procesamiento_Estado Procesamiento_ActualizarModo(Mandos_Modo modo, bool hay_consigna,
                                                  uint32_t consigna_rpm);

/**
 * @brief Consultar la consigna vigente.
 * @return Consigna en RPM, de 0 a TPI_RPM_MAX; cero fuera de AUTO_ACTIVO.
 * @note USW.2.1.2.13. Sin efectos laterales.
 */
uint32_t Procesamiento_Consigna(void);

/**
 * @brief Decidir la alarma de temperatura con histeresis.
 * @details Apagada, se enciende si la temperatura supera
 * TPI_ALARMA_UMBRAL_CENTESIMAS (45,00 grados). Encendida, se apaga si la
 * temperatura es menor o igual al umbral menos TPI_ALARMA_HISTERESIS_CENTESIMAS
 * (43,00 grados). Entre ambos valores conserva la decision anterior.
 * @param[in] temperatura_centesimas Temperatura en centesimas de grado Celsius.
 * @return true si la alarma debe estar encendida.
 * @note USW.2.1.2.14. Arranca apagada. Una llamada por periodo de 500 ms. No
 * detiene el motor ni depende del modo.
 */
bool Procesamiento_EvaluarAlarma(int32_t temperatura_centesimas);

/**
 * @brief Calcular un paso del regulador PI de velocidad.
 * @details Error = consigna - velocidad, en milesimas de RPM, sin recortar la
 * medicion. Mando = Kp * error + integral, limitado de 0 a 1000 y redondeado una
 * sola vez al entero mas cercano (el empate sube). Despues suma Ki * error a la
 * integral, limitada de 0 a 1000, salvo que el mando sin limitar este saturado en
 * el sentido del error (anti-windup): en ese caso la integral no cambia.
 * Las ganancias son las fracciones TPI_CONTROL_KP_NUM/KP_DEN y KI_NUM/KI_DEN de
 * Control.h; Ki ya incluye el paso de 500 ms. Con consigna cero devuelve cero y
 * borra la integral. La integral tambien se borra en MANUAL y al inicializar el
 * modo, de manera que cada entrada a automatico regula desde cero.
 * @param[in] consigna_rpm Consigna en RPM, de 0 a TPI_RPM_MAX.
 * @param[in] rpm_milesimas Velocidad medida en la ultima ventana, en milesimas de RPM.
 * @return Mando de 0 a TPI_MANDO_MANUAL_MAX por mil.
 * @pre Una llamada por ventana nueva de 500 ms, solo en AUTO_ACTIVO.
 * @note USW.2.1.2.9. Aritmetica entera de 64 bits, sin desborde para cualquier
 * entrada de 32 bits. Los valores de Control.h son provisionales hasta sintonizar
 * con el motor.
 */
uint16_t Procesamiento_Regular(uint32_t consigna_rpm, uint32_t rpm_milesimas);

/** @} */
#endif /* TPI_PROCESAMIENTO_H */
