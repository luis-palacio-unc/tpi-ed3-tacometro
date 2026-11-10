/**
 * @file Base_Tiempos.h
 * @brief Referencia modular de milisegundos atendidos por SysTick del LPC1769.
 * @details Configuracion explicita de CPU, lectura coherente y parada/reinicio.
 * No configura PLL/reloj/flash, no espera ni implementa adquisicion o planificador.
 * @note ESW.2.1.17: extension local de soporte, no numeracion del anexo B original.
 */
#ifndef TPI_BASE_TIEMPOS_H
#define TPI_BASE_TIEMPOS_H
#include <stdint.h>

/**
 * @defgroup base_tiempos ESW.2.1.17 - Base de tiempos
 * @brief Propietario unico de SysTick; APIs para lazo principal privilegiado.
 * @details Contexto privado unico, sin heap. Acceso serializado desde el lazo;
 * no llamar APIs desde ISR/NMI ni tareas concurrentes. El puerto preserva PRIMASK.
 * Solo ACTIVA habilita usar el valor como tiempo actual. El contador avanza por
 * interrupciones atendidas, no recupera ticks colapsados. CPU estable y latencia
 * suficiente para atender cada tick son precondiciones externas. Suspension,
 * debug y cambios de reloj invalidan equivalencia con tiempo transcurrido real.
 * El reloj declarado debe corresponder al configurado; no se comprueba fisicamente.
 * @{
 */

/** @brief Frecuencia minima Hz: LOAD=1 para un tick cada 1 ms. */
#define BASE_TIEMPOS_CPU_MIN_HZ UINT32_C(2000)
/** @brief Limite CPU del LPC1769 en Hz; no configura esa frecuencia. */
#define BASE_TIEMPOS_CPU_MAX_HZ UINT32_C(120000000)
/** @brief Ticks por segundo; resolucion de la referencia igual a 1 ms. */
#define BASE_TIEMPOS_TICKS_POR_SEGUNDO UINT32_C(1000)

/** @brief Disponibilidad del tiempo o diagnostico; distinto de cero ms valido. */
typedef enum {
    BASE_TIEMPOS_SIN_DATOS = 0, /**< Nunca inicializada; contador cero. */
    BASE_TIEMPOS_ACTIVA, /**< Referencia habilitada, sujeta a precondiciones temporales. */
    BASE_TIEMPOS_DETENIDA, /**< Tiempo congelado; no usar como reloj actual. */
    BASE_TIEMPOS_ERROR_ARGUMENTO, /**< Retorno de Leer(NULL), sin alterar la base. */
    BASE_TIEMPOS_ERROR_FRECUENCIA /**< Base deshabilitada y a cero hasta reinicio valido. */
} Base_Tiempos_Estado;

/** @brief Instantanea coherente de la epoca actual; destino exclusivo del llamador. */
typedef struct {
    uint32_t milisegundos; /**< Contador modulo 2^32; cero si SIN_DATOS/ERROR_FRECUENCIA. */
    Base_Tiempos_Estado estado; /**< ACTIVA habilita consumo; DETENIDA informa tiempo congelado. */
} Base_Tiempos_Lectura;

/**
 * @brief Configurar o reiniciar SysTick y comenzar una epoca desde cero.
 * @details Valida frecuencia 2000..120000000 Hz, multiplo de 1000; recarga Hz/1000-1.
 * Deshabilita SysTick, borra contador hardware/evento pendiente y descarta tiempo
 * anterior incluso si frecuencia es invalida. Frecuencia valida configura prioridad
 * minima 31 y fuente CPU; no cambia prioridades ajenas ni agrupamiento NVIC.
 * @param[in] frecuencia_cpu_hz Frecuencia real y estable de CPU, suministrada por integrador.
 * @return ACTIVA o ERROR_FRECUENCIA; mismo estado queda disponible al leer.
 * @pre Propiedad exclusiva de SysTick y acceso desde lazo privilegiado serializado.
 * @note USW.2.1.17.1. Preserva PRIMASK. Reiniciar consumidores temporales al cambiar
 * epoca; no existe reanudacion conservando tiempo. No inicializa el reloj de CPU.
 */
Base_Tiempos_Estado Base_Tiempos_Inicializar(uint32_t frecuencia_cpu_hz);

/**
 * @brief Obtener estado y tiempo capturados dentro de una misma seccion critica.
 * @details Escribe destino tras restaurar PRIMASK; puede atenderse un tick posterior
 * al snapshot antes de retornar. La instantanea sigue siendo coherente y anterior.
 * @param[out] lectura Destino vivo/escribible exclusivo; no compartir con ISR.
 * @return Estado capturado SIN_DATOS/ACTIVA/DETENIDA/ERROR_FRECUENCIA, tambien en
 * lectura->estado; ERROR_ARGUMENTO si NULL sin acceder al puerto ni alterar la base.
 * @pre API desde lazo privilegiado serializado; solo consumir tiempo actual en ACTIVA.
 * @note USW.2.1.17.2. Resta temporal modular requiere cadencia/limites del consumidor;
 * no compara marcas ni detecta vueltas completas omitidas. No es orden de eventos B01.
 */
Base_Tiempos_Estado Base_Tiempos_Leer(Base_Tiempos_Lectura *lectura);

/**
 * @brief Deshabilitar SysTick, cancelar pendiente y congelar contador software.
 * @details ACTIVA pasa a DETENIDA; conserva SIN_DATOS/ERROR_FRECUENCIA/DETENIDA.
 * Repetir es idempotente para tiempo y diagnostico; limpia periférico cada vez.
 * @return DETENIDA, SIN_DATOS o ERROR_FRECUENCIA segun estado previo.
 * @pre API desde lazo privilegiado serializado y propiedad exclusiva del periferico.
 * @note USW.2.1.17.3. Preserva PRIMASK; solo reinicializar vuelve a activar desde cero.
 */
Base_Tiempos_Estado Base_Tiempos_Detener(void);

/**
 * @brief Atender excepcion SysTick incrementando el contador solo cuando ACTIVA.
 * @details Incremento uint32_t modular; sin registros, callbacks, espera o servicios.
 * @details No devuelve valor; puede modificar contador privado una vez por excepcion atendida.
 * @pre Invocacion por vector SysTick. El lazo no debe llamar directamente al handler.
 * @note USW.2.1.17.4. Handler fuerte reemplaza alias por defecto; no habilita SysTick.
 * Ticks pendientes colapsados no se reconstruyen; ninguna precision fisica acreditada.
 */
void SysTick_Handler(void);

/** @} */
#endif /* TPI_BASE_TIEMPOS_H */
