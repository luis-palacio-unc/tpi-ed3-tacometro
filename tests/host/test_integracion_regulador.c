#include "Procesamiento.h"
#include "Driver_Motor.h"
#include <stddef.h>
#include <stdio.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d %s\n",__FILE__,__LINE__,#x); return false; } } while (0)
typedef struct { uint16_t ultimo; unsigned llamadas; bool acepta; } Motor;
static bool Escribir(void *contexto,uint16_t mando)
{
    Motor *m=contexto;
    m->ultimo=mando; ++m->llamadas;
    return m->acepta;
}
static Driver_Motor_Estado Emitir(Procesamiento_Contexto *app,Procesamiento_Regulador *pi,
    const Procesamiento_Entradas *entradas,const Medicion_Velocidad_Lectura *medicion,
    const Driver_Motor_Salida *motor)
{
    Procesamiento_Salida seleccion;
    if (!Procesamiento_Actualizar(app,entradas,&seleccion)) {
        (void)Procesamiento_ReiniciarRegulador(pi);
        return Driver_Motor_SolicitarCero(motor);
    }
    if (seleccion.fuente==PROCESAMIENTO_FUENTE_MANUAL) {
        (void)Procesamiento_ReiniciarRegulador(pi);
        return Driver_Motor_Aplicar(motor,seleccion.mando_manual_por_mil);
    }
    if (seleccion.fuente==PROCESAMIENTO_FUENTE_CERO) {
        (void)Procesamiento_ReiniciarRegulador(pi);
        return Driver_Motor_SolicitarCero(motor);
    }
    uint16_t mando;
    const Procesamiento_EstadoRegulador estado=Procesamiento_Regular(pi,seleccion.consigna_rpm,medicion,&mando);
    if (estado==PROCESAMIENTO_REGULADOR_VALIDO || estado==PROCESAMIENTO_REGULADOR_RETENIENDO_MEDICION) {
        return Driver_Motor_Aplicar(motor,mando);
    }
    return Driver_Motor_SolicitarCero(motor);
}

static bool Transiciones(void)
{
    Procesamiento_Contexto app;
    Procesamiento_Regulador pi;
    const Procesamiento_ConfigRegulador cfg={4U,1000U,1U,1000U,4U};
    CHECK(Procesamiento_Inicializar(&app,PROCESAMIENTO_MODO_MANUAL,0U));
    CHECK(Procesamiento_InicializarRegulador(&pi,&cfg));
    Motor m={0U,0U,true};
    const Driver_Motor_Salida salida={Escribir,&m};
    Medicion_Velocidad_Lectura medida={0U,MEDICION_VELOCIDAD_VALIDA};
    Procesamiento_Entradas in={PROCESAMIENTO_MODO_MANUAL,900U,1U,NULL};
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==900U);
    const Procesamiento_Consigna vieja={185U,1U,true};
    in=(Procesamiento_Entradas){PROCESAMIENTO_MODO_AUTOMATICO,UINT16_MAX,2U,&vieja};
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==0U && !pi.tiene_medicion);
    const Procesamiento_Consigna nueva={100U,3U,true};
    in.orden_actual=3U;in.consigna=&nueva;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==400U);
    in.orden_actual=4U;in.consigna=NULL;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==500U);
    const Procesamiento_Consigna fuera={186U,5U,true};
    in.orden_actual=5U;in.consigna=&fuera;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && app.consigna_rpm==100U && m.ultimo==600U);
    in=(Procesamiento_Entradas){PROCESAMIENTO_MODO_MANUAL,250U,6U,NULL};
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==250U && pi.integral_num==0 && !pi.tiene_medicion);
    in=(Procesamiento_Entradas){PROCESAMIENTO_MODO_AUTOMATICO,0U,7U,&nueva};
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==0U);
    in.orden_actual=8U;in.consigna=NULL;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==0U);
    const Procesamiento_Consigna renovada={100U,9U,true};
    in.orden_actual=9U;in.consigna=&renovada;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==400U);
    const Procesamiento_Consigna cero={0U,10U,true};
    in.orden_actual=10U;in.consigna=&cero;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==0U && !pi.tiene_medicion);
    const Procesamiento_Consigna volver={100U,11U,true};
    in.orden_actual=11U;in.consigna=&volver;medida.estado=MEDICION_VELOCIDAD_SIN_DATOS;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==0U);
    in.orden_actual=12U;in.consigna=NULL;medida.estado=MEDICION_VELOCIDAD_VALIDA;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==400U);
    m.acepta=false;in.orden_actual=13U;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ERROR_SALIDA && m.ultimo==500U);
    m.acepta=true;in.orden_actual=14U;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==600U);
    CHECK(m.llamadas==14U);
    in.orden_actual=13U;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==0U && !pi.tiene_medicion);
    CHECK(m.llamadas==15U);
    return true;
}

static bool RetencionCompuesta(void)
{
    Procesamiento_Contexto app;
    Procesamiento_Regulador pi;
    const Procesamiento_ConfigRegulador cfg={4U,1000U,1U,1000U,1U};
    CHECK(Procesamiento_Inicializar(&app,PROCESAMIENTO_MODO_AUTOMATICO,0U));
    CHECK(Procesamiento_InicializarRegulador(&pi,&cfg));
    Motor m={0U,0U,true};
    const Driver_Motor_Salida salida={Escribir,&m};
    const Procesamiento_Consigna c={100U,1U,true};
    Procesamiento_Entradas in={PROCESAMIENTO_MODO_AUTOMATICO,0U,1U,&c};
    Medicion_Velocidad_Lectura medida={0U,MEDICION_VELOCIDAD_VALIDA};
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==400U);
    in.consigna=NULL;in.orden_actual=2U;medida.estado=MEDICION_VELOCIDAD_SIN_DATOS;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==500U);
    in.orden_actual=3U;
    CHECK(Emitir(&app,&pi,&in,&medida,&salida)==DRIVER_MOTOR_ACEPTADA && m.ultimo==0U);
    CHECK(m.llamadas==3U && !pi.tiene_medicion && pi.integral_num==0);
    return true;
}
int main(void)
{
    if (!Transiciones()) { return 1; }
    puts("PASS S7 transiciones/B01/retorno proveedor");
    if (!RetencionCompuesta()) { return 1; }
    puts("PASS S7 retencion/cero/B08");
    puts("2/2 casos correctos");
    return 0;
}
