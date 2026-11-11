/* Pruebas host del regulador PI con las ganancias por defecto de Control.h. */
#include "Procesamiento.h"
#include "Control.h"
#include <stdbool.h>
#include <stdio.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return false; } } while (0)

/* Consigna cero borra la integral: deja el regulador como recien iniciado. */
static void Reiniciar(void)
{
    (void)Procesamiento_Regular(0U, 0U);
}

static bool PasoYEmpate(void)
{
    Reiniciar();
    CHECK(Procesamiento_Regular(100U, 0U) == 400U);
    CHECK(Procesamiento_Regular(100U, 0U) == 500U);
    Reiniciar();
    /* Error de 125 milesimas: P = 0,5 por mil, el empate sube a 1. */
    CHECK(Procesamiento_Regular(1U, 875U) == 1U);
    Reiniciar();
    CHECK(Procesamiento_Regular(1U, 876U) == 0U);
    return true;
}

static bool AntiWindup(void)
{
    Reiniciar();
    CHECK(Procesamiento_Regular(185U, 0U) == 740U);
    CHECK(Procesamiento_Regular(185U, 0U) == 925U);
    for (unsigned k = 0U; k < 40U; ++k) {
        CHECK(Procesamiento_Regular(185U, 0U) == 1000U);
    }
    /* Integral congelada en 370: 400 + 370. Sin anti-windup daria 1000. */
    CHECK(Procesamiento_Regular(100U, 0U) == 770U);
    CHECK(Procesamiento_Regular(100U, UINT32_MAX) == 0U);
    /* El paso saturado hacia abajo no integro: 400 + 470. */
    CHECK(Procesamiento_Regular(100U, 0U) == 870U);
    return true;
}

static bool CongelamientoInferiorEnCero(void)
{
    Reiniciar();
    for (unsigned k = 0U; k < 4U; ++k) {
        (void)Procesamiento_Regular(1U, 0U);
    }
    /* Integral 4; error -1000 milesimas da exactamente cero: no integra. */
    CHECK(Procesamiento_Regular(1U, 2000U) == 0U);
    CHECK(Procesamiento_Regular(1U, 1000U) == 4U);
    return true;
}

static bool ConsignaCeroReinicia(void)
{
    Reiniciar();
    (void)Procesamiento_Regular(100U, 0U);
    (void)Procesamiento_Regular(100U, 0U);
    CHECK(Procesamiento_Regular(0U, 0U) == 0U);
    CHECK(Procesamiento_Regular(100U, 0U) == 400U);
    return true;
}

static bool ModoBorraLaIntegral(void)
{
    Reiniciar();
    (void)Procesamiento_Regular(100U, 0U);
    (void)Procesamiento_ActualizarModo(MANDOS_MODO_MANUAL, false, 0U);
    CHECK(Procesamiento_Regular(100U, 0U) == 400U);
    (void)Procesamiento_InicializarModo(MANDOS_MODO_AUTOMATICO);
    CHECK(Procesamiento_Regular(100U, 0U) == 400U);
    (void)Procesamiento_ActualizarModo(MANDOS_MODO_AUTOMATICO, true, 100U);
    CHECK(Procesamiento_Regular(100U, 0U) == 500U);
    return true;
}

static bool Extremos(void)
{
    const uint32_t velocidades[] = {0U, 1U, 184999U, 185000U, 185001U, UINT32_MAX};
    const uint32_t consignas[] = {1U, 185U, 186U, UINT32_MAX};
    Reiniciar();
    for (unsigned c = 0U; c < sizeof(consignas) / sizeof(consignas[0]); ++c) {
        for (unsigned v = 0U; v < sizeof(velocidades) / sizeof(velocidades[0]); ++v) {
            CHECK(Procesamiento_Regular(consignas[c], velocidades[v]) <= 1000U);
        }
    }
    return true;
}

int main(void)
{
    static const struct { const char *nombre; bool (*test)(void); } casos[] = {
        {"S1 paso y empate", PasoYEmpate}, {"S2 anti-windup", AntiWindup},
        {"S2 saturacion inferior exacta", CongelamientoInferiorEnCero},
        {"S3 consigna cero reinicia", ConsignaCeroReinicia},
        {"S3 el modo borra la integral", ModoBorraLaIntegral},
        {"S4 extremos de 32 bits", Extremos}
    };
    if (TPI_CONTROL_KP_NUM != 4U || TPI_CONTROL_KP_DEN != 1000U ||
        TPI_CONTROL_KI_NUM != 1U || TPI_CONTROL_KI_DEN != 1000U) {
        fprintf(stderr, "Estas pruebas valen solo para las ganancias por defecto\n");
        return 1;
    }
    for (unsigned i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].test()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%u/%u casos correctos\n", (unsigned)(sizeof(casos) / sizeof(casos[0])),
           (unsigned)(sizeof(casos) / sizeof(casos[0])));
    return 0;
}
