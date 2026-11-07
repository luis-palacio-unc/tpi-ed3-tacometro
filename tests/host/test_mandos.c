#include "Mandos.h"
#include "Procesamiento.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static bool Esperar(uint32_t lectura, uint32_t escala, Mandos_Estado estado, uint16_t valor)
{
    Mandos_MandoManual mando;
    CHECK(Mandos_Inicializar(&mando));
    CHECK(Mandos_NormalizarManual(&mando, lectura, escala) == estado);
    CHECK(mando.estado == estado && mando.por_mil == valor);
    return true;
}

/* Oraculo por distancias enteras; no calcula el cociente de produccion. */
static bool VerificarDistancia(uint32_t lectura, uint32_t escala)
{
    Mandos_MandoManual mando;
    const Mandos_Estado estado = Mandos_NormalizarManual(&mando, lectura, escala);
    CHECK(mando.estado == estado);
    if (escala == 0U) {
        CHECK(estado == MANDOS_ERROR_ESCALA && mando.por_mil == 0U);
    } else if (lectura > escala) {
        CHECK(estado == MANDOS_ERROR_LECTURA && mando.por_mil == 0U);
    } else {
        CHECK(estado == MANDOS_VALIDO && mando.por_mil <= 1000U);
        const uint64_t exacto = (uint64_t)lectura * 1000U;
        const uint64_t obtenido = (uint64_t)mando.por_mil * escala;
        const uint64_t distancia = obtenido >= exacto ? obtenido - exacto : exacto - obtenido;
        CHECK(distancia <= escala / 2U);
        if (escala % 2U == 0U && distancia == escala / 2U) {
            CHECK(obtenido > exacto);
        }
    }
    return true;
}

static bool AusenciaInicialYCeroValido(void)
{
    Mandos_MandoManual mando;
    CHECK(Mandos_Inicializar(&mando));
    CHECK(mando.estado == MANDOS_SIN_DATOS && mando.por_mil == 0U);
    CHECK(Mandos_NormalizarManual(&mando, 0U, 4095U) == MANDOS_VALIDO);
    CHECK(mando.estado == MANDOS_VALIDO && mando.por_mil == 0U);
    return true;
}

static bool ExtremosDeEscalas(void)
{
    const uint32_t escalas[] = {1U, 2U, 3U, 7U, 1000U, 1023U, 4095U, 65535U, UINT32_MAX};
    for (size_t i = 0U; i < sizeof(escalas) / sizeof(escalas[0]); ++i) {
        CHECK(Esperar(0U, escalas[i], MANDOS_VALIDO, 0U));
        CHECK(Esperar(escalas[i], escalas[i], MANDOS_VALIDO, 1000U));
    }
    CHECK(MANDOS_MANDO_MAX == 1000U && TPI_MANDO_MANUAL_MAX == 1000U);
    CHECK(PROCESAMIENTO_MANDO_MAX == MANDOS_MANDO_MAX && TPI_RPM_MAX == 185U);
    return true;
}

static bool MitadDelRecorrido(void)
{
    CHECK(Esperar(1U, 2U, MANDOS_VALIDO, 500U));
    CHECK(Esperar(512U, 1023U, MANDOS_VALIDO, 500U));
    CHECK(Esperar(2048U, 4095U, MANDOS_VALIDO, 500U));
    CHECK(Esperar(UINT32_MAX / 2U, UINT32_MAX, MANDOS_VALIDO, 500U));
    CHECK(Esperar(UINT32_MAX / 2U + 1U, UINT32_MAX, MANDOS_VALIDO, 500U));
    return true;
}

static bool EscalasEquivalentes(void)
{
    CHECK(Esperar(341U, 1023U, MANDOS_VALIDO, 333U));
    CHECK(Esperar(1365U, 4095U, MANDOS_VALIDO, 333U));
    CHECK(Esperar(682U, 1023U, MANDOS_VALIDO, 667U));
    CHECK(Esperar(2730U, 4095U, MANDOS_VALIDO, 667U));
    CHECK(Esperar(25U, 100U, MANDOS_VALIDO, 250U));
    CHECK(Esperar(1024U, 4096U, MANDOS_VALIDO, 250U));
    return true;
}

static bool RedondeoYEmpates(void)
{
    CHECK(Esperar(1U, 3U, MANDOS_VALIDO, 333U));
    CHECK(Esperar(2U, 3U, MANDOS_VALIDO, 667U));
    CHECK(Esperar(1U, 16U, MANDOS_VALIDO, 63U));
    CHECK(Esperar(1U, 1999U, MANDOS_VALIDO, 1U));
    CHECK(Esperar(1U, 2000U, MANDOS_VALIDO, 1U));
    CHECK(Esperar(1U, 2001U, MANDOS_VALIDO, 0U));
    CHECK(Esperar(1999U, 2000U, MANDOS_VALIDO, 1000U));
    return true;
}

static bool EscalaCeroInvalida(void)
{
    Mandos_MandoManual mando;
    CHECK(Mandos_NormalizarManual(&mando, 4095U, 4095U) == MANDOS_VALIDO);
    CHECK(Mandos_NormalizarManual(&mando, 0U, 0U) == MANDOS_ERROR_ESCALA);
    CHECK(mando.estado == MANDOS_ERROR_ESCALA && mando.por_mil == 0U);
    CHECK(Esperar(UINT32_MAX, 0U, MANDOS_ERROR_ESCALA, 0U));
    return true;
}

static bool LecturaExcesivaInvalida(void)
{
    Mandos_MandoManual mando;
    CHECK(Mandos_NormalizarManual(&mando, 4095U, 4095U) == MANDOS_VALIDO);
    CHECK(Mandos_NormalizarManual(&mando, 4096U, 4095U) == MANDOS_ERROR_LECTURA);
    CHECK(mando.estado == MANDOS_ERROR_LECTURA && mando.por_mil == 0U);
    CHECK(Esperar(UINT32_MAX, UINT32_MAX - 1U, MANDOS_ERROR_LECTURA, 0U));
    CHECK(Esperar(2U, 1U, MANDOS_ERROR_LECTURA, 0U));
    return true;
}

static bool RecuperacionYCambioDeEscala(void)
{
    Mandos_MandoManual mando;
    CHECK(Mandos_NormalizarManual(&mando, 1U, 0U) == MANDOS_ERROR_ESCALA);
    CHECK(Mandos_NormalizarManual(&mando, 512U, 1023U) == MANDOS_VALIDO);
    CHECK(mando.por_mil == 500U);
    CHECK(Mandos_NormalizarManual(&mando, 4096U, 4095U) == MANDOS_ERROR_LECTURA);
    CHECK(Mandos_NormalizarManual(&mando, 0U, 4095U) == MANDOS_VALIDO);
    CHECK(mando.por_mil == 0U && mando.estado == MANDOS_VALIDO);
    CHECK(Mandos_NormalizarManual(&mando, 7U, 7U) == MANDOS_VALIDO);
    CHECK(mando.por_mil == 1000U);
    return true;
}

static bool PunterosNulosYPrioridad(void)
{
    CHECK(!Mandos_Inicializar(NULL));
    CHECK(Mandos_NormalizarManual(NULL, 0U, 0U) == MANDOS_ERROR_ARGUMENTO);
    CHECK(Mandos_NormalizarManual(NULL, UINT32_MAX, 0U) == MANDOS_ERROR_ARGUMENTO);
    CHECK(Mandos_NormalizarManual(NULL, 1U, 2U) == MANDOS_ERROR_ARGUMENTO);
    CHECK(Mandos_NormalizarManual(NULL, 2U, 1U) == MANDOS_ERROR_ARGUMENTO);
    return true;
}

static bool Reinicio(void)
{
    Mandos_MandoManual mando;
    CHECK(Mandos_NormalizarManual(&mando, 4095U, 4095U) == MANDOS_VALIDO);
    CHECK(Mandos_Inicializar(&mando));
    CHECK(mando.estado == MANDOS_SIN_DATOS && mando.por_mil == 0U);
    CHECK(Mandos_NormalizarManual(&mando, 1U, 0U) == MANDOS_ERROR_ESCALA);
    CHECK(Mandos_Inicializar(&mando));
    CHECK(mando.estado == MANDOS_SIN_DATOS && mando.por_mil == 0U);
    return true;
}

static bool InstanciasIndependientes(void)
{
    Mandos_MandoManual a, b;
    CHECK(Mandos_NormalizarManual(&a, 2048U, 4095U) == MANDOS_VALIDO);
    CHECK(Mandos_NormalizarManual(&b, 341U, 1023U) == MANDOS_VALIDO);
    CHECK(a.por_mil == 500U && a.estado == MANDOS_VALIDO);
    CHECK(Mandos_NormalizarManual(&a, 1U, 0U) == MANDOS_ERROR_ESCALA);
    CHECK(b.por_mil == 333U && b.estado == MANDOS_VALIDO);
    CHECK(Mandos_Inicializar(&a));
    CHECK(b.por_mil == 333U && b.estado == MANDOS_VALIDO);
    return true;
}

static bool ExtremosDeTiposYMatriz(void)
{
    CHECK(Esperar(UINT32_MAX, UINT32_MAX, MANDOS_VALIDO, 1000U));
    CHECK(Esperar(UINT32_MAX - 1U, UINT32_MAX, MANDOS_VALIDO, 1000U));
    CHECK(Esperar(1U, UINT32_MAX, MANDOS_VALIDO, 0U));
    CHECK(Esperar(4294967U, 4294967U, MANDOS_VALIDO, 1000U));
    CHECK(Esperar(UINT32_MAX, 1U, MANDOS_ERROR_LECTURA, 0U));
    const uint32_t valores[] = {0U, 1U, 2U, 3U, 7U, 1000U, 1023U, 4095U, UINT32_MAX - 1U, UINT32_MAX};
    for (size_t r = 0U; r < sizeof(valores) / sizeof(valores[0]); ++r) {
        for (size_t e = 0U; e < sizeof(valores) / sizeof(valores[0]); ++e) {
            CHECK(VerificarDistancia(valores[r], valores[e]));
        }
    }
    return true;
}

static bool RecorridosMonotonos(void)
{
    const uint32_t escalas[] = {1U, 3U, 16U, 1000U, 1023U, 4095U, 65535U};
    for (size_t i = 0U; i < sizeof(escalas) / sizeof(escalas[0]); ++i) {
        Mandos_MandoManual mando;
        uint16_t anterior = 0U;
        for (uint32_t lectura = 0U; lectura <= escalas[i]; ++lectura) {
            CHECK(Mandos_NormalizarManual(&mando, lectura, escalas[i]) == MANDOS_VALIDO);
            CHECK(mando.por_mil >= anterior && mando.por_mil <= 1000U);
            CHECK(VerificarDistancia(lectura, escalas[i]));
            anterior = mando.por_mil;
        }
        CHECK(anterior == 1000U);
    }
    return true;
}

static bool BarridoDeterminista(void)
{
    uint32_t semilla = 0x425034U;
    for (uint32_t i = 0U; i < 10000U; ++i) {
        semilla = semilla * 1664525U + 1013904223U;
        const uint32_t lectura = semilla;
        semilla = semilla * 1664525U + 1013904223U;
        CHECK(VerificarDistancia(lectura, semilla));
    }
    return true;
}

static bool AdaptacionManualB01(void)
{
    Procesamiento_Contexto contexto;
    CHECK(Procesamiento_Inicializar(&contexto, PROCESAMIENTO_MODO_MANUAL, 0U));
    const uint32_t lecturas[] = {0U, 2048U, 4095U};
    const uint16_t esperados[] = {0U, 500U, 1000U};
    uint64_t orden = 0U;
    for (size_t i = 0U; i < sizeof(lecturas) / sizeof(lecturas[0]); ++i) {
        Mandos_MandoManual mando;
        CHECK(Mandos_NormalizarManual(&mando, lecturas[i], 4095U) == MANDOS_VALIDO);
        const Procesamiento_Entradas entradas = {PROCESAMIENTO_MODO_MANUAL, mando.por_mil, ++orden, NULL};
        Procesamiento_Salida salida;
        CHECK(Procesamiento_Actualizar(&contexto, &entradas, &salida));
        CHECK(salida.fuente == PROCESAMIENTO_FUENTE_MANUAL);
        CHECK(salida.mando_manual_por_mil == esperados[i] && salida.consigna_rpm == 0U);
    }
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S3 ausencia/cero valido", AusenciaInicialYCeroValido},
        {"S1 extremos y limite comun", ExtremosDeEscalas},
        {"S1 mitad del recorrido", MitadDelRecorrido}, {"S2 equivalencia", EscalasEquivalentes},
        {"S2 redondeo/empates", RedondeoYEmpates}, {"S4 escala cero", EscalaCeroInvalida},
        {"S4 lectura excesiva", LecturaExcesivaInvalida},
        {"S4 recuperacion/cambio de escala", RecuperacionYCambioDeEscala},
        {"S5 NULL/prioridad", PunterosNulosYPrioridad}, {"S3 reinicio", Reinicio},
        {"S6 instancias", InstanciasIndependientes},
        {"S5 extremos/100 fronteras", ExtremosDeTiposYMatriz},
        {"S1/S2 71680 puntos de recorrido", RecorridosMonotonos},
        {"S2/S5 10000 entradas", BarridoDeterminista}, {"S6 adaptacion B01", AdaptacionManualB01}
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
