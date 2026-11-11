/*
 * Pruebas host de Dispositivos/Timer.c (B11 + B18/I1).
 * Incluye el .c bajo prueba para alcanzar su estado interno y reemplaza por dobles
 * las funciones de la biblioteca original, del NVIC y SensorOptico_Read.
 * Modela TC, MR0 y el flag MR0 de T0IR; no ejecuta perifericos reales.
 */
#include "dobles_cmsis.h"
#include "lpc17xx_timer.h"
#include <stddef.h>
#include <stdio.h>

#include "../../Dispositivos/Timer.c"

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)
#define EVENTOS_MAX 16U

typedef enum {
    EV_INIT_TIMER, EV_CONFIG_MATCH, EV_SENSOR, EV_ENABLE, EV_ESTADO, EV_LIMPIAR, EV_MATCH
} Evento;

static uint32_t tc, mr0, sensor;
static bool flag_mr0, en_marcha, error_modelo, nvic_habilitada_en_config;
static unsigned lecturas_tc, lecturas_sensor, limpiezas, actualizaciones, arranques;
static TIM_TIMERCFG_T base_vista;
static TIM_MATCHCFG_T match_visto;
static Evento eventos[EVENTOS_MAX];
static unsigned eventos_total;

static void Registrar(Evento evento)
{
    if (eventos_total < EVENTOS_MAX) { eventos[eventos_total] = evento; }
    ++eventos_total;
}

uint32_t SensorOptico_Read(void)
{
    ++lecturas_sensor;
    Registrar(EV_SENSOR);
    return sensor;
}

void TIM_InitTimer(LPC_TIM_TypeDef *timer, const TIM_TIMERCFG_T *cfg)
{
    if (timer != LPC_TIM0 || cfg == NULL) { error_modelo = true; return; }
    nvic_habilitada_en_config = nvic.habilitada[TIMER0_IRQn];
    base_vista = *cfg;
    en_marcha = false; tc = 0U; flag_mr0 = false;
    Registrar(EV_INIT_TIMER);
}

void TIM_ConfigMatch(LPC_TIM_TypeDef *timer, const TIM_MATCHCFG_T *cfg)
{
    if (timer != LPC_TIM0 || cfg == NULL) { error_modelo = true; return; }
    nvic_habilitada_en_config = nvic_habilitada_en_config || nvic.habilitada[TIMER0_IRQn];
    match_visto = *cfg;
    mr0 = cfg->matchValue;
    Registrar(EV_CONFIG_MATCH);
}

void TIM_Enable(LPC_TIM_TypeDef *timer)
{
    if (timer != LPC_TIM0) { error_modelo = true; return; }
    en_marcha = true; ++arranques;
    Registrar(EV_ENABLE);
}

uint32_t TIM_ReadTimer(LPC_TIM_TypeDef *timer)
{
    if (timer != LPC_TIM0) { error_modelo = true; return 0U; }
    ++lecturas_tc;
    return tc;
}

FlagStatus TIM_GetIntStatus(LPC_TIM_TypeDef *timer, TIM_INT fuente)
{
    if (timer != LPC_TIM0 || fuente != TIM_MR0_INT) { error_modelo = true; return RESET; }
    Registrar(EV_ESTADO);
    return flag_mr0 ? SET : RESET;
}

void TIM_ClearIntPending(LPC_TIM_TypeDef *timer, TIM_INT fuente)
{
    if (timer != LPC_TIM0 || fuente != TIM_MR0_INT) { error_modelo = true; return; }
    flag_mr0 = false; ++limpiezas;
    Registrar(EV_LIMPIAR);
}

void TIM_UpdateMatchValue(LPC_TIM_TypeDef *timer, TIM_MATCH_CH canal, uint32_t valor)
{
    if (timer != LPC_TIM0 || canal != TIM_MATCH_0) { error_modelo = true; return; }
    mr0 = valor; ++actualizaciones;
    Registrar(EV_MATCH);
}

static void Preparar(void)
{
    NvicReiniciar();
    tc = UINT32_C(0xA5A5A5A5); mr0 = UINT32_C(0x5A5A5A5A); sensor = 0U;
    flag_mr0 = false; en_marcha = false; error_modelo = false; nvic_habilitada_en_config = true;
    lecturas_tc = 0U; lecturas_sensor = 0U; limpiezas = 0U; actualizaciones = 0U; arranques = 0U;
    memset(&base_vista, 0xFF, sizeof(base_vista)); memset(&match_visto, 0xFF, sizeof(match_visto));
    eventos_total = 0U;
    Timer_Init();
}

static void CerrarVentana(uint32_t conteo_sensor)
{
    sensor = conteo_sensor;
    tc = mr0;
    flag_mr0 = true;
    TIMER0_IRQHandler();
}

static bool InicializacionAcordada(void)
{
    Preparar();
    const Evento esperado[] = {EV_INIT_TIMER, EV_CONFIG_MATCH};
    uint32_t pulsos = UINT32_C(0xDEADBEEF);
    CHECK(TPI_CONTROL_PERIODO_MS == 500U && TIMER_BASE_MS == 1U);
    CHECK(eventos_total == 2U && memcmp(eventos, esperado, sizeof(esperado)) == 0);
    CHECK(base_vista.prescaleOpt == TIM_US && base_vista.prescaleValue == 1000U);
    CHECK(match_visto.channel == TIM_MATCH_0 && match_visto.matchValue == 500U);
    CHECK(match_visto.intEn == ENABLE && match_visto.resetEn == DISABLE);
    CHECK(match_visto.stopEn == DISABLE && match_visto.extOpt == TIM_NOTHING);
    CHECK(!nvic_habilitada_en_config && nvic.deshabilitaciones == 1U);
    CHECK(nvic.habilitada[TIMER0_IRQn] && nvic.prioridad[TIMER0_IRQn] == 1U);
    CHECK(nvic.habilitaciones == 1U && nvic.prioridades == 1U && nvic.pedidos == 0U);
    for (unsigned i = 0U; i < NVIC_MODELO_IRQS; ++i) {
        CHECK(i == (unsigned)TIMER0_IRQn || (!nvic.habilitada[i] && nvic.prioridad[i] == UINT32_MAX));
    }
    CHECK(!en_marcha && arranques == 0U && lecturas_sensor == 0U);
    CHECK(!Timer_ConsumirVentana(&pulsos) && pulsos == UINT32_C(0xDEADBEEF));
    CHECK(!nvic.error && !error_modelo);
    return true;
}

static bool ArranqueTomaReferenciaDelSensor(void)
{
    Preparar();
    const Evento esperado[] = {EV_INIT_TIMER, EV_CONFIG_MATCH, EV_SENSOR, EV_ENABLE};
    uint32_t pulsos = 0U;
    sensor = 12345U;
    Timer_Start();
    CHECK(eventos_total == 4U && memcmp(eventos, esperado, sizeof(esperado)) == 0);
    CHECK(en_marcha && arranques == 1U && lecturas_sensor == 1U && mr0 == 500U);
    CerrarVentana(12345U + 60U);
    CHECK(Timer_ConsumirVentana(&pulsos) && pulsos == 60U && !error_modelo);
    return true;
}

static bool LecturaEnMilisegundosConCuentaLibre(void)
{
    Preparar();
    Timer_Start();
    const uint32_t marcas[] = {0U, 1U, 499U, 500U, 501U, 123456U, UINT32_MAX};
    for (size_t i = 0U; i < sizeof(marcas) / sizeof(marcas[0]); ++i) {
        tc = marcas[i];
        CHECK(Timer_Read() == marcas[i] && tc == marcas[i]);
    }
    CHECK(lecturas_tc == 7U && limpiezas == 0U && actualizaciones == 0U && mr0 == 500U);
    CerrarVentana(0U);
    CHECK(tc == 500U && Timer_Read() == 500U && mr0 == 1000U && !error_modelo);
    return true;
}

static bool VentanaSeConsumeUnaSolaVez(void)
{
    Preparar();
    sensor = 1000U;
    Timer_Start();
    uint32_t pulsos = UINT32_C(0xDEADBEEF);
    CHECK(!Timer_ConsumirVentana(&pulsos) && pulsos == UINT32_C(0xDEADBEEF));
    eventos_total = 0U;
    CerrarVentana(1060U);
    const Evento esperado[] = {EV_ESTADO, EV_SENSOR, EV_LIMPIAR, EV_MATCH};
    CHECK(eventos_total == 4U && memcmp(eventos, esperado, sizeof(esperado)) == 0);
    CHECK(!flag_mr0 && limpiezas == 1U && actualizaciones == 1U && mr0 == 1000U);
    CHECK(Timer_ConsumirVentana(&pulsos) && pulsos == 60U);
    pulsos = UINT32_C(0xDEADBEEF);
    CHECK(!Timer_ConsumirVentana(&pulsos) && pulsos == UINT32_C(0xDEADBEEF));
    CerrarVentana(1060U);
    CHECK(Timer_ConsumirVentana(&pulsos) && pulsos == 0U);
    CHECK(!Timer_ConsumirVentana(&pulsos) && mr0 == 1500U && !error_modelo);
    return true;
}

static bool MatchAvanzaDeAQuinientosYDaLaVuelta(void)
{
    Preparar();
    Timer_Start();
    for (uint32_t ventana = 1U; ventana <= 8U; ++ventana) {
        CHECK(mr0 == 500U * ventana);
        CerrarVentana(ventana);
    }
    CHECK(actualizaciones == 8U && limpiezas == 8U && mr0 == 4500U);
    Timer_ProximoMatch = UINT32_MAX - 299U; mr0 = UINT32_MAX - 299U;
    CerrarVentana(9U);
    CHECK(mr0 == 200U && Timer_ProximoMatch == 200U);
    CerrarVentana(10U);
    CHECK(mr0 == 700U && !error_modelo);
    return true;
}

static bool ConteoDeSensorDaLaVuelta(void)
{
    Preparar();
    sensor = UINT32_MAX - 29U;
    Timer_Start();
    uint32_t pulsos = 0U;
    CerrarVentana(30U);
    CHECK(Timer_ConsumirVentana(&pulsos) && pulsos == 60U);
    CerrarVentana(30U + 185U);
    CHECK(Timer_ConsumirVentana(&pulsos) && pulsos == 185U);
    CerrarVentana(29U + 185U);
    CHECK(Timer_ConsumirVentana(&pulsos) && pulsos == UINT32_MAX && !error_modelo);
    return true;
}

static bool VentanaPerdidaConservaLaMasReciente(void)
{
    Preparar();
    sensor = 100U;
    Timer_Start();
    uint32_t pulsos = 0U;
    CerrarVentana(160U);
    CerrarVentana(185U);
    CHECK(Timer_ConsumirVentana(&pulsos) && pulsos == 25U);
    pulsos = UINT32_C(0xDEADBEEF);
    CHECK(!Timer_ConsumirVentana(&pulsos) && pulsos == UINT32_C(0xDEADBEEF));
    CerrarVentana(190U); CerrarVentana(200U); CerrarVentana(207U);
    CHECK(Timer_ConsumirVentana(&pulsos) && pulsos == 7U);
    CHECK(!Timer_ConsumirVentana(&pulsos) && mr0 == 3000U && !error_modelo);
    return true;
}

static bool IsrSinFlagNoPublica(void)
{
    Preparar();
    sensor = 40U;
    Timer_Start();
    uint32_t pulsos = UINT32_C(0xDEADBEEF);
    sensor = 99U;
    TIMER0_IRQHandler();
    CHECK(lecturas_sensor == 1U && limpiezas == 0U && actualizaciones == 0U && mr0 == 500U);
    CHECK(!Timer_ConsumirVentana(&pulsos) && pulsos == UINT32_C(0xDEADBEEF));
    CerrarVentana(100U);
    CHECK(Timer_ConsumirVentana(&pulsos) && pulsos == 60U && !error_modelo);
    return true;
}

static bool DestinoNuloNoConsume(void)
{
    Preparar();
    Timer_Start();
    uint32_t pulsos = 0U;
    CerrarVentana(77U);
    CHECK(!Timer_ConsumirVentana(NULL) && !Timer_ConsumirVentana(NULL));
    CHECK(Timer_ConsumirVentana(&pulsos) && pulsos == 77U);
    CHECK(!Timer_ConsumirVentana(&pulsos) && !error_modelo);
    return true;
}

static bool ContadorDeVentanasDaLaVuelta(void)
{
    Preparar();
    Timer_Start();
    uint32_t pulsos = 0U;
    Timer_VentanasCerradas = UINT32_MAX; Timer_VentanasConsumidas = UINT32_MAX;
    CHECK(!Timer_ConsumirVentana(&pulsos));
    CerrarVentana(5U);
    CHECK(Timer_VentanasCerradas == 0U);
    CHECK(Timer_ConsumirVentana(&pulsos) && pulsos == 5U && !Timer_ConsumirVentana(&pulsos));
    CHECK(!error_modelo);
    return true;
}

static bool ReinicializarAbreEpocaNueva(void)
{
    Preparar();
    sensor = 10U;
    Timer_Start();
    uint32_t pulsos = UINT32_C(0xDEADBEEF);
    CerrarVentana(70U);
    Timer_Init();
    CHECK(mr0 == 500U && !en_marcha && nvic.habilitada[TIMER0_IRQn]);
    CHECK(!Timer_ConsumirVentana(&pulsos) && pulsos == UINT32_C(0xDEADBEEF));
    sensor = 500U;
    Timer_Start();
    CerrarVentana(512U);
    CHECK(Timer_ConsumirVentana(&pulsos) && pulsos == 12U && mr0 == 1000U && !error_modelo);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"I1 InicializacionAcordada", InicializacionAcordada},
        {"I1 ArranqueTomaReferenciaDelSensor", ArranqueTomaReferenciaDelSensor},
        {"WT2 LecturaEnMilisegundosConCuentaLibre", LecturaEnMilisegundosConCuentaLibre},
        {"I1 VentanaSeConsumeUnaSolaVez", VentanaSeConsumeUnaSolaVez},
        {"I1 MatchAvanzaDeAQuinientosYDaLaVuelta", MatchAvanzaDeAQuinientosYDaLaVuelta},
        {"I1 ConteoDeSensorDaLaVuelta", ConteoDeSensorDaLaVuelta},
        {"I1 VentanaPerdidaConservaLaMasReciente", VentanaPerdidaConservaLaMasReciente},
        {"I1 IsrSinFlagNoPublica", IsrSinFlagNoPublica},
        {"I1 DestinoNuloNoConsume", DestinoNuloNoConsume},
        {"I1 ContadorDeVentanasDaLaVuelta", ContadorDeVentanasDaLaVuelta},
        {"I1 ReinicializarAbreEpocaNueva", ReinicializarAbreEpocaNueva}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
