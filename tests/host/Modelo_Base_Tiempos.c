#include "Modelo_Base_Tiempos.h"
#include <string.h>
Modelo_Base modelo_base;
static void Registrar(Modelo_Operacion op, Base_Tiempos_Registro reg, uint32_t valor)
{
    if (modelo_base.cantidad < 64U) {
        modelo_base.eventos[modelo_base.cantidad++] = (Modelo_Evento){op, reg, valor};
    }
}
void Modelo_Limpiar(void) { memset(&modelo_base, 0, sizeof(modelo_base)); }
void Modelo_LimpiarTraza(void) { modelo_base.cantidad = 0U; }
void Modelo_Despachar(void)
{
    if (modelo_base.pendiente && modelo_base.primask == 0U && (modelo_base.control & 3U) == 3U) {
        modelo_base.pendiente = false;
        ++modelo_base.entregados;
        SysTick_Handler();
    }
}
void Modelo_Tick(void)
{
    if ((modelo_base.control & 3U) == 3U) { modelo_base.pendiente = true; }
    Modelo_Despachar();
}
void Modelo_Avanzar(uint32_t ticks)
{
    for (uint32_t i = 0U; i < ticks; ++i) { Modelo_Tick(); }
}
uint32_t Base_Tiempos_Puerto_EntrarCritica(void)
{
    const uint32_t anterior = modelo_base.primask;
    Registrar(MODELO_ENTRAR, BASE_TIEMPOS_REG_CONTROL, anterior);
    modelo_base.primask = 1U;
    return anterior;
}
void Base_Tiempos_Puerto_SalirCritica(uint32_t anterior)
{
    Registrar(MODELO_SALIR, BASE_TIEMPOS_REG_CONTROL, anterior);
    if (modelo_base.primask != 1U) { modelo_base.error = true; }
    if (modelo_base.inyectar_al_salir) {
        modelo_base.pendiente = true;
        modelo_base.inyectar_al_salir = false;
    }
    modelo_base.primask = anterior;
    Modelo_Despachar();
}
void Base_Tiempos_Puerto_Escribir(Base_Tiempos_Registro reg, uint32_t valor)
{
    Registrar(MODELO_ESCRIBIR, reg, valor);
    if (modelo_base.primask != 1U) { modelo_base.error = true; }
    switch (reg) {
    case BASE_TIEMPOS_REG_CONTROL: modelo_base.control = valor; break;
    case BASE_TIEMPOS_REG_RECARGA: modelo_base.recarga = valor; break;
    case BASE_TIEMPOS_REG_ACTUAL: modelo_base.actual = 0U; break;
    case BASE_TIEMPOS_REG_PENDIENTE:
        modelo_base.icsr = valor;
        if (valor == UINT32_C(0x02000000)) { modelo_base.pendiente = false; }
        else { modelo_base.error = true; }
        break;
    case BASE_TIEMPOS_REG_PRIORIDAD: modelo_base.prioridad = valor; break;
    default: modelo_base.error = true; break;
    }
}
