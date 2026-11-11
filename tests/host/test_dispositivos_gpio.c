#include "Driver_Alarmas.h"
#include "Selector.h"
#include "LED.h"
#include "Buzzer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>
#include <string.h>

#define CHECK(e) do { if(!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); return false; } } while(0)
#define LED_MASK UINT32_C(0x00400000)
#define SELECTOR_MASK UINT32_C(0x00000040)
#define BUZZER_MASK UINT32_C(0x00000080)
static uint32_t puertos[5];
static unsigned lecturas,escrituras;
static bool error_modelo;

uint32_t GPIO_ReadValue(LPC_PORT port)
{
    if(port<PORT_0 || port>PORT_4) { error_modelo=true; return 0U; }
    ++lecturas;
    return puertos[(unsigned)port];
}

void GPIO_SetPinState(LPC_PORT port,LPC_PIN pin,SetState nivel)
{
    if(port<PORT_0 || port>PORT_4 || pin<PIN_0 || pin>PIN_31 ||
       (nivel!=SET && nivel!=RESET)) { error_modelo=true; return; }
    ++escrituras;
    const uint32_t mascara=UINT32_C(1)<<(unsigned)pin;
    if(nivel==SET) { puertos[(unsigned)port]|=mascara; }
    else { puertos[(unsigned)port]&=~mascara; }
}

static uint32_t dir_salida[5], niveles_al_dirigir[5];
static unsigned direcciones, configuraciones;
static PINSEL_CFG_T pin_visto;

void PINSEL_ConfigPin(const PINSEL_CFG_T *cfg)
{
    if(cfg==NULL) { error_modelo=true; return; }
    if(direcciones!=0U || escrituras!=0U) { error_modelo=true; }
    pin_visto=*cfg; ++configuraciones;
}

void GPIO_SetDir(LPC_PORT port,uint32_t mascara,GPIO_DIR dir)
{
    if(port<PORT_0 || port>PORT_4 || (dir!=GPIO_INPUT && dir!=GPIO_OUTPUT)) { error_modelo=true; return; }
    ++direcciones;
    niveles_al_dirigir[(unsigned)port]=puertos[(unsigned)port];
    if(dir==GPIO_OUTPUT) { dir_salida[(unsigned)port]|=mascara; }
    else { dir_salida[(unsigned)port]&=~mascara; }
}

static void Preparar(void)
{
    memset(dir_salida,0,sizeof(dir_salida)); memset(niveles_al_dirigir,0,sizeof(niveles_al_dirigir));
    direcciones=0U; configuraciones=0U; memset(&pin_visto,0xFF,sizeof(pin_visto));
    const uint32_t iniciales[]={UINT32_C(0x12345678),UINT32_C(0x87654321),
        UINT32_C(0xA5A5A5A5),UINT32_C(0x5A5A5A5A),UINT32_C(0xC3C3C3C3)};
    memcpy(puertos,iniciales,sizeof(puertos));
    lecturas=0U; escrituras=0U; error_modelo=false;
}

static bool SelectorNiveles(void)
{
    Preparar(); puertos[2]=0U;
    CHECK(!Selector_Read());
    puertos[2]=SELECTOR_MASK;
    CHECK(Selector_Read());
    CHECK(lecturas==2U && escrituras==0U && !error_modelo);
    return true;
}

static bool SelectorIgnoraOtrosBits(void)
{
    Preparar();
    for(unsigned bit=0U;bit<32U;++bit) {
        const uint32_t valor=UINT32_C(1)<<bit;
        puertos[2]=valor;
        CHECK(Selector_Read()==(bit==6U));
        CHECK(puertos[2]==valor);
        puertos[2]=~valor;
        CHECK(Selector_Read()==(bit!=6U));
        CHECK(puertos[2]==~valor);
    }
    CHECK(lecturas==64U && escrituras==0U && !error_modelo);
    return true;
}

static bool SelectorIgnoraOtrosPuertos(void)
{
    Preparar(); puertos[2]=0U;
    for(unsigned port=0U;port<5U;++port) { if(port!=2U) { puertos[port]=UINT32_MAX; } }
    uint32_t antes[5]; memcpy(antes,puertos,sizeof(antes));
    for(unsigned i=0U;i<3U;++i) { CHECK(!Selector_Read()); }
    CHECK(memcmp(antes,puertos,sizeof(antes))==0 && escrituras==0U && !error_modelo);
    return true;
}

static bool SalidaAislada(void (*escribir)(bool),unsigned port,
                         uint32_t mascara,bool activo_alto)
{
    const uint32_t patrones[]={0U,UINT32_MAX,UINT32_C(0xAAAAAAAA),UINT32_C(0x55555555)};
    for(unsigned p=0U;p<4U;++p) {
        for(unsigned valor=0U;valor<2U;++valor) {
            Preparar(); puertos[port]=patrones[p];
            uint32_t esperado[5]; memcpy(esperado,puertos,sizeof(esperado));
            const bool activar=valor!=0U;
            if(activar==activo_alto) { esperado[port]|=mascara; }
            else { esperado[port]&=~mascara; }
            escribir(activar);
            CHECK(memcmp(esperado,puertos,sizeof(esperado))==0);
            CHECK(lecturas==0U && escrituras==1U && !error_modelo);
        }
    }
    return true;
}

static bool LedActivoBajo(void) { return SalidaAislada(LED_Write,0U,LED_MASK,false); }
static bool BuzzerActivoAlto(void) { return SalidaAislada(Buzzer_Write,2U,BUZZER_MASK,true); }

static bool RepeticionRestauraSalida(void)
{
    Preparar();
    LED_Write(true); Buzzer_Write(true);
    puertos[0]|=LED_MASK; puertos[2]&=~BUZZER_MASK;
    LED_Write(true); Buzzer_Write(true);
    CHECK((puertos[0]&LED_MASK)==0U && (puertos[2]&BUZZER_MASK)!=0U);
    CHECK(escrituras==4U && !error_modelo);
    return true;
}

static bool SalidasIntercaladas(void)
{
    Preparar();
    for(unsigned valor=0U;valor<2U;++valor) {
        if(valor==0U) { puertos[2]&=~SELECTOR_MASK; } else { puertos[2]|=SELECTOR_MASK; }
        const bool selector=Selector_Read();
        LED_Write(true); Buzzer_Write(false);
        CHECK((puertos[0]&LED_MASK)==0U && (puertos[2]&BUZZER_MASK)==0U);
        LED_Write(false); Buzzer_Write(true);
        CHECK((puertos[0]&LED_MASK)!=0U && (puertos[2]&BUZZER_MASK)!=0U);
        CHECK(Selector_Read()==selector);
    }
    CHECK(!error_modelo);
    return true;
}

static bool ComposicionAlarmasSinAdaptadores(void)
{
    const Driver_Alarmas_Salidas salidas = {LED_Write, Buzzer_Write};
    const bool pedidos[] = {true, false, false, true};
    const uint32_t patrones[] = {0U, UINT32_MAX, UINT32_C(0xAAAAAAAA), UINT32_C(0x55555555)};
    for (size_t p = 0U; p < sizeof(patrones) / sizeof(patrones[0]); ++p) {
        Preparar(); puertos[0] = patrones[p]; puertos[2] = patrones[p];
        uint32_t esperado[5]; memcpy(esperado, puertos, sizeof(esperado));
        const bool selector = (patrones[p] & SELECTOR_MASK) != 0U;
        for (size_t i = 0U; i < sizeof(pedidos) / sizeof(pedidos[0]); ++i) {
            if (pedidos[i]) { esperado[0] &= ~LED_MASK; esperado[2] |= BUZZER_MASK; }
            else { esperado[0] |= LED_MASK; esperado[2] &= ~BUZZER_MASK; }
            CHECK(Driver_Alarmas_Aplicar(&salidas, pedidos[i]) == DRIVER_ALARMAS_ACEPTADA);
            CHECK(memcmp(esperado, puertos, sizeof(esperado)) == 0);
            CHECK(Selector_Read() == selector);
            CHECK(escrituras == 2U * (i + 1U) && !error_modelo);
        }
    }
    return true;
}

static bool SelectorInitEntradaConPullUp(void)
{
    Preparar(); dir_salida[2]=UINT32_MAX;
    uint32_t antes[5]; memcpy(antes,puertos,sizeof(antes));
    Selector_Init();
    CHECK(configuraciones==1U && direcciones==1U && escrituras==0U && lecturas==0U);
    CHECK(pin_visto.port==PORT_2 && pin_visto.pin==PIN_6 && pin_visto.func==PINSEL_FUNC_00);
    CHECK(pin_visto.mode==PINSEL_PULLUP && pin_visto.openDrain==DISABLE);
    CHECK(dir_salida[2]==~SELECTOR_MASK && dir_salida[0]==0U);
    CHECK(memcmp(antes,puertos,sizeof(antes))==0 && !error_modelo);
    return true;
}

static bool SalidaInit(void (*iniciar)(void),LPC_PORT port,LPC_PIN pin,uint32_t mascara,bool nivel_inicial)
{
    const uint32_t patrones[]={0U,UINT32_MAX,UINT32_C(0xAAAAAAAA),UINT32_C(0x55555555)};
    for(unsigned p=0U;p<4U;++p) {
        Preparar(); puertos[(unsigned)port]=patrones[p];
        uint32_t esperado[5]; memcpy(esperado,puertos,sizeof(esperado));
        if(nivel_inicial) { esperado[(unsigned)port]|=mascara; }
        else { esperado[(unsigned)port]&=~mascara; }
        iniciar();
        CHECK(configuraciones==1U && escrituras==1U && direcciones==1U && lecturas==0U);
        CHECK(pin_visto.port==port && pin_visto.pin==pin && pin_visto.func==PINSEL_FUNC_00);
        CHECK(pin_visto.mode==PINSEL_TRISTATE && pin_visto.openDrain==DISABLE);
        CHECK(niveles_al_dirigir[(unsigned)port]==esperado[(unsigned)port]);
        CHECK(memcmp(esperado,puertos,sizeof(esperado))==0);
        for(unsigned otro=0U;otro<5U;++otro) {
            CHECK(dir_salida[otro]==(otro==(unsigned)port?mascara:0U));
        }
        CHECK(!error_modelo);
    }
    return true;
}

static bool LedInitApagado(void) { return SalidaInit(LED_Init,PORT_0,PIN_22,LED_MASK,true); }
static bool BuzzerInitDesactivado(void) { return SalidaInit(Buzzer_Init,PORT_2,PIN_7,BUZZER_MASK,false); }

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[]={
        {"SelectorNiveles",SelectorNiveles},{"SelectorIgnoraOtrosBits",SelectorIgnoraOtrosBits},
        {"SelectorIgnoraOtrosPuertos",SelectorIgnoraOtrosPuertos},{"LedActivoBajo",LedActivoBajo},
        {"BuzzerActivoAlto",BuzzerActivoAlto},
        {"RepeticionRestauraSalida",RepeticionRestauraSalida},
        {"SalidasIntercaladas",SalidasIntercaladas},
        {"S9 ComposicionAlarmasSinAdaptadores",ComposicionAlarmasSinAdaptadores},
        {"I1 SelectorInitEntradaConPullUp",SelectorInitEntradaConPullUp},
        {"I1 LedInitApagado",LedInitApagado},{"I1 BuzzerInitDesactivado",BuzzerInitDesactivado}};
    for(unsigned i=0U;i<sizeof(casos)/sizeof(casos[0]);++i) {
        if(!casos[i].ejecutar()) { fprintf(stderr,"FAIL %s\n",casos[i].nombre); return 1; }
    }
    puts("PASS B12/B07-R1/I1: 11 casos; GPIO modelado por API, sin ejecutar perifericos.");
    return 0;
}
