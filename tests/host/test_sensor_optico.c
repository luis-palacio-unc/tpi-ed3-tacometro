#include "Sensor_Optico.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static uint32_t cuenta_base, cuenta_eventos;
static unsigned consultas_base, consultas_eventos;
static bool error_modelo;

uint32_t TIM_ReadTimer(LPC_TIM_TypeDef *timer)
{
    if (timer == LPC_TIM1) { ++consultas_eventos; return cuenta_eventos; }
    if (timer == LPC_TIM0) { ++consultas_base; return cuenta_base; }
    error_modelo = true;
    return 0U;
}

typedef enum { EV_CONTADOR, EV_PIN, EV_ENABLE } Evento;
static Evento eventos[8];
static size_t eventos_total;
static TIM_COUNTERCFG_T contador_visto;
static PINSEL_CFG_T pin_visto;

static void Registrar(Evento evento)
{
    if (eventos_total < sizeof(eventos) / sizeof(eventos[0])) { eventos[eventos_total] = evento; }
    ++eventos_total;
}

void TIM_InitCounter(LPC_TIM_TypeDef *timer, const TIM_COUNTERCFG_T *cfg)
{
    if (timer != LPC_TIM1 || cfg == NULL) { error_modelo = true; return; }
    contador_visto = *cfg;
    cuenta_eventos = 0U;
    Registrar(EV_CONTADOR);
}

void PINSEL_ConfigPin(const PINSEL_CFG_T *cfg)
{
    if (cfg == NULL) { error_modelo = true; return; }
    pin_visto = *cfg;
    Registrar(EV_PIN);
}

void TIM_Enable(LPC_TIM_TypeDef *timer)
{
    if (timer != LPC_TIM1) { error_modelo = true; return; }
    Registrar(EV_ENABLE);
}

static void Preparar(void)
{
    eventos_total = 0U; pin_visto.port = PORT_4; pin_visto.pin = PIN_31;
    pin_visto.func = PINSEL_FUNC_00; pin_visto.mode = PINSEL_PULLDOWN; pin_visto.openDrain = ENABLE;
    contador_visto.input = TIM_CAPTURE_1; contador_visto.edge = TIM_CTR_ANY;
    cuenta_base = UINT32_C(0xA5A5A5A5);
    cuenta_eventos = 0U;
    consultas_base = 0U; consultas_eventos = 0U; error_modelo = false;
}

static bool LecturasExactasYBaseIndependiente(void)
{
    Preparar();
    const uint32_t valores[] = {0U, 1U, 60U, 185U, 200U, UINT32_MAX};
    for (size_t i = 0U; i < sizeof(valores) / sizeof(valores[0]); ++i) {
        cuenta_eventos = valores[i];
        CHECK(SensorOptico_Read() == valores[i]);
        CHECK(cuenta_eventos == valores[i] && cuenta_base == UINT32_C(0xA5A5A5A5));
    }
    CHECK(consultas_eventos == 6U && consultas_base == 0U && !error_modelo);
    return true;
}

static bool RepeticionYAvance(void)
{
    Preparar();
    const uint32_t valores[] = {10U, 10U, 70U, 70U, 71U};
    for (size_t i = 0U; i < sizeof(valores) / sizeof(valores[0]); ++i) {
        cuenta_eventos = valores[i];
        CHECK(SensorOptico_Read() == valores[i]);
        CHECK(SensorOptico_Read() == valores[i] && cuenta_eventos == valores[i]);
    }
    CHECK(consultas_eventos == 10U && consultas_base == 0U && !error_modelo);
    return true;
}

static bool VentanaUnSegundo(void)
{
    const struct { uint32_t pulsos; } casos[] = {{0U}, {60U}, {185U}, {200U}};
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        Preparar();
        cuenta_eventos = 12345U;
        const uint32_t anterior = SensorOptico_Read();
        cuenta_eventos = 12345U + casos[i].pulsos;
        const uint32_t actual = SensorOptico_Read();
        const uint32_t pulsos = actual - anterior;
        CHECK(pulsos == casos[i].pulsos);
        CHECK(cuenta_eventos == actual && consultas_eventos == 2U && consultas_base == 0U && !error_modelo);
    }
    return true;
}

static bool VentanaCruzaRollover(void)
{
    Preparar();
    cuenta_eventos = UINT32_MAX - 29U;
    const uint32_t anterior = SensorOptico_Read();
    cuenta_eventos = 30U;
    const uint32_t actual = SensorOptico_Read();
    const uint32_t pulsos = actual - anterior;
    CHECK(pulsos == 60U);
    CHECK(cuenta_eventos == 30U && cuenta_base == UINT32_C(0xA5A5A5A5));
    CHECK(consultas_eventos == 2U && consultas_base == 0U && !error_modelo);
    return true;
}

static bool NuevaEpocaReferenciaExterna(void)
{
    Preparar();
    cuenta_eventos = 700U;
    CHECK(SensorOptico_Read() == 700U);
    cuenta_eventos = 0U;
    const uint32_t nueva_referencia = SensorOptico_Read();
    cuenta_eventos = 60U;
    const uint32_t actual = SensorOptico_Read();
    const uint32_t pulsos = actual - nueva_referencia;
    CHECK(pulsos == 60U && cuenta_eventos == 60U);
    CHECK(consultas_eventos == 3U && consultas_base == 0U && !error_modelo);
    return true;
}

static bool DuracionYFactorExplicitos(void)
{
    Preparar();
    cuenta_eventos = 100U;
    const uint32_t anterior = SensorOptico_Read();
    cuenta_eventos = 112U;
    const uint32_t actual = SensorOptico_Read();
    const uint32_t pulsos = actual - anterior;
    CHECK(pulsos == 12U && cuenta_eventos == 112U);
    CHECK(consultas_eventos == 2U && consultas_base == 0U && !error_modelo);
    return true;
}

static bool InicializacionAcordada(void)
{
    Preparar();
    cuenta_eventos = 777U;
    SensorOptico_Init();
    CHECK(eventos_total == 3U && eventos[0] == EV_CONTADOR && eventos[1] == EV_PIN);
    CHECK(eventos[2] == EV_ENABLE);
    CHECK(contador_visto.input == TIM_CAPTURE_0 && contador_visto.edge == TIM_CTR_RISING);
    CHECK(((unsigned)contador_visto.edge | ((unsigned)contador_visto.input << 2)) == 0x01U);
    CHECK(pin_visto.port == PORT_1 && pin_visto.pin == PIN_18 && pin_visto.func == PINSEL_FUNC_11);
    CHECK(pin_visto.mode == PINSEL_PULLUP && pin_visto.openDrain == DISABLE);
    CHECK(SensorOptico_Read() == 0U && cuenta_base == UINT32_C(0xA5A5A5A5));
    CHECK(consultas_eventos == 1U && consultas_base == 0U && !error_modelo);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"I1 inicializacion acordada", InicializacionAcordada},
        {"S1 lecturas exactas/base independiente", LecturasExactasYBaseIndependiente},
        {"S2 repeticion/avance", RepeticionYAvance},
        {"S3 ventana de referencia", VentanaUnSegundo}, {"S4 rollover", VentanaCruzaRollover},
        {"S2 nueva referencia externa", NuevaEpocaReferenciaExterna},
        {"S3 diferencia de conteo", DuracionYFactorExplicitos}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
