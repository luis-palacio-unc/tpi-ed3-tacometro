#include "Base_Tiempos.h"
#include "Temporizacion.h"
#include "Modelo_Timer0.h"
#include <stdio.h>
#include <stddef.h>
#define CHECK(e) do { if(!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); return false; } } while(0)
static bool Lectura(Base_Tiempos_Estado estado,uint32_t ms)
{
    Base_Tiempos_Lectura l={999U,BASE_TIEMPOS_ERROR_ARGUMENTO};
    CHECK(Base_Tiempos_Leer(&l)==estado && l.estado==estado && l.milisegundos==ms);
    CHECK(!modelo_error); return true;
}
static bool SinInicializar(void)
{
    Modelo_Limpiar(); LPC_TIM0->TC=123U;
    CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_SIN_DATOS && modelo_cantidad==0U);
    CHECK(Lectura(BASE_TIEMPOS_SIN_DATOS,0U) && LPC_TIM0->TC==123U);
    uint32_t ms=999U; CHECK(!Base_Tiempos_ProveerMilisegundos(NULL,&ms) && ms==0U);
    return true;
}
static bool ConfiguracionYProteccion(void)
{
    Modelo_Limpiar();
    LPC_SC->PCONP=UINT32_C(0xA5A50000); LPC_SC->PCLKSEL0=UINT32_C(0x11223344);
    LPC_SC->PCLKSEL1=UINT32_C(0x55667788); LPC_SC->PLL0CON=3U; LPC_SC->FLASHCFG=99U;
    LPC_TIM1->TC=777U; LPC_TIM0->TC=999U; LPC_TIM0->PC=888U;
    LPC_TIM0->TCR=1U; LPC_TIM0->MCR=UINT32_MAX; LPC_TIM0->CCR=UINT32_MAX;
    LPC_TIM0->CTCR=3U; LPC_TIM0->EMR=UINT32_MAX; LPC_TIM0->MR0=123U;
    CHECK(TPI_BASE_TEMPORAL_MS==1U);
    CHECK(Base_Tiempos_Inicializar(25000000U,TPI_BASE_TEMPORAL_MS)==BASE_TIEMPOS_ACTIVA);
    CHECK(LPC_SC->PCONP==UINT32_C(0xA5A50002));
    CHECK(LPC_SC->PCLKSEL0==UINT32_C(0x11223344) && LPC_SC->PCLKSEL1==UINT32_C(0x55667788));
    CHECK(LPC_SC->PLL0CON==3U && LPC_SC->FLASHCFG==99U && LPC_TIM1->TC==777U);
    CHECK(LPC_TIM0->PR==24999U && LPC_TIM0->TCR==1U && LPC_TIM0->TC==0U && LPC_TIM0->PC==0U);
    CHECK(LPC_TIM0->CTCR==0U && LPC_TIM0->MCR==0U && LPC_TIM0->CCR==0U && LPC_TIM0->EMR==0U);
    CHECK(LPC_TIM0->IR==63U && LPC_TIM0->MR0==123U && !modelo_irq_habilitada && !modelo_irq_pendiente);
    CHECK(modelo_cantidad==4U && modelo_eventos[0].operacion=='D' && modelo_eventos[1].operacion=='C');
    CHECK(modelo_eventos[2].tcr==2U && modelo_eventos[2].tc==0U && modelo_eventos[2].pc==0U);
    CHECK(modelo_eventos[3].tcr==1U && modelo_eventos[3].pr==24999U);
    CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U)); return true;
}
static bool Bases(void)
{
    const uint32_t bases[]={1U,2U,3U,5U};
    const uint32_t esperados[]={24999U,49999U,74999U,124999U};
    for(size_t i=0U;i<4U;++i) {
        Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,bases[i])==BASE_TIEMPOS_ACTIVA);
        CHECK(LPC_TIM0->PR==esperados[i]); Modelo_AvanzarPclk(25000U);
        CHECK(Lectura(BASE_TIEMPOS_ACTIVA,bases[i]==1U?1U:0U));
        Modelo_AvanzarPclk((uint64_t)25000U*(bases[i]-1U));
        CHECK(Lectura(BASE_TIEMPOS_ACTIVA,bases[i]));
    }
    return true;
}
static bool PrescalerExtremos(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(1000U,1U)==BASE_TIEMPOS_ACTIVA && LPC_TIM0->PR==0U);
    Modelo_AvanzarPclk(1U); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,1U));
    CHECK(Base_Tiempos_Inicializar(65536000U,65536U)==BASE_TIEMPOS_ACTIVA && LPC_TIM0->PR==UINT32_MAX);
    Modelo_AvanzarPclk(UINT32_MAX); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U));
    Modelo_AvanzarPclk(1U); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,65536U));
    CHECK(Base_Tiempos_Inicializar(120000000U,1U)==BASE_TIEMPOS_ACTIVA && LPC_TIM0->PR==119999U);
    CHECK(Base_Tiempos_Inicializar(1000U,INT32_MAX)==BASE_TIEMPOS_ACTIVA && LPC_TIM0->PR==UINT32_C(0x7FFFFFFE));
    Modelo_AvanzarPclk(INT32_MAX); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,INT32_MAX));
    return true;
}
static bool RelojYPeriodoInvalidos(void)
{
    Modelo_Limpiar();
    const uint32_t relojes[]={0U,120000001U,UINT32_MAX};
    for(size_t i=0U;i<3U;++i) {
        CHECK(Base_Tiempos_Inicializar(relojes[i],1U)==BASE_TIEMPOS_ERROR_RELOJ);
        CHECK(Lectura(BASE_TIEMPOS_ERROR_RELOJ,0U) && (LPC_TIM0->TCR&1U)==0U);
    }
    const uint32_t periodos[]={0U,UINT32_C(0x80000000),UINT32_MAX};
    for(size_t i=0U;i<3U;++i) {
        CHECK(Base_Tiempos_Inicializar(25000000U,periodos[i])==BASE_TIEMPOS_ERROR_PERIODO);
        CHECK(Lectura(BASE_TIEMPOS_ERROR_PERIODO,0U));
    }
    return true;
}
static bool PrescalerInvalido(void)
{
    Modelo_Limpiar();
    CHECK(Base_Tiempos_Inicializar(999U,1U)==BASE_TIEMPOS_ERROR_PRESCALER);
    CHECK(Base_Tiempos_Inicializar(25000001U,1U)==BASE_TIEMPOS_ERROR_PRESCALER);
    CHECK(Base_Tiempos_Inicializar(65536000U,65537U)==BASE_TIEMPOS_ERROR_PRESCALER);
    CHECK(Base_Tiempos_Inicializar(120000000U,INT32_MAX)==BASE_TIEMPOS_ERROR_PRESCALER);
    CHECK(Lectura(BASE_TIEMPOS_ERROR_PRESCALER,0U) && (LPC_TIM0->TCR&1U)==0U);
    CHECK(Base_Tiempos_Inicializar(999U,1000U)==BASE_TIEMPOS_ACTIVA && LPC_TIM0->PR==998U);
    return true;
}
static bool CuentaSinConsultas(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,1U)==BASE_TIEMPOS_ACTIVA);
    Modelo_AvanzarPclk(24999U); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U));
    Modelo_AvanzarPclk(1U); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,1U));
    Modelo_AvanzarPclk(25000000U); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,1001U));
    CHECK(!modelo_irq_habilitada); return true;
}
static bool ConsultasNoAlteran(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,5U)==BASE_TIEMPOS_ACTIVA);
    Modelo_AvanzarPclk(300001U); const uint32_t tc=LPC_TIM0->TC,pc=LPC_TIM0->PC;
    const size_t n=modelo_cantidad;
    for(unsigned i=0U;i<5U;++i) { CHECK(Lectura(BASE_TIEMPOS_ACTIVA,10U)); }
    CHECK(tc==LPC_TIM0->TC && pc==LPC_TIM0->PC && LPC_TIM0->TCR==1U && modelo_cantidad==n);
    return true;
}
static bool DetenerCongela(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,2U)==BASE_TIEMPOS_ACTIVA);
    Modelo_AvanzarPclk(975000U); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,38U));
    CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA && LPC_TIM0->TCR==0U);
    Modelo_AvanzarPclk(250000000U); CHECK(Lectura(BASE_TIEMPOS_DETENIDA,38U)); return true;
}
static bool ReinicioYRecuperacion(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,1U)==BASE_TIEMPOS_ACTIVA);
    Modelo_AvanzarPclk(250000U); CHECK(Base_Tiempos_Inicializar(25000000U,5U)==BASE_TIEMPOS_ACTIVA);
    CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U));
    CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA);
    CHECK(Base_Tiempos_Inicializar(0U,1U)==BASE_TIEMPOS_ERROR_RELOJ);
    CHECK(Base_Tiempos_Inicializar(25000000U,1U)==BASE_TIEMPOS_ACTIVA);
    CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U)); return true;
}
static bool NullYProveedor(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,1U)==BASE_TIEMPOS_ACTIVA);
    size_t n=modelo_cantidad;
    CHECK(Base_Tiempos_Leer(NULL)==BASE_TIEMPOS_ERROR_ARGUMENTO && modelo_cantidad==n);
    CHECK(!Base_Tiempos_ProveerMilisegundos(NULL,NULL));
    uint32_t ms=999U; CHECK(Base_Tiempos_ProveerMilisegundos(NULL,&ms) && ms==0U);
    Modelo_AvanzarPclk(50000U); int opaco=123;
    CHECK(Base_Tiempos_ProveerMilisegundos(&opaco,&ms) && ms==2U && opaco==123);
    CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA);
    ms=999U; CHECK(!Base_Tiempos_ProveerMilisegundos(NULL,&ms) && ms==0U);
    CHECK(Base_Tiempos_Inicializar(0U,1U)==BASE_TIEMPOS_ERROR_RELOJ);
    ms=999U; CHECK(!Base_Tiempos_ProveerMilisegundos(NULL,&ms) && ms==0U); return true;
}
static bool RolloverTC(void)
{
    const uint32_t bases[]={1U,3U,5U};
    for(size_t i=0U;i<3U;++i) {
        Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,bases[i])==BASE_TIEMPOS_ACTIVA);
        LPC_TIM0->TC=UINT32_MAX; CHECK(Lectura(BASE_TIEMPOS_ACTIVA,(uint32_t)(UINT64_C(0xFFFFFFFF)*bases[i])));
        Modelo_AvanzarPclk((uint64_t)25000U*bases[i]);
        CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U));
        Modelo_AvanzarPclk((uint64_t)25000U*bases[i]); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,bases[i]));
    }
    return true;
}
static bool RolloverMS(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,2U)==BASE_TIEMPOS_ACTIVA);
    LPC_TIM0->TC=UINT32_C(0x7FFFFFFF); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,UINT32_MAX-1U));
    Modelo_AvanzarPclk(50000U); CHECK(Lectura(BASE_TIEMPOS_ACTIVA,0U));
    CHECK(LPC_TIM0->TC==UINT32_C(0x80000000)); return true;
}
static bool PrioridadYFueraRango(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(0U,0U)==BASE_TIEMPOS_ERROR_RELOJ);
    CHECK(Base_Tiempos_Inicializar(999U,0U)==BASE_TIEMPOS_ERROR_PERIODO);
    CHECK(Base_Tiempos_Inicializar(999U,1U)==BASE_TIEMPOS_ERROR_PRESCALER); return true;
}
static bool ParadaIdempotente(void)
{
    Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,1U)==BASE_TIEMPOS_ACTIVA);
    Modelo_AvanzarPclk(175000U); CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA);
    const size_t n=modelo_cantidad; CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_DETENIDA && modelo_cantidad==n);
    CHECK(Lectura(BASE_TIEMPOS_DETENIDA,7U));
    CHECK(Base_Tiempos_Inicializar(0U,1U)==BASE_TIEMPOS_ERROR_RELOJ);
    CHECK(Base_Tiempos_Detener()==BASE_TIEMPOS_ERROR_RELOJ); return true;
}
static bool OraculoTiempoAbsoluto(void)
{
    const uint32_t bases[]={1U,2U,3U,5U};
    const uint64_t pasos[]={0U,1U,19U,20U,UINT64_C(0x80000000),UINT64_C(0x100000007),7U,UINT64_C(0x800000000)};
    uint64_t vueltas=0U;
    for(size_t b=0U;b<4U;++b) {
        Modelo_Limpiar(); CHECK(Base_Tiempos_Inicializar(25000000U,bases[b])==BASE_TIEMPOS_ACTIVA);
        uint64_t absoluto=0U;
        for(unsigned i=0U;i<512U;++i) {
            const uint64_t paso=pasos[i%8U]; absoluto+=paso;
            Modelo_AvanzarPclk(paso*25000U);
            const uint64_t esperado=absoluto/bases[b]*bases[b];
            CHECK(Lectura(BASE_TIEMPOS_ACTIVA,(uint32_t)esperado));
        }
        vueltas+=absoluto/bases[b]/UINT64_C(0x100000000);
    }
    printf("PASS oraculo temporal 2048 muestras, %llu vueltas TC\n",(unsigned long long)vueltas);
    return true;
}
int main(void)
{
    const struct { const char *nombre; bool (*prueba)(void); } casos[]={
#define CASO(f) {#f,f}
        CASO(SinInicializar),CASO(ConfiguracionYProteccion),CASO(Bases),CASO(PrescalerExtremos),
        CASO(RelojYPeriodoInvalidos),CASO(PrescalerInvalido),CASO(CuentaSinConsultas),
        CASO(ConsultasNoAlteran),CASO(DetenerCongela),CASO(ReinicioYRecuperacion),
        CASO(NullYProveedor),CASO(RolloverTC),CASO(RolloverMS),CASO(PrioridadYFueraRango),
        CASO(ParadaIdempotente),CASO(OraculoTiempoAbsoluto)
    };
    for(size_t i=0U;i<sizeof(casos)/sizeof(casos[0]);++i) {
        if(!casos[i].prueba()) { fprintf(stderr,"FAIL %s\n",casos[i].nombre); return 1; }
        printf("PASS %s\n",casos[i].nombre);
    }
    return 0;
}
