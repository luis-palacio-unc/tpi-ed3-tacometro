#include "Selector.h"
#include "LED.h"
#include "Buzzer.h"
#include "Driver_Alarmas.h"
#include "lpc17xx_gpio.h"
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

static void Preparar(void)
{
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

static bool SalidaAislada(Driver_Alarmas_EscribirSalida escribir,unsigned port,
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
            CHECK(escribir(NULL,activar));
            CHECK(memcmp(esperado,puertos,sizeof(esperado))==0);
            CHECK(lecturas==0U && escrituras==1U && !error_modelo);
        }
    }
    return true;
}

static bool LedActivoBajo(void) { return SalidaAislada(LED_Write,0U,LED_MASK,false); }
static bool BuzzerActivoAlto(void) { return SalidaAislada(Buzzer_Write,2U,BUZZER_MASK,true); }

static bool ContextosIgnorados(void)
{
    Preparar(); int contexto_led=123,contexto_buzzer=456;
    CHECK(LED_Write(&contexto_led,true) && Buzzer_Write(&contexto_buzzer,true));
    CHECK(LED_Write(&contexto_led,false) && Buzzer_Write(&contexto_buzzer,false));
    CHECK(contexto_led==123 && contexto_buzzer==456);
    CHECK((puertos[0]&LED_MASK)!=0U && (puertos[2]&BUZZER_MASK)==0U);
    CHECK(escrituras==4U && !error_modelo);
    return true;
}

static bool RepeticionRestauraSalida(void)
{
    Preparar();
    CHECK(LED_Write(NULL,true) && Buzzer_Write(NULL,true));
    puertos[0]|=LED_MASK; puertos[2]&=~BUZZER_MASK;
    CHECK(LED_Write(NULL,true) && Buzzer_Write(NULL,true));
    CHECK((puertos[0]&LED_MASK)==0U && (puertos[2]&BUZZER_MASK)!=0U);
    CHECK(escrituras==4U && !error_modelo);
    return true;
}

static bool ComposicionAlarmas(void)
{
    Preparar();
    int contexto_led=11,contexto_buzzer=22;
    const Driver_Alarmas_Salidas salidas={LED_Write,&contexto_led,Buzzer_Write,&contexto_buzzer};
    const bool pedidos[]={true,false,false,true};
    puertos[2]|=SELECTOR_MASK;
    uint32_t esperado[5]; memcpy(esperado,puertos,sizeof(esperado));
    for(unsigned i=0U;i<4U;++i) {
        const bool activar=pedidos[i];
        if(activar) { esperado[0]&=~LED_MASK; esperado[2]|=BUZZER_MASK; }
        else { esperado[0]|=LED_MASK; esperado[2]&=~BUZZER_MASK; }
        CHECK(Driver_Alarmas_Aplicar(&salidas,activar)==DRIVER_ALARMAS_ACEPTADA);
        CHECK(memcmp(esperado,puertos,sizeof(esperado))==0);
        CHECK(Selector_Read());
    }
    CHECK(contexto_led==11 && contexto_buzzer==22 && escrituras==8U && !error_modelo);
    return true;
}

static bool SalidasIntercaladas(void)
{
    Preparar();
    for(unsigned valor=0U;valor<2U;++valor) {
        if(valor==0U) { puertos[2]&=~SELECTOR_MASK; } else { puertos[2]|=SELECTOR_MASK; }
        const bool selector=Selector_Read();
        CHECK(LED_Write(NULL,true) && Buzzer_Write(NULL,false));
        CHECK((puertos[0]&LED_MASK)==0U && (puertos[2]&BUZZER_MASK)==0U);
        CHECK(LED_Write(NULL,false) && Buzzer_Write(NULL,true));
        CHECK((puertos[0]&LED_MASK)!=0U && (puertos[2]&BUZZER_MASK)!=0U);
        CHECK(Selector_Read()==selector);
    }
    CHECK(!error_modelo);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[]={
        {"SelectorNiveles",SelectorNiveles},{"SelectorIgnoraOtrosBits",SelectorIgnoraOtrosBits},
        {"SelectorIgnoraOtrosPuertos",SelectorIgnoraOtrosPuertos},{"LedActivoBajo",LedActivoBajo},
        {"BuzzerActivoAlto",BuzzerActivoAlto},{"ContextosIgnorados",ContextosIgnorados},
        {"RepeticionRestauraSalida",RepeticionRestauraSalida},{"ComposicionAlarmas",ComposicionAlarmas},
        {"SalidasIntercaladas",SalidasIntercaladas}};
    for(unsigned i=0U;i<sizeof(casos)/sizeof(casos[0]);++i) {
        if(!casos[i].ejecutar()) { fprintf(stderr,"FAIL %s\n",casos[i].nombre); return 1; }
    }
    puts("PASS B12: 9 casos; GPIO modelado por API, sin ejecutar perifericos.");
    return 0;
}
