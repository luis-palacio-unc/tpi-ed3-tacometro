/**
 * @file Base_Tiempos.c
 * @brief Validacion de periodo/PCLK y lectura normalizada del contador hardware.
 * @details Solo guarda periodo/estado, nunca acumula tiempo. Configuracion usa
 * uint64_t acotado para admitir PR=UINT32_MAX sin overflow intermedio.
 * @note ESW.2.1.17. No incluye ni enlaza servicios/aplicacion; no ISR ni esperas.
 */
#include "Base_Tiempos.h"
#include "Base_Tiempos_Puerto.h"
#include <stddef.h>
/** @addtogroup base_tiempos
 * @{
 */
/** @brief Periodo vigente ms, cero si sin configurar/error; solo lazo serializado. */
static uint32_t periodo_base_ms;
/** @brief Estado unico, SIN_DATOS por inicializacion estatica; nunca mutado por ISR. */
static Base_Tiempos_Estado estado_base;

Base_Tiempos_Estado Base_Tiempos_Inicializar(uint32_t frecuencia_pclk_hz,uint32_t periodo_ms)
{
    Base_Tiempos_Puerto_Preparar();
    periodo_base_ms=0U;
    estado_base=BASE_TIEMPOS_ERROR_RELOJ;
    if(frecuencia_pclk_hz==0U || frecuencia_pclk_hz>BASE_TIEMPOS_PCLK_MAX_HZ) {
        return estado_base;
    }
    estado_base=BASE_TIEMPOS_ERROR_PERIODO;
    if(periodo_ms==0U || periodo_ms>BASE_TIEMPOS_PERIODO_MAX_MS) {
        return estado_base;
    }
    estado_base=BASE_TIEMPOS_ERROR_PRESCALER;
    const uint64_t producto=(uint64_t)frecuencia_pclk_hz*periodo_ms;
    const uint64_t divisor=producto/UINT64_C(1000);
    if(producto%UINT64_C(1000)!=0U || divisor==0U || divisor>UINT64_C(0x100000000)) {
        return estado_base;
    }
    periodo_base_ms=periodo_ms;
    Base_Tiempos_Puerto_Iniciar((uint32_t)(divisor-1U));
    estado_base=BASE_TIEMPOS_ACTIVA;
    return estado_base;
}

Base_Tiempos_Estado Base_Tiempos_Leer(Base_Tiempos_Lectura *lectura)
{
    if(lectura==NULL) {
        return BASE_TIEMPOS_ERROR_ARGUMENTO;
    }
    lectura->milisegundos=0U;
    if(estado_base==BASE_TIEMPOS_ACTIVA || estado_base==BASE_TIEMPOS_DETENIDA) {
        const uint32_t conteos=Base_Tiempos_Puerto_LeerTC();
        lectura->milisegundos=(uint32_t)(conteos*periodo_base_ms);
    }
    lectura->estado=estado_base;
    return estado_base;
}

Base_Tiempos_Estado Base_Tiempos_Detener(void)
{
    if(estado_base==BASE_TIEMPOS_ACTIVA) {
        Base_Tiempos_Puerto_Detener();
        estado_base=BASE_TIEMPOS_DETENIDA;
    }
    return estado_base;
}

bool Base_Tiempos_ProveerMilisegundos(void *contexto,uint32_t *milisegundos)
{
    (void)contexto;
    if(milisegundos==NULL) {
        return false;
    }
    *milisegundos=0U;
    Base_Tiempos_Lectura lectura;
    if(Base_Tiempos_Leer(&lectura)!=BASE_TIEMPOS_ACTIVA) {
        return false;
    }
    *milisegundos=lectura.milisegundos;
    return true;
}
/** @} */
