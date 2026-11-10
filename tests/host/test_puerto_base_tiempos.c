#include "Base_Tiempos_Puerto.h"
#include "LPC17xx.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
SysTick_Type cmsis_systick;
SCB_Type cmsis_scb;
static uint32_t mascara;
static char eventos[32];
static size_t cantidad;
static int ultima_irq;
#define CHECK(e) do { if(!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e); return false; } } while(0)
static void Evento(char e) { if(cantidad<sizeof(eventos)) { eventos[cantidad++]=e; } }
uint32_t __get_PRIMASK(void) { Evento('L'); return mascara; }
void __disable_irq(void) { Evento('D'); mascara=1U; }
void __set_PRIMASK(uint32_t valor) { Evento('R'); mascara=valor; }
void __DSB(void) { Evento('B'); }
void __ISB(void) { Evento('I'); }
void NVIC_SetPriority(int irq,uint32_t prioridad)
{
    ultima_irq=irq;
    if(irq==SysTick_IRQn) { SCB->SHP[11]=(uint8_t)(prioridad << 3U); }
    Evento('P');
}
static bool Criticas(void)
{
    for(uint32_t inicial=0U;inicial<=1U;++inicial) {
        cantidad=0U; mascara=inicial;
        const uint32_t anterior=Base_Tiempos_Puerto_EntrarCritica();
        CHECK(anterior==inicial && mascara==1U);
        Base_Tiempos_Puerto_SalirCritica(anterior);
        CHECK(mascara==inicial && cantidad==7U && memcmp(eventos,"LDBIBRI",7U)==0);
    }
    return true;
}
static bool MapaRegistros(void)
{
    SysTick->CTRL=UINT32_MAX; SysTick->LOAD=88U; SysTick->VAL=77U; SysTick->CALIB=66U;
    Base_Tiempos_Puerto_Escribir(BASE_TIEMPOS_REG_CONTROL,0U);
    CHECK(SysTick->CTRL==0U && SysTick->LOAD==88U && SysTick->VAL==77U);
    Base_Tiempos_Puerto_Escribir(BASE_TIEMPOS_REG_RECARGA,119999U);
    CHECK(SysTick->LOAD==119999U && SysTick->VAL==77U);
    Base_Tiempos_Puerto_Escribir(BASE_TIEMPOS_REG_ACTUAL,0U);
    CHECK(SysTick->VAL==0U && SysTick->CALIB==66U);
    Base_Tiempos_Puerto_Escribir(BASE_TIEMPOS_REG_CONTROL,7U);
    CHECK(SysTick->CTRL==7U && SysTick->LOAD==119999U && SysTick->CALIB==66U);
    return true;
}
static bool IcsrYPrioridad(void)
{
    SCB->ICSR=UINT32_C(0x14000000); SCB->ajeno=UINT32_C(0xCAFEBABE);
    for(unsigned i=0U;i<12U;++i) { SCB->SHP[i]=(uint8_t)(i+1U); }
    Base_Tiempos_Puerto_Escribir(BASE_TIEMPOS_REG_PENDIENTE,UINT32_C(0x02000000));
    CHECK(SCB->ICSR==UINT32_C(0x02000000)); /* Detecta read-modify-write indebido. */
    Base_Tiempos_Puerto_Escribir(BASE_TIEMPOS_REG_PRIORIDAD,31U);
    CHECK(ultima_irq==SysTick_IRQn && SCB->SHP[11]==248U && SCB->ajeno==UINT32_C(0xCAFEBABE));
    for(unsigned i=0U;i<11U;++i) { CHECK(SCB->SHP[i]==i+1U); }
    return true;
}
static bool RegistroDesconocido(void)
{
    const uint32_t control=SysTick->CTRL,load=SysTick->LOAD,val=SysTick->VAL,icsr=SCB->ICSR;
    const size_t n=cantidad;
    Base_Tiempos_Puerto_Escribir((Base_Tiempos_Registro)99,UINT32_MAX);
    CHECK(control==SysTick->CTRL && load==SysTick->LOAD && val==SysTick->VAL && icsr==SCB->ICSR && n==cantidad);
    return true;
}
int main(void)
{
    if(!Criticas() || !MapaRegistros() || !IcsrYPrioridad() || !RegistroDesconocido()) { return 1; }
    puts("PASS 4 casos puerto LPC1769 sobre dobles CMSIS; no MCU"); return 0;
}
