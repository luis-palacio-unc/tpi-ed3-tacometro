#include "Medicion_Velocidad.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static bool Esperar(uint32_t pulsos, uint32_t us, Medicion_Velocidad_Estado estado,
                    uint32_t esperado)
{
    Medicion_Velocidad_Lectura lectura;
    CHECK(Medicion_Velocidad_Inicializar(&lectura));
    CHECK(Medicion_Velocidad_Actualizar(&lectura, pulsos, us) == estado);
    CHECK(lectura.estado == estado && lectura.rpm_milesimas == esperado);
    return true;
}

static bool AusenciaInicial(void)
{
    Medicion_Velocidad_Lectura lectura;
    CHECK(Medicion_Velocidad_Inicializar(&lectura));
    CHECK(lectura.estado == MEDICION_VELOCIDAD_SIN_DATOS && lectura.rpm_milesimas == 0U);
    CHECK(MEDICION_VELOCIDAD_PULSOS_POR_VUELTA == 60U);
    CHECK(MEDICION_VELOCIDAD_ESCALA_RPM == 1000U);
    CHECK(Medicion_Velocidad_Actualizar(&lectura, 0U, 1000000U) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(lectura.estado == MEDICION_VELOCIDAD_VALIDA && lectura.rpm_milesimas == 0U);
    return true;
}

static bool Referencias(void)
{
    CHECK(Esperar(0U, 1000000U, MEDICION_VELOCIDAD_VALIDA, 0U));
    CHECK(Esperar(60U, 1000000U, MEDICION_VELOCIDAD_VALIDA, 60000U));
    CHECK(Esperar(185U, 1000000U, MEDICION_VELOCIDAD_VALIDA, 185000U));
    CHECK(Esperar(200U, 1000000U, MEDICION_VELOCIDAD_VALIDA, 200000U));
    return true;
}

static bool SinRecorteDeConsigna(void)
{
    CHECK(Esperar(186U, 1000000U, MEDICION_VELOCIDAD_VALIDA, 186000U));
    CHECK(Esperar(1000U, 1000000U, MEDICION_VELOCIDAD_VALIDA, 1000000U));
    return true;
}

static bool VentanasEquivalentes(void)
{
    CHECK(Esperar(3U, 20000U, MEDICION_VELOCIDAD_VALIDA, 150000U));
    CHECK(Esperar(300U, 2000000U, MEDICION_VELOCIDAD_VALIDA, 150000U));
    CHECK(Esperar(600U, 10000000U, MEDICION_VELOCIDAD_VALIDA, 60000U));
    return true;
}

static bool Redondeo(void)
{
    CHECK(Esperar(1U, 3000000U, MEDICION_VELOCIDAD_VALIDA, 333U));
    CHECK(Esperar(1U, 6000000U, MEDICION_VELOCIDAD_VALIDA, 167U));
    CHECK(Esperar(1U, 16000000U, MEDICION_VELOCIDAD_VALIDA, 63U));
    CHECK(Esperar(1U, 15999999U, MEDICION_VELOCIDAD_VALIDA, 63U));
    CHECK(Esperar(1U, 16000001U, MEDICION_VELOCIDAD_VALIDA, 62U));
    return true;
}

static bool CeroRedondeadoValido(void)
{
    CHECK(Esperar(1U, UINT32_MAX, MEDICION_VELOCIDAD_VALIDA, 0U));
    CHECK(Esperar(1U, 2000000000U, MEDICION_VELOCIDAD_VALIDA, 1U));
    CHECK(Esperar(1U, 2000000001U, MEDICION_VELOCIDAD_VALIDA, 0U));
    return true;
}

static bool DuracionNulaYRecuperacion(void)
{
    Medicion_Velocidad_Lectura lectura;
    CHECK(Medicion_Velocidad_Actualizar(&lectura, 185U, 1000000U) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(Medicion_Velocidad_Actualizar(&lectura, 60U, 0U) == MEDICION_VELOCIDAD_ERROR_DURACION);
    CHECK(lectura.estado == MEDICION_VELOCIDAD_ERROR_DURACION && lectura.rpm_milesimas == 0U);
    CHECK(Medicion_Velocidad_Actualizar(&lectura, 60U, 1000000U) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(lectura.rpm_milesimas == 60000U);
    CHECK(Esperar(0U, 0U, MEDICION_VELOCIDAD_ERROR_DURACION, 0U));
    CHECK(Esperar(UINT32_MAX, 0U, MEDICION_VELOCIDAD_ERROR_DURACION, 0U));
    return true;
}

static bool RangoYRecuperacion(void)
{
    Medicion_Velocidad_Lectura lectura;
    CHECK(Medicion_Velocidad_Actualizar(&lectura, 200U, 1000000U) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(Medicion_Velocidad_Actualizar(&lectura, UINT32_MAX, 1U) == MEDICION_VELOCIDAD_ERROR_RANGO);
    CHECK(lectura.estado == MEDICION_VELOCIDAD_ERROR_RANGO && lectura.rpm_milesimas == 0U);
    CHECK(Medicion_Velocidad_Actualizar(&lectura, 0U, 1U) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(lectura.rpm_milesimas == 0U);
    return true;
}

static bool ExtremosDeTipos(void)
{
    CHECK(Esperar(UINT32_MAX, UINT32_MAX, MEDICION_VELOCIDAD_VALIDA, 1000000000U));
    CHECK(Esperar(UINT32_MAX, 1000000000U, MEDICION_VELOCIDAD_VALIDA, UINT32_MAX));
    CHECK(Esperar(UINT32_MAX, 999999999U, MEDICION_VELOCIDAD_ERROR_RANGO, 0U));
    CHECK(Esperar(UINT32_MAX, 1000000001U, MEDICION_VELOCIDAD_VALIDA, UINT32_MAX - 4U));
    CHECK(Esperar(4304180U, 1002145U, MEDICION_VELOCIDAD_VALIDA, UINT32_MAX));
    CHECK(Esperar(4304180U, 1002144U, MEDICION_VELOCIDAD_ERROR_RANGO, 0U));
    CHECK(Esperar(0U, UINT32_MAX, MEDICION_VELOCIDAD_VALIDA, 0U));
    return true;
}

static bool DuracionMinima(void)
{
    CHECK(Esperar(0U, 1U, MEDICION_VELOCIDAD_VALIDA, 0U));
    CHECK(Esperar(1U, 1U, MEDICION_VELOCIDAD_VALIDA, 1000000000U));
    CHECK(Esperar(4U, 1U, MEDICION_VELOCIDAD_VALIDA, 4000000000U));
    CHECK(Esperar(5U, 1U, MEDICION_VELOCIDAD_ERROR_RANGO, 0U));
    return true;
}

static bool PunterosNulos(void)
{
    CHECK(!Medicion_Velocidad_Inicializar(NULL));
    CHECK(Medicion_Velocidad_Actualizar(NULL, 0U, 0U) == MEDICION_VELOCIDAD_ERROR_ARGUMENTO);
    CHECK(Medicion_Velocidad_Actualizar(NULL, 185U, 1000000U) == MEDICION_VELOCIDAD_ERROR_ARGUMENTO);
    CHECK(Medicion_Velocidad_Actualizar(NULL, UINT32_MAX, 1U) == MEDICION_VELOCIDAD_ERROR_ARGUMENTO);
    return true;
}

static bool Reinicio(void)
{
    Medicion_Velocidad_Lectura lectura;
    CHECK(Medicion_Velocidad_Actualizar(&lectura, 185U, 1000000U) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(Medicion_Velocidad_Inicializar(&lectura));
    CHECK(lectura.estado == MEDICION_VELOCIDAD_SIN_DATOS && lectura.rpm_milesimas == 0U);
    CHECK(Medicion_Velocidad_Actualizar(&lectura, 0U, 0U) == MEDICION_VELOCIDAD_ERROR_DURACION);
    CHECK(Medicion_Velocidad_Inicializar(&lectura));
    CHECK(lectura.estado == MEDICION_VELOCIDAD_SIN_DATOS && lectura.rpm_milesimas == 0U);
    return true;
}

static bool InstanciasIndependientes(void)
{
    Medicion_Velocidad_Lectura a, b;
    CHECK(Medicion_Velocidad_Actualizar(&a, 200U, 1000000U) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(Medicion_Velocidad_Inicializar(&b));
    CHECK(a.rpm_milesimas == 200000U && a.estado == MEDICION_VELOCIDAD_VALIDA);
    CHECK(Medicion_Velocidad_Actualizar(&b, 1U, 0U) == MEDICION_VELOCIDAD_ERROR_DURACION);
    CHECK(a.rpm_milesimas == 200000U && a.estado == MEDICION_VELOCIDAD_VALIDA);
    CHECK(Medicion_Velocidad_Actualizar(&a, 60U, 1000000U) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(b.rpm_milesimas == 0U && b.estado == MEDICION_VELOCIDAD_ERROR_DURACION);
    return true;
}

/* Oraculo por distancias: no calcula el cociente pulsos/tiempo de produccion. */
static bool VerificarDistancia(uint32_t pulsos, uint32_t us)
{
    Medicion_Velocidad_Lectura lectura;
    const Medicion_Velocidad_Estado estado = Medicion_Velocidad_Actualizar(&lectura, pulsos, us);
    CHECK(lectura.estado == estado);
    if (us == 0U) {
        CHECK(estado == MEDICION_VELOCIDAD_ERROR_DURACION && lectura.rpm_milesimas == 0U);
        return true;
    }
    const uint64_t n = (uint64_t)pulsos * UINT64_C(1000000000);
    const uint64_t extremo = (uint64_t)UINT32_MAX * us;
    const bool fuera = n > extremo && n - extremo >= ((uint64_t)us + 1U) / 2U;
    if (fuera) {
        CHECK(estado == MEDICION_VELOCIDAD_ERROR_RANGO && lectura.rpm_milesimas == 0U);
    } else {
        CHECK(estado == MEDICION_VELOCIDAD_VALIDA);
        const uint64_t producto = (uint64_t)lectura.rpm_milesimas * us;
        const uint64_t diferencia = producto >= n ? producto - n : n - producto;
        CHECK(diferencia <= us / 2U);
        if (us % 2U == 0U && diferencia == us / 2U) {
            CHECK(producto > n);
        }
    }
    return true;
}

static bool MatrizDeFronteras(void)
{
    const uint32_t pulsos[] = {0U, 1U, 2U, 4U, 5U, 60U, 185U, 200U, UINT32_MAX - 1U, UINT32_MAX};
    const uint32_t tiempos[] = {0U, 1U, 2U, 3U, 1000U, 1000000U,
                               999999999U, 1000000000U, 1000000001U, UINT32_MAX};
    for (size_t p = 0U; p < sizeof(pulsos) / sizeof(pulsos[0]); ++p) {
        for (size_t t = 0U; t < sizeof(tiempos) / sizeof(tiempos[0]); ++t) {
            CHECK(VerificarDistancia(pulsos[p], tiempos[t]));
        }
    }
    return true;
}

static bool BarridoDeterminista(void)
{
    uint32_t semilla = 0x425033U;
    for (uint32_t i = 0U; i < 10000U; ++i) {
        semilla = semilla * 1664525U + 1013904223U;
        const uint32_t pulsos = semilla;
        semilla = semilla * 1664525U + 1013904223U;
        CHECK(VerificarDistancia(pulsos, semilla));
    }
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S3 ausencia inicial/cero valido", AusenciaInicial},
        {"S1 referencias", Referencias}, {"S1 medicion sin recorte", SinRecorteDeConsigna},
        {"S2 ventanas equivalentes", VentanasEquivalentes}, {"S2 redondeo/empate", Redondeo},
        {"S2 valor pequeno/cero valido", CeroRedondeadoValido},
        {"S4 duracion nula/recuperacion", DuracionNulaYRecuperacion},
        {"S5 rango/recuperacion", RangoYRecuperacion}, {"S5 extremos uint32", ExtremosDeTipos},
        {"S5 duracion minima", DuracionMinima}, {"S6 NULL", PunterosNulos},
        {"S3 reinicio", Reinicio}, {"S6 instancias", InstanciasIndependientes},
        {"S2/S5 matriz 100 fronteras", MatrizDeFronteras},
        {"S2/S5 barrido 10000 ventanas", BarridoDeterminista}
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
