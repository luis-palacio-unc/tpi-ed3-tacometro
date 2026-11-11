#include "Motor.h"
#include "lpc17xx_pwm.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

LPC_PWM_TypeDef tpi_pwm_modelo;

static const uint32_t centinelas[] = {0U, 0U, 23U, 37U, 53U, 71U, 97U};
static uint32_t matches[7];
static size_t llamadas;
static bool error_modelo;

void PWM_MatchUpdateSingle(PWM_MATCH_OPT match, uint32_t valor)
{
    ++llamadas;
    const unsigned indice = (unsigned)match;
    if (indice > 6U) { error_modelo = true; return; }
    matches[indice] = valor;
}

static void Preparar(uint32_t periodo)
{
    for (size_t i = 0U; i < 7U; ++i) { matches[i] = centinelas[i]; }
    matches[0] = periodo;
    tpi_pwm_modelo.MR0 = periodo;
    tpi_pwm_modelo.MR1 = 73U;
    llamadas = 0U;
    error_modelo = false;
}

static bool OtrosConservados(uint32_t periodo)
{
    CHECK(matches[0] == periodo && !error_modelo);
    CHECK(tpi_pwm_modelo.MR0 == periodo && tpi_pwm_modelo.MR1 == 73U);
    for (size_t i = 2U; i < 7U; ++i) { CHECK(matches[i] == centinelas[i]); }
    return true;
}

static bool ProbarOrden(uint16_t mando, uint32_t periodo, uint32_t esperado)
{
    const size_t previas = llamadas;
    CHECK(Motor_Write(mando));
    CHECK(llamadas == previas + 1U && matches[1] == esperado);
    CHECK(OtrosConservados(periodo));
    return true;
}

static bool DominioYExtremos(void)
{
    Preparar(1000U);
    const struct { uint16_t mando; uint32_t esperado; } casos[] = {
        {0U, 0U}, {1U, 1U}, {185U, 185U}, {333U, 333U},
        {500U, 500U}, {999U, 999U}, {1000U, 1001U}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        CHECK(ProbarOrden(casos[i].mando, 1000U, casos[i].esperado));
    }
    CHECK(llamadas == 7U);
    return true;
}

static bool RedondeoYEmpates(void)
{
    const struct { uint32_t periodo; uint16_t mando; uint32_t esperado; } casos[] = {
        {1U, 0U, 0U}, {1U, 499U, 0U}, {1U, 500U, 2U}, {1U, 1000U, 2U},
        {2U, 249U, 0U}, {2U, 250U, 1U}, {2U, 749U, 1U}, {2U, 750U, 3U},
        {3U, 166U, 0U}, {3U, 167U, 1U}, {3U, 500U, 2U},
        {3U, 833U, 2U}, {3U, 834U, 4U}, {3U, 999U, 4U},
        {1001U, 1U, 1U}, {1001U, 499U, 499U}, {1001U, 500U, 501U},
        {1001U, 999U, 1000U}, {1001U, 1000U, 1002U}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        Preparar(casos[i].periodo);
        CHECK(ProbarOrden(casos[i].mando, casos[i].periodo, casos[i].esperado));
    }
    return true;
}

static bool PeriodosGrandes(void)
{
    const struct { uint32_t periodo; uint16_t mando; uint32_t esperado; } casos[] = {
        {UINT32_MAX - 1U, 0U, 0U}, {UINT32_MAX - 1U, 1U, 4294967U},
        {UINT32_MAX - 1U, 500U, 2147483647U},
        {UINT32_MAX - 1U, 999U, 4290672327U},
        {UINT32_MAX - 1U, 1000U, UINT32_MAX},
        {UINT32_MAX - 2U, 500U, 2147483647U},
        {UINT32_MAX - 2U, 1000U, UINT32_MAX - 1U},
        {4000000000U, 1U, 4000000U}, {4000000000U, 500U, 2000000000U}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        Preparar(casos[i].periodo);
        CHECK(ProbarOrden(casos[i].mando, casos[i].periodo, casos[i].esperado));
    }
    return true;
}

static bool InvalidosYRecuperacion(void)
{
    Preparar(1000U);
    CHECK(ProbarOrden(500U, 1000U, 500U));
    for (uint32_t mando = 1001U; mando <= UINT16_MAX; ++mando) {
        CHECK(!Motor_Write((uint16_t)mando));
        CHECK(llamadas == 1U && matches[1] == 500U);
    }
    const uint16_t validos[] = {0U, 500U, 1000U};
    for (size_t i = 0U; i < sizeof(validos) / sizeof(validos[0]); ++i) {
        tpi_pwm_modelo.MR0 = 0U;
        CHECK(!Motor_Write(validos[i]));
        CHECK(tpi_pwm_modelo.MR0 == 0U);
        tpi_pwm_modelo.MR0 = UINT32_MAX;
        CHECK(!Motor_Write(validos[i]));
        CHECK(tpi_pwm_modelo.MR0 == UINT32_MAX);
        CHECK(llamadas == 1U && matches[1] == 500U);
    }
    tpi_pwm_modelo.MR0 = 0U;
    CHECK(!Motor_Write(UINT16_MAX));
    tpi_pwm_modelo.MR0 = UINT32_MAX;
    CHECK(!Motor_Write(UINT16_MAX));
    tpi_pwm_modelo.MR0 = 1000U;
    CHECK(OtrosConservados(1000U));
    CHECK(ProbarOrden(0U, 1000U, 0U));
    CHECK(ProbarOrden(1000U, 1000U, 1001U));
    printf("64535 mandos fuera de rango rechazados sin escritura\n");
    return true;
}

static bool RepeticionYLecturaMR0(void)
{
    Preparar(1000U);
    CHECK(ProbarOrden(250U, 1000U, 250U));
    CHECK(ProbarOrden(250U, 1000U, 250U));
    CHECK(ProbarOrden(0U, 1000U, 0U));
    CHECK(ProbarOrden(0U, 1000U, 0U));
    CHECK(ProbarOrden(1000U, 1000U, 1001U));
    matches[0] = 2000U;
    tpi_pwm_modelo.MR0 = 2000U;
    CHECK(ProbarOrden(250U, 2000U, 500U));
    CHECK(ProbarOrden(250U, 2000U, 500U));
    CHECK(llamadas == 7U);
    return true;
}

static bool PropiedadesDeCuantizacion(void)
{
    const uint32_t periodos[] = {1U, 2U, 3U, 7U, 31U, 999U, 1000U, 1001U,
        65535U, 16777216U, 2147483648U, UINT32_MAX - 2U, UINT32_MAX - 1U};
    size_t combinaciones = 0U;
    for (size_t i = 0U; i < sizeof(periodos) / sizeof(periodos[0]); ++i) {
        const uint32_t periodo = periodos[i];
        Preparar(periodo);
        uint32_t anterior = 0U;
        for (uint32_t mando = 0U; mando <= 1000U; ++mando) {
            CHECK(Motor_Write((uint16_t)mando));
            CHECK(llamadas == (size_t)mando + 1U && OtrosConservados(periodo));
            const uint32_t match = matches[1];
            CHECK(match < periodo || match == periodo + 1U);
            const uint32_t activos = match > periodo ? periodo : match;
            const int64_t error = (int64_t)activos * 1000 - (int64_t)mando * periodo;
            CHECK(error >= -499 && error <= 500);
            CHECK(activos >= anterior);
            if (mando == 0U) { CHECK(match == 0U); }
            if (mando == 1000U) { CHECK(match == periodo + 1U); }
            anterior = activos;
            ++combinaciones;
        }
    }
    CHECK(combinaciones == 13013U);
    printf("13013 combinaciones: cota de error, empate arriba y monotonia\n");
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S1 dominio/extremos", DominioYExtremos},
        {"S2 redondeo/empates", RedondeoYEmpates},
        {"S2 periodos grandes", PeriodosGrandes},
        {"S3 invalidos/recuperacion", InvalidosYRecuperacion},
        {"S4/S6 repeticion/lectura actual MR0", RepeticionYLecturaMR0},
        {"S2 propiedades de cuantizacion", PropiedadesDeCuantizacion}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
