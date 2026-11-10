#include "Base_Tiempos.h"
#include "Mandos.h"
#include "Modelo_Base_Tiempos.h"
#include <stdio.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); return false; } } while(0)
static bool Actualizar(Mandos_Selector *s, Mandos_ModoSelector modo)
{
    Base_Tiempos_Lectura l;
    if(Base_Tiempos_Leer(&l)!=BASE_TIEMPOS_ACTIVA) { return false; }
    (void)Mandos_ActualizarSelector(s,modo,MANDOS_SELECTOR_FUENTE_VALIDA,l.milisegundos);
    return true;
}
static bool FronteraManual(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    Mandos_Selector s; CHECK(Mandos_InicializarSelector(&s,20U));
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL) && s.estado==MANDOS_SELECTOR_SIN_DATOS);
    Modelo_Avanzar(19U); CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL) && s.estado==MANDOS_SELECTOR_SIN_DATOS);
    Modelo_Tick(); CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL));
    CHECK(s.estado==MANDOS_SELECTOR_VALIDO && s.modo==MANDOS_SELECTOR_MODO_MANUAL); return true;
}
static bool FronteraAutomaticaYRebote(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(120000000U)==BASE_TIEMPOS_ACTIVA);
    Mandos_Selector s; CHECK(Mandos_InicializarSelector(&s,5U));
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL)); Modelo_Avanzar(5U);
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL) && s.modo==MANDOS_SELECTOR_MODO_MANUAL);
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO)); Modelo_Avanzar(4U);
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL)); Modelo_Tick();
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO)); Modelo_Avanzar(4U);
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO) && s.modo==MANDOS_SELECTOR_MODO_MANUAL);
    Modelo_Tick(); CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO));
    CHECK(s.estado==MANDOS_SELECTOR_VALIDO && s.modo==MANDOS_SELECTOR_MODO_AUTOMATICO); return true;
}
static bool DisponibilidadDeBase(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    Mandos_Selector s; CHECK(Mandos_InicializarSelector(&s,5U));
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL));
    CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA); Modelo_Avanzar(50U);
    CHECK(!Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL) && s.tiene_tiempo);
    CHECK(s.estado==MANDOS_SELECTOR_SIN_DATOS);
    CHECK(Base_Tiempos_Inicializar(0U)==BASE_TIEMPOS_ERROR_FRECUENCIA);
    CHECK(!Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO) && s.estado==MANDOS_SELECTOR_SIN_DATOS);
    return true;
}
static bool ReinicioDeEpocaYConsumidor(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    Mandos_Selector s; CHECK(Mandos_InicializarSelector(&s,2U));
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL)); Modelo_Avanzar(2U);
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_MANUAL) && s.estado==MANDOS_SELECTOR_VALIDO);
    CHECK(Base_Tiempos_Inicializar(4000000U)==BASE_TIEMPOS_ACTIVA);
    CHECK(Mandos_InicializarSelector(&s,2U));
    CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO) && s.estado==MANDOS_SELECTOR_SIN_DATOS);
    Modelo_Avanzar(2U); CHECK(Actualizar(&s,MANDOS_SELECTOR_MODO_AUTOMATICO));
    CHECK(s.estado==MANDOS_SELECTOR_VALIDO && s.modo==MANDOS_SELECTOR_MODO_AUTOMATICO); return true;
}
int main(void)
{
    if(!FronteraManual() || !FronteraAutomaticaYRebote() || !DisponibilidadDeBase() || !ReinicioDeEpocaYConsumidor()) { return 1; }
    puts("PASS 4 integraciones Base_Tiempos/B10"); return 0;
}
