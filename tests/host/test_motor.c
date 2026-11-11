#include "Motor.h"
#include "lpc17xx_pwm.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; } } while (0)

static size_t llamadas;
static PWM_MATCH_OPT ultimo_registro;
static uint32_t ultimo_valor;

void PWM_MatchUpdateSingle(PWM_MATCH_OPT registro, uint32_t valor)
{
    ++llamadas;
    ultimo_registro = registro;
    ultimo_valor = valor;
}

#define EVENTOS_MAX 16U
typedef enum {
    EV_BASE, EV_MATCH0, EV_MATCH1, EV_CANAL_MODO, EV_PIN, EV_CANAL_ON, EV_CONTADOR, EV_PWM
} Evento;
static Evento eventos[EVENTOS_MAX];
static size_t eventos_total;
static PWM_TIMERCFG_T base_vista;
static PWM_MATCHCFG_T match_visto[2];
static PWM_CHANNEL canal_modo, canal_habilitado;
static PWM_CHANNEL_EDGE flanco_visto;
static PWM_PIN_OPTION pin_visto;
static bool error_modelo;

static void Registrar(Evento evento)
{
    if (eventos_total < EVENTOS_MAX) { eventos[eventos_total] = evento; }
    ++eventos_total;
}

void PWM_InitTimer(const PWM_TIMERCFG_T *cfg)
{
    if (cfg == NULL) { error_modelo = true; return; }
    base_vista = *cfg;
    Registrar(EV_BASE);
}

void PWM_ConfigMatch(const PWM_MATCHCFG_T *cfg)
{
    if (cfg == NULL || (cfg->channel != PWM_MATCH_0 && cfg->channel != PWM_MATCH_1)) {
        error_modelo = true;
        return;
    }
    match_visto[cfg->channel] = *cfg;
    Registrar(cfg->channel == PWM_MATCH_0 ? EV_MATCH0 : EV_MATCH1);
}

void PWM_ChannelConfig(PWM_CHANNEL canal, PWM_CHANNEL_EDGE flanco)
{
    canal_modo = canal; flanco_visto = flanco;
    Registrar(EV_CANAL_MODO);
}

void PWM_PinConfig(PWM_PIN_OPTION opcion) { pin_visto = opcion; Registrar(EV_PIN); }
void PWM_ChannelEnable(PWM_CHANNEL canal) { canal_habilitado = canal; Registrar(EV_CANAL_ON); }
void PWM_CounterEnable(void) { Registrar(EV_CONTADOR); }
void PWM_Enable(void) { Registrar(EV_PWM); }

static void Preparar(void)
{
    eventos_total = 0U; error_modelo = false;
    memset(&base_vista, 0xFF, sizeof(base_vista)); memset(match_visto, 0xFF, sizeof(match_visto));
    canal_modo = PWM_CHANNEL_6; canal_habilitado = PWM_CHANNEL_6;
    flanco_visto = PWM_DUAL_EDGE; pin_visto = PWM6_P2_5;
    llamadas = 0U;
    ultimo_registro = PWM_MATCH_0;
    ultimo_valor = UINT32_C(0xA5A5A5A5);
}

static bool OrdenExacta(uint16_t mando)
{
    const size_t anteriores = llamadas;
    CHECK(Motor_Write(mando));
    CHECK(llamadas == anteriores + 1U);
    CHECK(ultimo_registro == PWM_MATCH_1 && ultimo_valor == mando);
    return true;
}

static bool ValoresDeReferencia(void)
{
    Preparar();
    const uint16_t mandos[] = {0U, 1U, 185U, 250U, 500U, 999U, 1000U};
    for (size_t i = 0U; i < sizeof(mandos) / sizeof(mandos[0]); ++i) {
        CHECK(OrdenExacta(mandos[i]));
    }
    CHECK(llamadas == 7U && ultimo_valor == 1000U);
    return true;
}

static bool DominioCompleto(void)
{
    Preparar();
    for (uint32_t mando = 0U; mando <= 1000U; ++mando) {
        CHECK(OrdenExacta((uint16_t)mando));
    }
    CHECK(llamadas == 1001U);
    puts("1001 mandos validos enviados sin conversion");
    return true;
}

static bool RechazoYRecuperacion(void)
{
    Preparar();
    CHECK(OrdenExacta(500U));
    for (uint32_t mando = 1001U; mando <= UINT16_MAX; ++mando) {
        CHECK(!Motor_Write((uint16_t)mando));
        CHECK(llamadas == 1U && ultimo_registro == PWM_MATCH_1 && ultimo_valor == 500U);
    }
    CHECK(OrdenExacta(0U));
    CHECK(OrdenExacta(1000U));
    puts("64535 mandos fuera de rango rechazados sin escritura");
    return true;
}

static bool RepeticionYCero(void)
{
    Preparar();
    const uint16_t mandos[] = {500U, 500U, 0U, 0U, 1000U, 1000U, 500U};
    for (size_t i = 0U; i < sizeof(mandos) / sizeof(mandos[0]); ++i) {
        CHECK(OrdenExacta(mandos[i]));
    }
    CHECK(llamadas == 7U);
    return true;
}

static bool InicializacionAcordada(void)
{
    Preparar();
    const Evento esperado[] = {EV_BASE, EV_MATCH0, EV_MATCH1, EV_CANAL_MODO, EV_PIN,
                               EV_CANAL_ON, EV_CONTADOR, EV_PWM};
    Motor_Init();
    CHECK(eventos_total == sizeof(esperado) / sizeof(esperado[0]));
    CHECK(memcmp(eventos, esperado, sizeof(esperado)) == 0);
    CHECK(base_vista.prescaleOpt == PWM_TICK && base_vista.prescaleValue - 1U == 24U);
    CHECK(UINT32_C(25000000) / base_vista.prescaleValue == UINT32_C(1000000));
    CHECK(match_visto[0].channel == PWM_MATCH_0 && match_visto[0].matchValue == 1000U);
    CHECK(match_visto[0].resetEn == ENABLE && match_visto[0].intEn == DISABLE);
    CHECK(match_visto[0].stopEn == DISABLE);
    CHECK(UINT32_C(1000000) / match_visto[0].matchValue == 1000U);
    CHECK(match_visto[1].channel == PWM_MATCH_1 && match_visto[1].matchValue == 0U);
    CHECK(match_visto[1].resetEn == DISABLE && match_visto[1].intEn == DISABLE);
    CHECK(match_visto[1].stopEn == DISABLE);
    CHECK(canal_modo == PWM_CHANNEL_1 && flanco_visto == PWM_SINGLE_EDGE);
    CHECK(pin_visto == PWM1_P2_0 && canal_habilitado == PWM_CHANNEL_1);
    CHECK(llamadas == 0U && !error_modelo);
    CHECK(OrdenExacta(0U) && OrdenExacta(1000U) && eventos_total == 8U);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"I1 inicializacion acordada", InicializacionAcordada},
        {"S1 referencias exactas", ValoresDeReferencia},
        {"S2 dominio completo", DominioCompleto},
        {"S3 rechazo/recuperacion", RechazoYRecuperacion},
        {"S4 repeticion/cero", RepeticionYCero}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
