/* Pruebas host de modos y alarma de Procesamiento (B18/I2); logica pura. */
#include "Procesamiento.h"
#include "Alarma.h"
#include <stdbool.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

#define MAN MANDOS_MODO_MANUAL
#define AUT MANDOS_MODO_AUTOMATICO

/* Debe ser el primer caso de alarma: observa el estado de arranque. */
static bool AlarmaS1_Histeresis(void)
{
    const struct { int32_t temperatura; bool activa; } pasos[] = {
        {4400, false}, {4500, false}, {4501, true}, {4500, true}, {4400, true}, {4301, true},
        {4300, false}, {4301, false}, {4500, false}, {9000, true}, {4300, false},
        {4501, true}, {-500, false}, {4501, true}, {0, false}
    };
    CHECK(TPI_ALARMA_UMBRAL_CENTESIMAS == 4500 && TPI_ALARMA_HISTERESIS_CENTESIMAS == 200);
    for (unsigned i = 0U; i < sizeof(pasos) / sizeof(pasos[0]); ++i) {
        CHECK(Procesamiento_EvaluarAlarma(pasos[i].temperatura) == pasos[i].activa);
    }
    return true;
}

static bool ModosS1_Arranque(void)
{
    CHECK(Procesamiento_InicializarModo(MAN) == PROCESAMIENTO_MANUAL);
    CHECK(Procesamiento_Consigna() == 0U);
    CHECK(Procesamiento_InicializarModo(AUT) == PROCESAMIENTO_AUTO_ESPERA);
    CHECK(Procesamiento_Consigna() == 0U);
    CHECK(Procesamiento_ActualizarModo(AUT, false, 0U) == PROCESAMIENTO_AUTO_ESPERA);
    return true;
}

static bool ModosS2_ConsignaActiva(void)
{
    CHECK(Procesamiento_InicializarModo(AUT) == PROCESAMIENTO_AUTO_ESPERA);
    CHECK(Procesamiento_ActualizarModo(AUT, false, 100U) == PROCESAMIENTO_AUTO_ESPERA);
    CHECK(Procesamiento_ActualizarModo(AUT, true, 100U) == PROCESAMIENTO_AUTO_ACTIVO);
    CHECK(Procesamiento_Consigna() == 100U);
    CHECK(Procesamiento_ActualizarModo(AUT, false, 50U) == PROCESAMIENTO_AUTO_ACTIVO);
    CHECK(Procesamiento_Consigna() == 100U);
    CHECK(Procesamiento_ActualizarModo(AUT, true, 0U) == PROCESAMIENTO_AUTO_ACTIVO);
    CHECK(Procesamiento_Consigna() == 0U);
    CHECK(Procesamiento_ActualizarModo(AUT, true, 185U) == PROCESAMIENTO_AUTO_ACTIVO);
    CHECK(Procesamiento_Consigna() == 185U);
    return true;
}

static bool ModosS3_ConsignaFueraDeRango(void)
{
    CHECK(Procesamiento_InicializarModo(AUT) == PROCESAMIENTO_AUTO_ESPERA);
    CHECK(Procesamiento_ActualizarModo(AUT, true, 186U) == PROCESAMIENTO_AUTO_ESPERA);
    CHECK(Procesamiento_ActualizarModo(AUT, true, UINT32_MAX) == PROCESAMIENTO_AUTO_ESPERA);
    CHECK(Procesamiento_Consigna() == 0U);
    CHECK(Procesamiento_ActualizarModo(AUT, true, 184U) == PROCESAMIENTO_AUTO_ACTIVO);
    CHECK(Procesamiento_ActualizarModo(AUT, true, 186U) == PROCESAMIENTO_AUTO_ACTIVO);
    CHECK(Procesamiento_Consigna() == 184U);
    return true;
}

static bool ModosS4_EntradaEnAutomatico(void)
{
    CHECK(Procesamiento_InicializarModo(MAN) == PROCESAMIENTO_MANUAL);
    CHECK(Procesamiento_ActualizarModo(MAN, true, 100U) == PROCESAMIENTO_MANUAL);
    CHECK(Procesamiento_Consigna() == 0U);
    CHECK(Procesamiento_ActualizarModo(AUT, true, 100U) == PROCESAMIENTO_AUTO_ESPERA);
    CHECK(Procesamiento_Consigna() == 0U);
    CHECK(Procesamiento_ActualizarModo(AUT, false, 100U) == PROCESAMIENTO_AUTO_ESPERA);
    CHECK(Procesamiento_ActualizarModo(AUT, true, 120U) == PROCESAMIENTO_AUTO_ACTIVO);
    CHECK(Procesamiento_Consigna() == 120U);
    CHECK(Procesamiento_ActualizarModo(MAN, false, 0U) == PROCESAMIENTO_MANUAL);
    CHECK(Procesamiento_Consigna() == 0U);
    CHECK(Procesamiento_ActualizarModo(AUT, false, 0U) == PROCESAMIENTO_AUTO_ESPERA);
    CHECK(Procesamiento_ActualizarModo(AUT, false, 0U) == PROCESAMIENTO_AUTO_ESPERA);
    CHECK(Procesamiento_Consigna() == 0U);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"alarma-temperatura S1 histeresis 45/43", AlarmaS1_Histeresis},
        {"modos S1 arranque", ModosS1_Arranque},
        {"modos S2 consigna activa", ModosS2_ConsignaActiva},
        {"modos S3 consigna fuera de rango", ModosS3_ConsignaFueraDeRango},
        {"modos S4 entrada en automatico", ModosS4_EntradaEnAutomatico}
    };
    for (unsigned i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%u/%u casos correctos\n", (unsigned)(sizeof(casos) / sizeof(casos[0])),
           (unsigned)(sizeof(casos) / sizeof(casos[0])));
    return 0;
}
