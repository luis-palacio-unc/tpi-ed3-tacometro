#include "Tiempo.h"
#include <stdio.h>
#include <stddef.h>
#define CHECK(e) do { if(!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); return false; } } while(0)
typedef struct { uint32_t ms; bool disponible; unsigned llamadas; void *recibido; } Fuente;
static bool Proveer(void *contexto,uint32_t *ms)
{
    Fuente *f=contexto; ++f->llamadas; f->recibido=contexto; *ms=f->ms; return f->disponible;
}
static bool LeerCeroYExtremos(void)
{
    Fuente f={0U,true,0U,NULL}; const Tiempo_Fuente fuente={Proveer,&f}; Tiempo_Lectura l;
    CHECK(Tiempo_Leer(&fuente,&l)==TIEMPO_VALIDO && l.milisegundos==0U && l.estado==TIEMPO_VALIDO);
    f.ms=UINT32_MAX; CHECK(Tiempo_Leer(&fuente,&l)==TIEMPO_VALIDO && l.milisegundos==UINT32_MAX);
    CHECK(f.llamadas==2U && f.recibido==&f); return true;
}
static bool Argumentos(void)
{
    Fuente f={17U,true,0U,NULL}; const Tiempo_Fuente fuente={Proveer,&f};
    const Tiempo_Fuente incompleta={NULL,&f}; Tiempo_Lectura l={999U,TIEMPO_VALIDO};
    CHECK(Tiempo_Leer(&fuente,NULL)==TIEMPO_ERROR_ARGUMENTO && f.llamadas==0U);
    CHECK(Tiempo_Leer(NULL,&l)==TIEMPO_ERROR_ARGUMENTO && l.milisegundos==0U && l.estado==TIEMPO_ERROR_ARGUMENTO);
    CHECK(Tiempo_Leer(&incompleta,&l)==TIEMPO_ERROR_ARGUMENTO && f.llamadas==0U);
    CHECK(Tiempo_Transcurrido(&fuente,0U,NULL)==TIEMPO_ERROR_ARGUMENTO && f.llamadas==0U);
    CHECK(Tiempo_Transcurrido(NULL,UINT32_MAX,&l)==TIEMPO_ERROR_ARGUMENTO && l.milisegundos==0U);
    return true;
}
static bool ContextosIndependientes(void)
{
    Fuente a={10U,true,0U,NULL},b={1000U,true,0U,NULL};
    const Tiempo_Fuente fa={Proveer,&a},fb={Proveer,&b}; Tiempo_Lectura l;
    CHECK(Tiempo_Leer(&fa,&l)==TIEMPO_VALIDO && l.milisegundos==10U);
    CHECK(Tiempo_Leer(&fb,&l)==TIEMPO_VALIDO && l.milisegundos==1000U);
    CHECK(a.llamadas==1U && b.llamadas==1U && a.recibido==&a && b.recibido==&b);
    return true;
}
static bool FalloYRecuperacion(void)
{
    Fuente f={UINT32_MAX,false,0U,NULL}; const Tiempo_Fuente fuente={Proveer,&f}; Tiempo_Lectura l={999U,TIEMPO_VALIDO};
    CHECK(Tiempo_Leer(&fuente,&l)==TIEMPO_NO_DISPONIBLE && l.estado==TIEMPO_NO_DISPONIBLE && l.milisegundos==0U);
    f.disponible=true; f.ms=42U;
    CHECK(Tiempo_Leer(&fuente,&l)==TIEMPO_VALIDO && l.milisegundos==42U && f.llamadas==2U);
    return true;
}
static bool IntervaloNormal(void)
{
    Fuente f={120U,true,0U,NULL}; const Tiempo_Fuente fuente={Proveer,&f}; Tiempo_Lectura l;
    CHECK(Tiempo_Transcurrido(&fuente,100U,&l)==TIEMPO_VALIDO && l.milisegundos==20U);
    CHECK(f.llamadas==1U); CHECK(Tiempo_Transcurrido(&fuente,120U,&l)==TIEMPO_VALIDO && l.milisegundos==0U);
    return true;
}
static bool IntervaloRollover(void)
{
    Fuente f={3U,true,0U,NULL}; const Tiempo_Fuente fuente={Proveer,&f}; Tiempo_Lectura l;
    CHECK(Tiempo_Transcurrido(&fuente,UINT32_MAX-4U,&l)==TIEMPO_VALIDO && l.milisegundos==8U);
    return true;
}
static bool Semirrango(void)
{
    Fuente f={INT32_MAX,true,0U,NULL}; const Tiempo_Fuente fuente={Proveer,&f}; Tiempo_Lectura l;
    CHECK(Tiempo_Transcurrido(&fuente,0U,&l)==TIEMPO_VALIDO && l.milisegundos==INT32_MAX);
    f.ms=UINT32_C(0x80000000); CHECK(Tiempo_Transcurrido(&fuente,0U,&l)==TIEMPO_ERROR_INTERVALO && l.milisegundos==0U && l.estado==TIEMPO_ERROR_INTERVALO);
    f.ms=0U; CHECK(Tiempo_Transcurrido(&fuente,1U,&l)==TIEMPO_ERROR_INTERVALO && l.milisegundos==0U);
    return true;
}
static bool PrioridadFuente(void)
{
    Fuente f={UINT32_C(0x80000000),false,0U,NULL}; const Tiempo_Fuente fuente={Proveer,&f}; Tiempo_Lectura l;
    CHECK(Tiempo_Transcurrido(&fuente,0U,&l)==TIEMPO_NO_DISPONIBLE && l.milisegundos==0U);
    CHECK(f.llamadas==1U); f.disponible=true;
    CHECK(Tiempo_Transcurrido(&fuente,0U,&l)==TIEMPO_ERROR_INTERVALO && f.llamadas==2U);
    return true;
}
static bool InterfazNoRetenida(void)
{
    Fuente a={10U,true,0U,NULL},b={20U,true,0U,NULL}; Tiempo_Fuente fuente={Proveer,&a}; Tiempo_Lectura l;
    CHECK(Tiempo_Leer(&fuente,&l)==TIEMPO_VALIDO && l.milisegundos==10U);
    fuente.contexto=&b; CHECK(Tiempo_Leer(&fuente,&l)==TIEMPO_VALIDO && l.milisegundos==20U);
    CHECK(a.llamadas==1U && b.llamadas==1U); return true;
}
int main(void)
{
    const struct { const char *nombre; bool (*prueba)(void); } casos[]={
#define CASO(f) {#f,f}
        CASO(LeerCeroYExtremos),CASO(Argumentos),CASO(ContextosIndependientes),CASO(FalloYRecuperacion),
        CASO(IntervaloNormal),CASO(IntervaloRollover),CASO(Semirrango),CASO(PrioridadFuente),CASO(InterfazNoRetenida)
    };
    for(size_t i=0U;i<sizeof(casos)/sizeof(casos[0]);++i) {
        if(!casos[i].prueba()) { fprintf(stderr,"FAIL %s\n",casos[i].nombre); return 1; }
        printf("PASS %s\n",casos[i].nombre);
    }
    return 0;
}
