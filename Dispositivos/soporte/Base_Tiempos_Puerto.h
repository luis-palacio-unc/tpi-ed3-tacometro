/**
 * @file Base_Tiempos_Puerto.h
 * @brief Contrato privado de accesos SysTick y secciones criticas de B11.
 * @details Solo lo usan soporte y pruebas; no es interfaz para servicios/aplicacion.
 * @note ESW.2.1.17. Backend ARM usa CMSIS existente, modelo host no ejecuta MCU.
 */
#ifndef TPI_BASE_TIEMPOS_PUERTO_H
#define TPI_BASE_TIEMPOS_PUERTO_H
#include <stdint.h>
/** @addtogroup base_tiempos
 * @{
 */
/** @brief ENABLE, TICKINT y CLKSOURCE=CPU; resto de bits escribibles en cero. */
#define BASE_TIEMPOS_CONTROL_ACTIVO UINT32_C(7)
/** @brief Escritura directa del bit PENDSTCLR de ICSR, sin read-modify-write. */
#define BASE_TIEMPOS_LIMPIAR_PENDIENTE UINT32_C(0x02000000)
/** @brief Prioridad logica minima, cinco bits implementados en LPC1769. */
#define BASE_TIEMPOS_PRIORIDAD_MINIMA UINT32_C(31)

/** @brief Operaciones privadas de escritura; no representan direcciones publicas. */
typedef enum {
    BASE_TIEMPOS_REG_CONTROL, /**< SysTick CTRL. */
    BASE_TIEMPOS_REG_RECARGA, /**< SysTick LOAD. */
    BASE_TIEMPOS_REG_ACTUAL, /**< SysTick VAL; escribir cero para reiniciar. */
    BASE_TIEMPOS_REG_PENDIENTE, /**< SCB ICSR; exclusivamente PENDSTCLR. */
    BASE_TIEMPOS_REG_PRIORIDAD /**< Prioridad SysTick, independiente de otras IRQ. */
} Base_Tiempos_Registro;

/**
 * @brief Guardar PRIMASK y enmascarar IRQ configurables con barreras CPU/compilador.
 * @return PRIMASK previo que debe restaurarse exactamente al salir.
 * @pre Lazo privilegiado serializado; no invocar desde ISR/NMI.
 * @note USW.2.1.17.6. Puerto interno; no cambia BASEPRI, FAULTMASK o agrupamiento.
 */
uint32_t Base_Tiempos_Puerto_EntrarCritica(void);
/**
 * @brief Completar accesos y restaurar mascara previa con barreras CPU/compilador.
 * @param[in] anterior PRIMASK devuelto por la entrada correspondiente, sin modificar.
 * @details No devuelve valor; puede permitir atender IRQ pendientes tras restaurar.
 * @pre Seccion critica correspondiente activa, desde el lazo privilegiado.
 * @note USW.2.1.17.7. No habilita interrupciones si el llamador las tenia deshabilitadas.
 */
void Base_Tiempos_Puerto_SalirCritica(uint32_t anterior);
/**
 * @brief Escribir un registro propio sin leer CTRL ni hacer read-modify-write ICSR.
 * @param[in] registro Operacion privada; enum desconocido no escribe.
 * @param[in] valor Valor validado por Base_Tiempos: control 0/7, recarga 1..119999,
 * actual cero, pendiente solo PENDSTCLR o prioridad 31, segun registro.
 * @details No devuelve valor; no hay confirmacion fisica ni validacion general de MMIO.
 * @pre Seccion critica activa, SysTick exclusivo y parametros internos coherentes.
 * @note USW.2.1.17.8. Prioridad usa CMSIS, conserva otros campos de prioridad.
 */
void Base_Tiempos_Puerto_Escribir(Base_Tiempos_Registro registro, uint32_t valor);
/** @} */
#endif
