/**
 * @file Base_Tiempos.c
 * @brief Ciclo de vida y contador de milisegundos SysTick con lectura coherente.
 * @details Estado/contador privados, protegidos por puerto; secuencia de accesos
 * compartida entre ARM y modelo host. Solo el ISR modifica tiempo mientras ACTIVA.
 * @note ESW.2.1.17. No inicia hardware desde main ni calcula ventanas de RPM.
 */
#include "Base_Tiempos.h"
#include "Base_Tiempos_Puerto.h"
#include <stddef.h>
/** @addtogroup base_tiempos
 * @{
 */
/** @brief Ticks atendidos modulo 2^32; acceso de lazo protegido frente a ISR. */
static volatile uint32_t milisegundos;
/** @brief Estado unico, inicialmente SIN_DATOS por inicializacion estatica C. */
static volatile Base_Tiempos_Estado estado_base;

/**
 * @brief Apagar contador/IRQ, reiniciar fase hardware y cancelar pendiente SysTick.
 * @details No devuelve valor; no cambia contador/estado software ni recarga/prioridad.
 * @pre Seccion critica activa y propiedad exclusiva del periferico.
 * @note USW.2.1.17.5. CTRL=0, VAL=0, ICSR=PENDSTCLR en ese orden; sin lectura ICSR.
 */
static void ApagarContador(void)
{
    Base_Tiempos_Puerto_Escribir(BASE_TIEMPOS_REG_CONTROL, 0U);
    Base_Tiempos_Puerto_Escribir(BASE_TIEMPOS_REG_ACTUAL, 0U);
    Base_Tiempos_Puerto_Escribir(BASE_TIEMPOS_REG_PENDIENTE, BASE_TIEMPOS_LIMPIAR_PENDIENTE);
}

Base_Tiempos_Estado Base_Tiempos_Inicializar(uint32_t frecuencia_cpu_hz)
{
    const uint32_t anterior = Base_Tiempos_Puerto_EntrarCritica();
    ApagarContador();
    milisegundos = 0U;
    estado_base = BASE_TIEMPOS_ERROR_FRECUENCIA;
    if (frecuencia_cpu_hz >= BASE_TIEMPOS_CPU_MIN_HZ &&
        frecuencia_cpu_hz <= BASE_TIEMPOS_CPU_MAX_HZ &&
        frecuencia_cpu_hz % BASE_TIEMPOS_TICKS_POR_SEGUNDO == 0U) {
        const uint32_t recarga = frecuencia_cpu_hz / BASE_TIEMPOS_TICKS_POR_SEGUNDO - 1U;
        Base_Tiempos_Puerto_Escribir(BASE_TIEMPOS_REG_RECARGA, recarga);
        Base_Tiempos_Puerto_Escribir(BASE_TIEMPOS_REG_PRIORIDAD, BASE_TIEMPOS_PRIORIDAD_MINIMA);
        estado_base = BASE_TIEMPOS_ACTIVA;
        Base_Tiempos_Puerto_Escribir(BASE_TIEMPOS_REG_CONTROL, BASE_TIEMPOS_CONTROL_ACTIVO);
    }
    const Base_Tiempos_Estado resultado = estado_base;
    Base_Tiempos_Puerto_SalirCritica(anterior);
    return resultado;
}

Base_Tiempos_Estado Base_Tiempos_Leer(Base_Tiempos_Lectura *lectura)
{
    if (lectura == NULL) {
        return BASE_TIEMPOS_ERROR_ARGUMENTO;
    }
    const uint32_t anterior = Base_Tiempos_Puerto_EntrarCritica();
    const uint32_t tiempo = milisegundos;
    const Base_Tiempos_Estado resultado = estado_base;
    Base_Tiempos_Puerto_SalirCritica(anterior);
    lectura->milisegundos = tiempo;
    lectura->estado = resultado;
    return resultado;
}

Base_Tiempos_Estado Base_Tiempos_Detener(void)
{
    const uint32_t anterior = Base_Tiempos_Puerto_EntrarCritica();
    ApagarContador();
    if (estado_base == BASE_TIEMPOS_ACTIVA) {
        estado_base = BASE_TIEMPOS_DETENIDA;
    }
    const Base_Tiempos_Estado resultado = estado_base;
    Base_Tiempos_Puerto_SalirCritica(anterior);
    return resultado;
}

void SysTick_Handler(void)
{
    if (estado_base == BASE_TIEMPOS_ACTIVA) {
        ++milisegundos;
    }
}
/** @} */
