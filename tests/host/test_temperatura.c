#include "Temperatura.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static bool Esperar(uint32_t muestra, uint32_t escala, uint32_t referencia,
                    Temperatura_Estado estado, uint32_t esperado)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Inicializar(&lectura));
    CHECK(Temperatura_Actualizar(&lectura, muestra, escala, referencia) == estado);
    CHECK(lectura.estado == estado && lectura.centesimas_c == esperado);
    return true;
}

/* Oraculo de intervalos exactos; no calcula el cociente de produccion. */
static bool VerificarIntervalo(uint32_t muestra, uint32_t escala, uint32_t referencia)
{
    Temperatura_Lectura lectura;
    const Temperatura_Estado estado = Temperatura_Actualizar(&lectura, muestra, escala, referencia);
    CHECK(lectura.estado == estado);
    if (escala == 0U) {
        CHECK(estado == TEMPERATURA_ERROR_ESCALA && lectura.centesimas_c == 0U);
    } else if (referencia == 0U) {
        CHECK(estado == TEMPERATURA_ERROR_REFERENCIA && lectura.centesimas_c == 0U);
    } else if (muestra >= escala) {
        CHECK(estado == TEMPERATURA_ERROR_MUESTRA && lectura.centesimas_c == 0U);
    } else {
        CHECK(estado == TEMPERATURA_VALIDA && lectura.centesimas_c <= 42949673U);
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
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Inicializar(&lectura));
    CHECK(lectura.estado == TEMPERATURA_SIN_DATOS && lectura.centesimas_c == 0U);
    CHECK(Temperatura_Actualizar(&lectura, 0U, 4096U, 3000000U) == TEMPERATURA_VALIDA);
    CHECK(lectura.estado == TEMPERATURA_VALIDA && lectura.centesimas_c == 0U);
    CHECK(TEMPERATURA_CENTESIMAS_POR_GRADO == 100U);
    return true;
}

static bool ReferenciasNominales(void)
{
    CHECK(Esperar(125U, 1500U, 3000000U, TEMPERATURA_VALIDA, 2500U));
    CHECK(Esperar(500U, 1500U, 3000000U, TEMPERATURA_VALIDA, 10000U));
    CHECK(Esperar(1024U, 4096U, 1000000U, TEMPERATURA_VALIDA, 2500U));
    return true;
}

static bool ConfiguracionesEquivalentes(void)
{
    CHECK(Esperar(1U, 4U, 1000000U, TEMPERATURA_VALIDA, 2500U));
    CHECK(Esperar(256U, 1024U, 1000000U, TEMPERATURA_VALIDA, 2500U));
    CHECK(Esperar(1024U, 4096U, 1000000U, TEMPERATURA_VALIDA, 2500U));
    CHECK(Esperar(2U, 4U, 500000U, TEMPERATURA_VALIDA, 2500U));
    CHECK(Esperar(500U, 3000U, 1500000U, TEMPERATURA_VALIDA, 2500U));
    return true;
}

static bool EscalaYUltimoCodigo(void)
{
    CHECK(Esperar(4095U, 4096U, 409600U, TEMPERATURA_VALIDA, 4095U));
    CHECK(Esperar(4096U, 4096U, 409600U, TEMPERATURA_ERROR_MUESTRA, 0U));
    CHECK(Esperar(1023U, 1024U, 102400U, TEMPERATURA_VALIDA, 1023U));
    CHECK(Esperar(0U, 1U, UINT32_MAX, TEMPERATURA_VALIDA, 0U));
    CHECK(Esperar(1U, 1U, UINT32_MAX, TEMPERATURA_ERROR_MUESTRA, 0U));
    return true;
}

static bool RedondeoYEmpates(void)
{
    CHECK(Esperar(1U, 2U, 99U, TEMPERATURA_VALIDA, 0U));
    CHECK(Esperar(1U, 2U, 100U, TEMPERATURA_VALIDA, 1U));
    CHECK(Esperar(1U, 2U, 101U, TEMPERATURA_VALIDA, 1U));
    CHECK(Esperar(1U, 2U, 299U, TEMPERATURA_VALIDA, 1U));
    CHECK(Esperar(1U, 2U, 300U, TEMPERATURA_VALIDA, 2U));
    CHECK(Esperar(1U, 2U, 301U, TEMPERATURA_VALIDA, 2U));
    return true;
}

static bool SinRedondeoIntermedio(void)
{
    CHECK(Esperar(1U, 4U, 4999U, TEMPERATURA_VALIDA, 12U));
    CHECK(Esperar(1U, 4U, 5000U, TEMPERATURA_VALIDA, 13U));
    CHECK(Esperar(1U, 4U, 5001U, TEMPERATURA_VALIDA, 13U));
    CHECK(Esperar(1U, 3U, 149U, TEMPERATURA_VALIDA, 0U));
    CHECK(Esperar(1U, 3U, 150U, TEMPERATURA_VALIDA, 1U));
    return true;
}

static bool EscalaCeroInvalida(void)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Actualizar(&lectura, 1U, 4U, 1000000U) == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Actualizar(&lectura, 0U, 0U, 3000000U) == TEMPERATURA_ERROR_ESCALA);
    CHECK(lectura.estado == TEMPERATURA_ERROR_ESCALA && lectura.centesimas_c == 0U);
    CHECK(Esperar(UINT32_MAX, 0U, 0U, TEMPERATURA_ERROR_ESCALA, 0U));
    return true;
}

static bool ReferenciaCeroInvalida(void)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Actualizar(&lectura, 1U, 4U, 1000000U) == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Actualizar(&lectura, 0U, 4U, 0U) == TEMPERATURA_ERROR_REFERENCIA);
    CHECK(lectura.estado == TEMPERATURA_ERROR_REFERENCIA && lectura.centesimas_c == 0U);
    CHECK(Esperar(UINT32_MAX, 1U, 0U, TEMPERATURA_ERROR_REFERENCIA, 0U));
    return true;
}

static bool MuestraExcesivaInvalida(void)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Actualizar(&lectura, 1U, 4U, 1000000U) == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Actualizar(&lectura, 4U, 4U, 1000000U) == TEMPERATURA_ERROR_MUESTRA);
    CHECK(lectura.estado == TEMPERATURA_ERROR_MUESTRA && lectura.centesimas_c == 0U);
    CHECK(Esperar(UINT32_MAX, 4096U, 3000000U, TEMPERATURA_ERROR_MUESTRA, 0U));
    return true;
}

static bool RecuperacionYCambioDeConfiguracion(void)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Actualizar(&lectura, 1U, 0U, 0U) == TEMPERATURA_ERROR_ESCALA);
    CHECK(Temperatura_Actualizar(&lectura, 1U, 4U, 1000000U) == TEMPERATURA_VALIDA);
    CHECK(lectura.centesimas_c == 2500U);
    CHECK(Temperatura_Actualizar(&lectura, 1U, 4U, 0U) == TEMPERATURA_ERROR_REFERENCIA);
    CHECK(Temperatura_Actualizar(&lectura, 500U, 1500U, 3000000U) == TEMPERATURA_VALIDA);
    CHECK(lectura.centesimas_c == 10000U);
    CHECK(Temperatura_Actualizar(&lectura, 1500U, 1500U, 3000000U) == TEMPERATURA_ERROR_MUESTRA);
    CHECK(Temperatura_Actualizar(&lectura, 0U, 4096U, 3000000U) == TEMPERATURA_VALIDA);
    CHECK(lectura.centesimas_c == 0U && lectura.estado == TEMPERATURA_VALIDA);
    return true;
}

static bool PunterosNulosYPrioridad(void)
{
    CHECK(!Temperatura_Inicializar(NULL));
    CHECK(Temperatura_Actualizar(NULL, 0U, 0U, 0U) == TEMPERATURA_ERROR_ARGUMENTO);
    CHECK(Temperatura_Actualizar(NULL, UINT32_MAX, 0U, UINT32_MAX) == TEMPERATURA_ERROR_ARGUMENTO);
    CHECK(Temperatura_Actualizar(NULL, 0U, 1U, 0U) == TEMPERATURA_ERROR_ARGUMENTO);
    CHECK(Temperatura_Actualizar(NULL, UINT32_MAX, UINT32_MAX, UINT32_MAX) == TEMPERATURA_ERROR_ARGUMENTO);
    CHECK(Temperatura_Actualizar(NULL, 1U, 4U, 1000000U) == TEMPERATURA_ERROR_ARGUMENTO);
    return true;
}

static bool Reinicio(void)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Actualizar(&lectura, 1U, 4U, 1000000U) == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Inicializar(&lectura));
    CHECK(lectura.estado == TEMPERATURA_SIN_DATOS && lectura.centesimas_c == 0U);
    CHECK(Temperatura_Actualizar(&lectura, 1U, 0U, 0U) == TEMPERATURA_ERROR_ESCALA);
    CHECK(Temperatura_Inicializar(&lectura));
    CHECK(lectura.estado == TEMPERATURA_SIN_DATOS && lectura.centesimas_c == 0U);
    return true;
}

static bool InstanciasIndependientes(void)
{
    Temperatura_Lectura a, b;
    CHECK(Temperatura_Actualizar(&a, 1U, 4U, 1000000U) == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Actualizar(&b, 500U, 1500U, 3000000U) == TEMPERATURA_VALIDA);
    CHECK(a.centesimas_c == 2500U && a.estado == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Actualizar(&a, 1U, 0U, 0U) == TEMPERATURA_ERROR_ESCALA);
    CHECK(b.centesimas_c == 10000U && b.estado == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Inicializar(&a));
    CHECK(b.centesimas_c == 10000U && b.estado == TEMPERATURA_VALIDA);
    return true;
}

static bool ExtremosDeTipos(void)
{
    CHECK(Esperar(UINT32_MAX - 1U, UINT32_MAX, UINT32_MAX, TEMPERATURA_VALIDA, 42949673U));
    CHECK(Esperar(UINT32_MAX - 2U, UINT32_MAX, UINT32_MAX, TEMPERATURA_VALIDA, 42949673U));
    CHECK(Esperar(UINT32_MAX, UINT32_MAX, UINT32_MAX, TEMPERATURA_ERROR_MUESTRA, 0U));
    CHECK(Esperar(1U, UINT32_MAX, UINT32_MAX, TEMPERATURA_VALIDA, 0U));
    CHECK(Esperar(UINT32_MAX - 1U, UINT32_MAX, 1U, TEMPERATURA_VALIDA, 0U));
    CHECK(Esperar(0U, UINT32_MAX, UINT32_MAX, TEMPERATURA_VALIDA, 0U));
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
            Temperatura_Lectura lectura;
            CHECK(Temperatura_Actualizar(&lectura, muestra, escalas[s], 3000000U) == TEMPERATURA_VALIDA);
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
