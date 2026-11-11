#include "Motor.h"
#include "lpc17xx_pwm.h"
#include <stddef.h>
#include <stdio.h>

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

static void Preparar(void)
{
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

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
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
