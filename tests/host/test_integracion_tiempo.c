#include "Tiempo.h"
#include "Base_Tiempos.h"
#include "Mandos.h"
#include "Modelo_Timer0.h"
#include <stdio.h>
#define CHECK(e) do { if(!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); return false; } } while(0)
static const Tiempo_Fuente fuente={Base_Tiempos_ProveerMilisegundos,NULL};
static bool Actualizar(Mandos_Selector *s,Mandos_ModoSelector modo)
{
    Tiempo_Lectura tiempo;
    if(Tiempo_Leer(&fuente,&tiempo)!=TIEMPO_VALIDO) { return false; }
    (void)Mandos_ActualizarSelector(s,modo,MANDOS_SELECTOR_FUENTE_VALIDA,tiempo.milisegundos);
    return true;
}
static bool FronteraConBases(void)
{
    const uint32_t bases[]={1U,2U,5U};
    for(size_t i=0U;i<3U;++i) {
        Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,bases[i])==BASE_TIEMPOS_ACTIVA);
        Mandos_Selector s; CHECK(Mandos_InicializarSelector(&s,20U));
        CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL) && s.estado==MANDOS_SELECTOR_SIN_DATOS);
        Modelo_AvanzarPclk((uint64_t)(20U-bases[i])*25000U);
        CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL) && s.estado==MANDOS_SELECTOR_SIN_DATOS);
        Modelo_AvanzarPclk((uint64_t)bases[i]*25000U);
        CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL) && s.modo==MANDOS_SELECTOR_MODO_MANUAL && s.estado==MANDOS_SELECTOR_VALIDO);
    }
    return true;
}
static bool DosConsumidores(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,1U)==BASE_TIEMPOS_ACTIVA);
    Tiempo_Lectura a,b,l; CHECK(Tiempo_Leer(&fuente,&a)==TIEMPO_VALIDO);
    Modelo_AvanzarPclk(175000U); CHECK(Tiempo_Leer(&fuente,&b)==TIEMPO_VALIDO);
    Modelo_AvanzarPclk(325000U); CHECK(Tiempo_Transcurrido(&fuente,a.milisegundos,&l)==TIEMPO_VALIDO && l.milisegundos==20U);
    CHECK(Tiempo_Transcurrido(&fuente,b.milisegundos,&l)==TIEMPO_VALIDO && l.milisegundos==13U);
    CHECK(LPC_TIM0->TC==20U); return true;
}
static bool IndisponibilidadYRecuperacion(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,1U)==BASE_TIEMPOS_ACTIVA);
    Mandos_Selector s; CHECK(Mandos_InicializarSelector(&s,5U));
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL));
    CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA); Modelo_AvanzarPclk(250000U);
    CHECK(!Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO) && s.estado==MANDOS_SELECTOR_SIN_DATOS);
    CHECK(Base_Tiempos_Inicializar(0U,1U)==BASE_TIEMPOS_ERROR_RELOJ);
    Tiempo_Lectura l; CHECK(Tiempo_Leer(&fuente,&l)==TIEMPO_NO_DISPONIBLE && l.milisegundos==0U);
    CHECK(Base_Tiempos_Inicializar(25000000U,1U)==BASE_TIEMPOS_ACTIVA);
    CHECK(Mandos_InicializarSelector(&s,5U)); CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO));
    Modelo_AvanzarPclk(125000U); CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO) && s.estado==MANDOS_SELECTOR_VALIDO);
    return true;
}
static bool RolloverEnServicioYMandos(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,3U)==BASE_TIEMPOS_ACTIVA);
    LPC_TIM0->TC=UINT32_MAX-1U;
    Tiempo_Lectura inicio,l; CHECK(Tiempo_Leer(&fuente,&inicio)==TIEMPO_VALIDO);
    Mandos_Selector s; CHECK(Mandos_InicializarSelector(&s,6U));
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL));
    Modelo_AvanzarPclk(75000U); CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL) && s.estado==MANDOS_SELECTOR_SIN_DATOS);
    Modelo_AvanzarPclk(75000U); CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL) && s.estado==MANDOS_SELECTOR_VALIDO);
    CHECK(Tiempo_Transcurrido(&fuente,inicio.milisegundos,&l)==TIEMPO_VALIDO && l.milisegundos==6U);
    return true;
}
static bool RebotesYReinicioDeEpoca(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,1U)==BASE_TIEMPOS_ACTIVA);
    Mandos_Selector s; CHECK(Mandos_InicializarSelector(&s,5U));
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL)); Modelo_AvanzarPclk(125000U);
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL) && s.estado==MANDOS_SELECTOR_VALIDO);
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO)); Modelo_AvanzarPclk(100000U);
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL) && s.modo==MANDOS_SELECTOR_MODO_MANUAL);
    CHECK(Base_Tiempos_Inicializar(25000000U,5U)==BASE_TIEMPOS_ACTIVA);
    CHECK(Mandos_InicializarSelector(&s,5U)); CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO));
    Modelo_AvanzarPclk(125000U); CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO) && s.modo==MANDOS_SELECTOR_MODO_AUTOMATICO);
    return true;
}
int main(void)
{
    if(!FronteraConBases() || !DosConsumidores() || !IndisponibilidadYRecuperacion() || !RolloverEnServicioYMandos() || !RebotesYReinicioDeEpoca()) { return 1; }
    puts("PASS 5 integraciones TIMER0/Tiempo/B10"); return 0;
}
