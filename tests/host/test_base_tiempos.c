#include "../../Dispositivos/soporte/Base_Tiempos.c"
#include "Modelo_Base_Tiempos.h"
#include <stdio.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); return false; } } while (0)
static bool Lectura(Base_Tiempos_Estado estado, uint32_t ms)
{
    Base_Tiempos_Lectura l = {123U, BASE_TIEMPOS_ERROR_ARGUMENTO};
    CHECK(Base_Tiempos_Leer(&l) == estado);
    CHECK(l.estado == estado && l.milisegundos == ms && !modelo_base.error);
    return true;
}
static bool SinInicializar(void)
{
    Modelo_Limpiar();
    CHECK(Base_Tiempos_Leer(NULL)==BASE_TIEMPOS_ERROR_ARGUMENTO && modelo_base.cantidad==0U);
    CHECK(Lectura(BASE_TIEMPOS_SIN_DATOS, 0U));
    SysTick_Handler();
    CHECK(Base_Tiempos_Detener() == BASE_TIEMPOS_SIN_DATOS);
    CHECK(Lectura(BASE_TIEMPOS_SIN_DATOS, 0U));
    return true;
}
static bool ConfiguracionYOrden(void)
{
    Modelo_Limpiar();
    CHECK(Base_Tiempos_Inicializar(4000000U) == BASE_TIEMPOS_ACTIVA);
    CHECK(modelo_base.cantidad == 8U);
    const Modelo_Evento esperados[] = {
        {MODELO_ENTRAR, BASE_TIEMPOS_REG_CONTROL, 0U},
        {MODELO_ESCRIBIR, BASE_TIEMPOS_REG_CONTROL, 0U},
        {MODELO_ESCRIBIR, BASE_TIEMPOS_REG_ACTUAL, 0U},
        {MODELO_ESCRIBIR, BASE_TIEMPOS_REG_PENDIENTE, UINT32_C(0x02000000)},
        {MODELO_ESCRIBIR, BASE_TIEMPOS_REG_RECARGA, 3999U},
        {MODELO_ESCRIBIR, BASE_TIEMPOS_REG_PRIORIDAD, 31U},
        {MODELO_ESCRIBIR, BASE_TIEMPOS_REG_CONTROL, 7U},
        {MODELO_SALIR, BASE_TIEMPOS_REG_CONTROL, 0U}
    };
    for (size_t i = 0U; i < 8U; ++i) {
        CHECK(modelo_base.eventos[i].op == esperados[i].op);
        CHECK(modelo_base.eventos[i].reg == esperados[i].reg);
        CHECK(modelo_base.eventos[i].valor == esperados[i].valor);
    }
    CHECK(modelo_base.control == 7U && modelo_base.actual == 0U);
    CHECK(Lectura(BASE_TIEMPOS_ACTIVA, 0U)); return true;
}
static bool FrecuenciasValidas(void)
{
    const struct { uint32_t hz, reload; } casos[] = {
        {2000U,1U},{3000U,2U},{4000000U,3999U},{100000000U,99999U},
        {119999000U,119998U},{120000000U,119999U}
    };
    for (size_t i = 0U; i < sizeof(casos)/sizeof(casos[0]); ++i) {
        Modelo_Limpiar();
        CHECK(Base_Tiempos_Inicializar(casos[i].hz) == BASE_TIEMPOS_ACTIVA);
        CHECK(modelo_base.recarga == casos[i].reload && modelo_base.control == 7U);
        CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U));
    }
    return true;
}
static bool FrecuenciasInvalidas(void)
{
    const uint32_t casos[] = {0U,1U,999U,1000U,1999U,2001U,4000001U,120000001U,120001000U,UINT32_MAX};
    for (size_t i = 0U; i < sizeof(casos)/sizeof(casos[0]); ++i) {
        Modelo_Limpiar(); modelo_base.recarga = 555U;
        CHECK(Base_Tiempos_Inicializar(casos[i]) == BASE_TIEMPOS_ERROR_FRECUENCIA);
        CHECK(modelo_base.control == 0U && modelo_base.recarga == 555U);
        CHECK(modelo_base.cantidad == 5U);
        CHECK(Lectura(BASE_TIEMPOS_ERROR_FRECUENCIA,0U));
    }
    return true;
}
static bool ErrorInvalidaEpoca(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    Modelo_Avanzar(7U); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,7U));
    CHECK(Base_Tiempos_Inicializar(1000U)==BASE_TIEMPOS_ERROR_FRECUENCIA);
    Modelo_Avanzar(5U); SysTick_Handler();
    CHECK(Lectura(BASE_TIEMPOS_ERROR_FRECUENCIA,0U)); return true;
}
static bool Recuperacion(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(0U)==BASE_TIEMPOS_ERROR_FRECUENCIA);
    CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_ERROR_FRECUENCIA);
    CHECK(Base_Tiempos_Inicializar(120000000U)==BASE_TIEMPOS_ACTIVA);
    Modelo_Tick(); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,1U)); return true;
}
static bool TicksAtendidos(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    Modelo_Avanzar(1000U);
    CHECK(modelo_base.entregados==1000U && Lectura(BASE_TIEMPOS_ACTIVA,1000U));
    return true;
}
static bool ParadaCongela(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    Modelo_Avanzar(37U); CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA);
    CHECK(modelo_base.control==0U); Modelo_Avanzar(50U); SysTick_Handler();
    CHECK(Lectura(BASE_TIEMPOS_DETENIDA,37U)); return true;
}
static bool ParadaIdempotente(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    Modelo_Avanzar(37U);
    CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA);
    CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA);
    CHECK(Lectura(BASE_TIEMPOS_DETENIDA,37U)); return true;
}
static bool ReinicioDesdeActiva(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    Modelo_Avanzar(17U); CHECK(Base_Tiempos_Inicializar(3000U)==BASE_TIEMPOS_ACTIVA);
    CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U)); Modelo_Tick();
    CHECK(Lectura(BASE_TIEMPOS_ACTIVA,1U)); return true;
}
static bool ReinicioDesdeDetenida(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    Modelo_Avanzar(7U);
    CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA);
    CHECK(Base_Tiempos_Inicializar(2000U)==BASE_TIEMPOS_ACTIVA);
    CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U)); return true;
}
static bool NullSinEfectos(void)
{
    for (unsigned i=0U;i<3U;++i) {
        Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
        Modelo_Avanzar(3U);
        Base_Tiempos_Estado estado=BASE_TIEMPOS_ACTIVA; uint32_t ms=3U;
        if(i==1U) { estado=Base_Tiempos_Detener(); }
        if(i==2U) { estado=Base_Tiempos_Inicializar(0U); ms=0U; }
        const size_t n=modelo_base.cantidad;
        CHECK(Base_Tiempos_Leer(NULL)==BASE_TIEMPOS_ERROR_ARGUMENTO);
        CHECK(modelo_base.cantidad==n && Lectura(estado,ms));
    }
    return true;
}
static bool PreservaMascara(void)
{
    Modelo_Limpiar(); modelo_base.primask=1U;
    CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA && modelo_base.primask==1U);
    Modelo_Tick(); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U) && modelo_base.primask==1U);
    modelo_base.primask=0U; Modelo_Despachar(); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,1U));
    modelo_base.primask=1U; CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA);
    CHECK(modelo_base.primask==1U && Lectura(BASE_TIEMPOS_DETENIDA,1U));
    CHECK(Base_Tiempos_Inicializar(0U)==BASE_TIEMPOS_ERROR_FRECUENCIA && modelo_base.primask==1U);
    return true;
}
static bool InstantaneaConTickAlSalir(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    Modelo_Avanzar(5U); modelo_base.inyectar_al_salir=true;
    CHECK(Lectura(BASE_TIEMPOS_ACTIVA,5U));
    CHECK(modelo_base.entregados==6U && Lectura(BASE_TIEMPOS_ACTIVA,6U));
    return true;
}
static bool PendienteAntiguoReinicio(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    Modelo_Avanzar(7U); modelo_base.primask=1U; Modelo_Tick();
    CHECK(modelo_base.pendiente);
    CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA && !modelo_base.pendiente);
    modelo_base.primask=0U; Modelo_Despachar(); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U));
    Modelo_Tick(); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,1U)); return true;
}
static bool PendienteAntiguoParada(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    Modelo_Avanzar(9U); modelo_base.primask=1U; Modelo_Tick();
    CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA && !modelo_base.pendiente);
    modelo_base.primask=0U; Modelo_Despachar(); CHECK(Lectura(BASE_TIEMPOS_DETENIDA,9U));
    CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U)); return true;
}
static bool Rollover(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    milisegundos = UINT32_MAX-1U; /* Siembra exclusiva de test; ninguna API productiva. */
    Modelo_Tick(); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,UINT32_MAX));
    Modelo_Tick(); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U));
    Modelo_Tick(); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,1U)); return true;
}
static bool TicksColapsadosNoSeRecuperan(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    modelo_base.primask=1U; Modelo_Avanzar(10U);
    CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U));
    modelo_base.primask=0U; Modelo_Despachar();
    CHECK(Lectura(BASE_TIEMPOS_ACTIVA,1U) && modelo_base.entregados==1U);
    return true;
}
int main(void)
{
    const struct { const char *nombre; bool (*prueba)(void); } casos[] = {
#define CASO(f) {#f,f}
        CASO(SinInicializar),CASO(ConfiguracionYOrden),CASO(FrecuenciasValidas),
        CASO(FrecuenciasInvalidas),CASO(ErrorInvalidaEpoca),CASO(Recuperacion),
        CASO(TicksAtendidos),CASO(ParadaCongela),CASO(ParadaIdempotente),
        CASO(ReinicioDesdeActiva),CASO(ReinicioDesdeDetenida),CASO(NullSinEfectos),
        CASO(PreservaMascara),CASO(InstantaneaConTickAlSalir),
        CASO(PendienteAntiguoReinicio),CASO(PendienteAntiguoParada),CASO(Rollover),
        CASO(TicksColapsadosNoSeRecuperan)
    };
    for(size_t i=0U;i<sizeof(casos)/sizeof(casos[0]);++i) {
        if(!casos[i].prueba()) { fprintf(stderr,"FAIL %s\n",casos[i].nombre); return 1; }
        printf("PASS %s\n",casos[i].nombre);
    }
    return 0;
}
