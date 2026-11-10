/**
 * @file Base_Tiempos_Puerto.h
 * @brief Puerto privado de TIMER0; ningun registro sale de Dispositivos.
 * @details Operaciones concretas de configuracion y lectura, sin marco MMIO generico.
 * @note ESW.2.1.17. Backend real CMSIS en ARM, dobles de encabezados solo en tests.
 */
#ifndef TPI_BASE_TIEMPOS_PUERTO_H
#define TPI_BASE_TIEMPOS_PUERTO_H
#include <stdint.h>
/** @addtogroup base_tiempos
 * @{
 */
/**
 * @brief Alimentar y preparar TIMER0 detenido/reset, con match/captura/IRQ deshabilitados.
 * @details Preserva bits PCONP ajenos y relojes. Borra TC/PC, IR con escritura directa;
 * no configura pines, PLL o PCLKSEL. No devuelve valor.
 * @pre Lazo privilegiado; TIMER0 exclusivo y escrituras PCONP serializadas.
 * @note USW.2.1.17.10. Deshabilita/limpia solamente IRQ TIMER0 en NVIC.
 */
void Base_Tiempos_Puerto_Preparar(void);
/**
 * @brief Escribir PR y habilitar cuenta continua sin IRQ.
 * @param[in] prescaler PR validado 0..UINT32_MAX; periodo por cuenta PR+1 ciclos PCLK.
 * @details No devuelve valor; libera reset y conserva TC/PC preparados en cero.
 * @pre Preparar ejecutado, configuracion validada y propiedad exclusiva.
 * @note USW.2.1.17.11. No usa match para reiniciar ni habilita NVIC.
 */
void Base_Tiempos_Puerto_Iniciar(uint32_t prescaler);
/**
 * @brief Consultar TC mediante una sola lectura volatil alineada de 32 bits.
 * @return Conteo hardware uint32_t, sin convertir unidades ni modificar registros.
 * @pre Timer alimentado/preparado; configuracion estable y acceso serializado.
 * @note USW.2.1.17.12. No detiene/reset ni depende de una ISR.
 */
uint32_t Base_Tiempos_Puerto_LeerTC(void);
/**
 * @brief Deshabilitar cuenta, conservar TC/PC/PR y completar escritura con DSB.
 * @details No devuelve valor ni cambia alimentacion, reloj o recursos ajenos.
 * @pre Timer preparado/activo y exclusivo, llamada desde lazo privilegiado.
 * @note USW.2.1.17.13. No modifica mascara global de interrupciones.
 */
void Base_Tiempos_Puerto_Detener(void);
/** @} */
#endif
