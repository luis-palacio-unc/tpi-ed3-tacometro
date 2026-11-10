/**
 * @file Tiempo.c
 * @brief Lectura portable e intervalos en milisegundos mediante proveedor externo.
 * @details Validacion/limpieza completas por llamada; resta modular acotada.
 * @note ESW.2.1.18. Sin reloj propio, registros, estado global mutable, heap o esperas.
 */
#include "Tiempo.h"
#include <stddef.h>
/** @addtogroup tiempo
 * @{
 */
Tiempo_Estado Tiempo_Leer(const Tiempo_Fuente *fuente,Tiempo_Lectura *lectura)
{
    if(lectura==NULL) {
        return TIEMPO_ERROR_ARGUMENTO;
    }
    lectura->milisegundos=0U;
    lectura->estado=TIEMPO_ERROR_ARGUMENTO;
    if(fuente==NULL || fuente->leer==NULL) {
        return lectura->estado;
    }
    uint32_t ahora=0U;
    lectura->estado=TIEMPO_NO_DISPONIBLE;
    if(fuente->leer(fuente->contexto,&ahora)) {
        lectura->milisegundos=ahora;
        lectura->estado=TIEMPO_VALIDO;
    }
    return lectura->estado;
}

Tiempo_Estado Tiempo_Transcurrido(const Tiempo_Fuente *fuente,uint32_t inicio_ms,Tiempo_Lectura *intervalo)
{
    const Tiempo_Estado estado=Tiempo_Leer(fuente,intervalo);
    if(estado!=TIEMPO_VALIDO) {
        return estado;
    }
    const uint32_t delta=(uint32_t)(intervalo->milisegundos-inicio_ms);
    if(delta>TIEMPO_INTERVALO_MAX_MS) {
        intervalo->milisegundos=0U;
        intervalo->estado=TIEMPO_ERROR_INTERVALO;
        return intervalo->estado;
    }
    intervalo->milisegundos=delta;
    return TIEMPO_VALIDO;
}
/** @} */
