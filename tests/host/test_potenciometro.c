#include "Potenciometro.h"
#include "Adquisicion_ADC_DMA.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static bool LecturasDeReferencia(void)
{
    const struct { uint32_t palabra; uint16_t muestra; } casos[] = {
        {UINT32_C(0x00000000), 0U},
        {UINT32_C(0x00008000), 2048U},
        {UINT32_C(0x0000FFF0), 4095U}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        ADC_DMA_Pot = casos[i].palabra;
        CHECK(Pot_Read() == casos[i].muestra);
        CHECK(ADC_DMA_Pot == casos[i].palabra);
    }
    return true;
}

static bool ActualizacionYRepeticion(void)
{
    const struct { uint32_t palabra; uint16_t muestra; } pasos[] = {
        {UINT32_C(0x00000100), 16U},
        {UINT32_C(0x0000C000), 3072U},
        {UINT32_C(0x00000100), 16U},
        {UINT32_C(0x00000000), 0U}
    };
    for (size_t i = 0U; i < sizeof(pasos) / sizeof(pasos[0]); ++i) {
        ADC_DMA_Pot = pasos[i].palabra;
        CHECK(Pot_Read() == pasos[i].muestra);
        CHECK(Pot_Read() == pasos[i].muestra);
        CHECK(ADC_DMA_Pot == pasos[i].palabra);
    }
    return true;
}

static bool SoloResultadoSinEscritura(void)
{
    const struct { uint32_t palabra; uint16_t muestra; } casos[] = {
        {UINT32_C(0x80001230), 291U},
        {UINT32_C(0x4000ABC0), 2748U},
        {UINT32_C(0xC0000010), 1U}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        ADC_DMA_Pot = casos[i].palabra;
        CHECK(Pot_Read() == casos[i].muestra);
        CHECK(Pot_Read() == casos[i].muestra);
        CHECK(ADC_DMA_Pot == casos[i].palabra);
    }
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S1 muestras 0/2048/4095", LecturasDeReferencia},
        {"S2 actualizacion/repeticion", ActualizacionYRepeticion},
        {"S3 solo RESULT/sin escritura", SoloResultadoSinEscritura}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
