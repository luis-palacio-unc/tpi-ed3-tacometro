#include "LM35.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static bool Esperar(uint32_t muestra, uint32_t escala, uint32_t referencia,
                    LM35_Estado estado, uint32_t esperado)
{
    LM35_Lectura lectura;
    CHECK(LM35_InicializarLectura(&lectura));
    CHECK(LM35_ConvertirMuestra(&lectura, muestra, escala, referencia) == estado);
    CHECK(lectura.estado == estado && lectura.centesimas_c == esperado);
    return true;
}

/* Oraculo de intervalos exactos; no calcula el cociente de produccion. */
static bool VerificarIntervalo(uint32_t muestra, uint32_t escala, uint32_t referencia)
{
    LM35_Lectura lectura;
    const LM35_Estado estado = LM35_ConvertirMuestra(&lectura, muestra, escala, referencia);
    CHECK(lectura.estado == estado);
    if (escala == 0U) {
        CHECK(estado == LM35_ERROR_ESCALA && lectura.centesimas_c == 0U);
    } else if (referencia == 0U) {
        CHECK(estado == LM35_ERROR_REFERENCIA && lectura.centesimas_c == 0U);
    } else if (muestra >= escala) {
        CHECK(estado == LM35_ERROR_MUESTRA && lectura.centesimas_c == 0U);
    } else {
        CHECK(estado == LM35_VALIDA && lectura.centesimas_c <= 42949673U);
        const uint64_t n = (uint64_t)muestra * referencia;
        const uint64_t d = (uint64_t)escala * 100U;
        if (lectura.centesimas_c == 0U) {
            CHECK(n < d / 2U);
        } else {
            const uint64_t base = ((uint64_t)lectura.centesimas_c - 1U) * d;
            CHECK(n >= base);
            const uint64_t diferencia = n - base;
            CHECK(diferencia >= d / 2U && diferencia < d + d / 2U);
        }
    }
    return true;
}

static bool AusenciaYCeroValido(void)
{
    LM35_Lectura lectura;
    CHECK(LM35_InicializarLectura(&lectura));
    CHECK(lectura.estado == LM35_SIN_DATOS && lectura.centesimas_c == 0U);
    CHECK(LM35_ConvertirMuestra(&lectura, 0U, 4096U, 3000000U) == LM35_VALIDA);
    CHECK(lectura.estado == LM35_VALIDA && lectura.centesimas_c == 0U);
    CHECK(LM35_CENTESIMAS_POR_GRADO == 100U);
    return true;
}

static bool ReferenciasNominales(void)
{
    CHECK(Esperar(125U, 1500U, 3000000U, LM35_VALIDA, 2500U));
    CHECK(Esperar(500U, 1500U, 3000000U, LM35_VALIDA, 10000U));
    CHECK(Esperar(1024U, 4096U, 1000000U, LM35_VALIDA, 2500U));
    return true;
}

static bool ConfiguracionesEquivalentes(void)
{
    CHECK(Esperar(1U, 4U, 1000000U, LM35_VALIDA, 2500U));
    CHECK(Esperar(256U, 1024U, 1000000U, LM35_VALIDA, 2500U));
    CHECK(Esperar(1024U, 4096U, 1000000U, LM35_VALIDA, 2500U));
    CHECK(Esperar(2U, 4U, 500000U, LM35_VALIDA, 2500U));
    CHECK(Esperar(500U, 3000U, 1500000U, LM35_VALIDA, 2500U));
    return true;
}

static bool EscalaYUltimoCodigo(void)
{
    CHECK(Esperar(4095U, 4096U, 409600U, LM35_VALIDA, 4095U));
    CHECK(Esperar(4096U, 4096U, 409600U, LM35_ERROR_MUESTRA, 0U));
    CHECK(Esperar(1023U, 1024U, 102400U, LM35_VALIDA, 1023U));
    CHECK(Esperar(0U, 1U, UINT32_MAX, LM35_VALIDA, 0U));
    CHECK(Esperar(1U, 1U, UINT32_MAX, LM35_ERROR_MUESTRA, 0U));
    return true;
}

static bool RedondeoYEmpates(void)
{
    CHECK(Esperar(1U, 2U, 99U, LM35_VALIDA, 0U));
    CHECK(Esperar(1U, 2U, 100U, LM35_VALIDA, 1U));
    CHECK(Esperar(1U, 2U, 101U, LM35_VALIDA, 1U));
    CHECK(Esperar(1U, 2U, 299U, LM35_VALIDA, 1U));
    CHECK(Esperar(1U, 2U, 300U, LM35_VALIDA, 2U));
    CHECK(Esperar(1U, 2U, 301U, LM35_VALIDA, 2U));
    return true;
}

static bool SinRedondeoIntermedio(void)
{
    CHECK(Esperar(1U, 4U, 4999U, LM35_VALIDA, 12U));
    CHECK(Esperar(1U, 4U, 5000U, LM35_VALIDA, 13U));
    CHECK(Esperar(1U, 4U, 5001U, LM35_VALIDA, 13U));
    CHECK(Esperar(1U, 3U, 149U, LM35_VALIDA, 0U));
    CHECK(Esperar(1U, 3U, 150U, LM35_VALIDA, 1U));
    return true;
}

static bool EscalaCeroInvalida(void)
{
    LM35_Lectura lectura;
    CHECK(LM35_ConvertirMuestra(&lectura, 1U, 4U, 1000000U) == LM35_VALIDA);
    CHECK(LM35_ConvertirMuestra(&lectura, 0U, 0U, 3000000U) == LM35_ERROR_ESCALA);
    CHECK(lectura.estado == LM35_ERROR_ESCALA && lectura.centesimas_c == 0U);
    CHECK(Esperar(UINT32_MAX, 0U, 0U, LM35_ERROR_ESCALA, 0U));
    return true;
}

static bool ReferenciaCeroInvalida(void)
{
    LM35_Lectura lectura;
    CHECK(LM35_ConvertirMuestra(&lectura, 1U, 4U, 1000000U) == LM35_VALIDA);
    CHECK(LM35_ConvertirMuestra(&lectura, 0U, 4U, 0U) == LM35_ERROR_REFERENCIA);
    CHECK(lectura.estado == LM35_ERROR_REFERENCIA && lectura.centesimas_c == 0U);
    CHECK(Esperar(UINT32_MAX, 1U, 0U, LM35_ERROR_REFERENCIA, 0U));
    return true;
}

static bool MuestraExcesivaInvalida(void)
{
    LM35_Lectura lectura;
    CHECK(LM35_ConvertirMuestra(&lectura, 1U, 4U, 1000000U) == LM35_VALIDA);
    CHECK(LM35_ConvertirMuestra(&lectura, 4U, 4U, 1000000U) == LM35_ERROR_MUESTRA);
    CHECK(lectura.estado == LM35_ERROR_MUESTRA && lectura.centesimas_c == 0U);
    CHECK(Esperar(UINT32_MAX, 4096U, 3000000U, LM35_ERROR_MUESTRA, 0U));
    return true;
}

static bool RecuperacionYCambioDeConfiguracion(void)
{
    LM35_Lectura lectura;
    CHECK(LM35_ConvertirMuestra(&lectura, 1U, 0U, 0U) == LM35_ERROR_ESCALA);
    CHECK(LM35_ConvertirMuestra(&lectura, 1U, 4U, 1000000U) == LM35_VALIDA);
    CHECK(lectura.centesimas_c == 2500U);
    CHECK(LM35_ConvertirMuestra(&lectura, 1U, 4U, 0U) == LM35_ERROR_REFERENCIA);
    CHECK(LM35_ConvertirMuestra(&lectura, 500U, 1500U, 3000000U) == LM35_VALIDA);
    CHECK(lectura.centesimas_c == 10000U);
    CHECK(LM35_ConvertirMuestra(&lectura, 1500U, 1500U, 3000000U) == LM35_ERROR_MUESTRA);
    CHECK(LM35_ConvertirMuestra(&lectura, 0U, 4096U, 3000000U) == LM35_VALIDA);
    CHECK(lectura.centesimas_c == 0U && lectura.estado == LM35_VALIDA);
    return true;
}

static bool PunterosNulosYPrioridad(void)
{
    CHECK(!LM35_InicializarLectura(NULL));
    CHECK(LM35_ConvertirMuestra(NULL, 0U, 0U, 0U) == LM35_ERROR_ARGUMENTO);
    CHECK(LM35_ConvertirMuestra(NULL, UINT32_MAX, 0U, UINT32_MAX) == LM35_ERROR_ARGUMENTO);
    CHECK(LM35_ConvertirMuestra(NULL, 0U, 1U, 0U) == LM35_ERROR_ARGUMENTO);
    CHECK(LM35_ConvertirMuestra(NULL, UINT32_MAX, UINT32_MAX, UINT32_MAX) == LM35_ERROR_ARGUMENTO);
    CHECK(LM35_ConvertirMuestra(NULL, 1U, 4U, 1000000U) == LM35_ERROR_ARGUMENTO);
    return true;
}

static bool Reinicio(void)
{
    LM35_Lectura lectura;
    CHECK(LM35_ConvertirMuestra(&lectura, 1U, 4U, 1000000U) == LM35_VALIDA);
    CHECK(LM35_InicializarLectura(&lectura));
    CHECK(lectura.estado == LM35_SIN_DATOS && lectura.centesimas_c == 0U);
    CHECK(LM35_ConvertirMuestra(&lectura, 1U, 0U, 0U) == LM35_ERROR_ESCALA);
    CHECK(LM35_InicializarLectura(&lectura));
    CHECK(lectura.estado == LM35_SIN_DATOS && lectura.centesimas_c == 0U);
    return true;
}

static bool InstanciasIndependientes(void)
{
    LM35_Lectura a, b;
    CHECK(LM35_ConvertirMuestra(&a, 1U, 4U, 1000000U) == LM35_VALIDA);
    CHECK(LM35_ConvertirMuestra(&b, 500U, 1500U, 3000000U) == LM35_VALIDA);
    CHECK(a.centesimas_c == 2500U && a.estado == LM35_VALIDA);
    CHECK(LM35_ConvertirMuestra(&a, 1U, 0U, 0U) == LM35_ERROR_ESCALA);
    CHECK(b.centesimas_c == 10000U && b.estado == LM35_VALIDA);
    CHECK(LM35_InicializarLectura(&a));
    CHECK(b.centesimas_c == 10000U && b.estado == LM35_VALIDA);
    return true;
}

static bool ExtremosDeTipos(void)
{
    CHECK(Esperar(UINT32_MAX - 1U, UINT32_MAX, UINT32_MAX, LM35_VALIDA, 42949673U));
    CHECK(Esperar(UINT32_MAX - 2U, UINT32_MAX, UINT32_MAX, LM35_VALIDA, 42949673U));
    CHECK(Esperar(UINT32_MAX, UINT32_MAX, UINT32_MAX, LM35_ERROR_MUESTRA, 0U));
    CHECK(Esperar(1U, UINT32_MAX, UINT32_MAX, LM35_VALIDA, 0U));
    CHECK(Esperar(UINT32_MAX - 1U, UINT32_MAX, 1U, LM35_VALIDA, 0U));
    CHECK(Esperar(0U, UINT32_MAX, UINT32_MAX, LM35_VALIDA, 0U));
    return true;
}

static bool MatrizDeFronteras(void)
{
    const uint32_t valores[] = {0U, 1U, 2U, 3U, 4U, 16U, 1000U, 4096U, UINT32_MAX - 1U, UINT32_MAX};
    for (size_t a = 0U; a < sizeof(valores) / sizeof(valores[0]); ++a) {
        for (size_t s = 0U; s < sizeof(valores) / sizeof(valores[0]); ++s) {
            for (size_t v = 0U; v < sizeof(valores) / sizeof(valores[0]); ++v) {
                CHECK(VerificarIntervalo(valores[a], valores[s], valores[v]));
            }
        }
    }
    return true;
}

static bool BarridoDeterminista(void)
{
    uint32_t semilla = 0x425035U;
    for (uint32_t i = 0U; i < 10000U; ++i) {
        semilla = semilla * 1664525U + 1013904223U;
        const uint32_t escala = semilla;
        semilla = semilla * 1664525U + 1013904223U;
        const uint32_t referencia = semilla;
        semilla = semilla * 1664525U + 1013904223U;
        uint32_t muestra = semilla;
        if (i % 2U == 0U && escala != 0U) {
            muestra %= escala;
        }
        CHECK(VerificarIntervalo(muestra, escala, referencia));
    }
    return true;
}

static bool RecorridosMonotonos(void)
{
    const uint32_t escalas[] = {1U, 2U, 3U, 16U, 1024U, 4096U, 65536U};
    for (size_t s = 0U; s < sizeof(escalas) / sizeof(escalas[0]); ++s) {
        uint32_t anterior = 0U;
        for (uint32_t muestra = 0U; muestra < escalas[s]; ++muestra) {
            LM35_Lectura lectura;
            CHECK(LM35_ConvertirMuestra(&lectura, muestra, escalas[s], 3000000U) == LM35_VALIDA);
            CHECK(lectura.centesimas_c >= anterior && lectura.centesimas_c <= 30000U);
            CHECK(VerificarIntervalo(muestra, escalas[s], 3000000U));
            anterior = lectura.centesimas_c;
        }
    }
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S4 ausencia/cero valido", AusenciaYCeroValido}, {"S1 referencias", ReferenciasNominales},
        {"S2 configuraciones equivalentes", ConfiguracionesEquivalentes},
        {"S1 ultimo codigo admisible", EscalaYUltimoCodigo}, {"S3 redondeo/empates", RedondeoYEmpates},
        {"S3 sin redondeo intermedio", SinRedondeoIntermedio}, {"S5 escala cero", EscalaCeroInvalida},
        {"S5 referencia cero", ReferenciaCeroInvalida}, {"S5 muestra excesiva", MuestraExcesivaInvalida},
        {"S5 recuperacion/configuracion", RecuperacionYCambioDeConfiguracion},
        {"S5 NULL/prioridad", PunterosNulosYPrioridad}, {"S4 reinicio", Reinicio},
        {"S7 instancias", InstanciasIndependientes}, {"S6 extremos uint32", ExtremosDeTipos},
        {"S3/S5/S6 1000 fronteras", MatrizDeFronteras}, {"S3/S6 10000 ternas", BarridoDeterminista},
        {"S1/S3 70678 puntos de recorrido", RecorridosMonotonos}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) {
            fprintf(stderr, "FAIL %s\n", casos[i].nombre);
            return 1;
        }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
