/* El fixture escribe la muestra mediante extern como productor de prueba.
 * Enlaza solo con LM35.c, sin Adquisicion_ADC_DMA.c ni funciones del periferico. */
#include "LM35.h"
#include "Analogica.h"
#include "Adquisicion_ADC_DMA.h"
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

typedef struct {
    uint32_t referencia;
    int32_t codigo512, codigo1024, codigo2048, codigo128;
} Referencia;

static const Referencia referencias[] = {
    {1000U, 1250, 2500, 5000, 313},
    {2000U, 2500, 5000, 10000, 625},
    {2540U, 3175, 6350, 12700, 794},
    {3300U, 4125, 8250, 16500, 1031},
    {5000U, 6250, 12500, 25000, 1563}
};

static const Referencia *ReferenciaActual(void)
{
    for (size_t i = 0U; i < sizeof(referencias) / sizeof(referencias[0]); ++i) {
        if (referencias[i].referencia == (uint32_t)ADC_VREF_MV) {
            return &referencias[i];
        }
    }
    return NULL;
}

static bool ValoresYCampos(void)
{
    const Referencia *ref = ReferenciaActual();
    CHECK(ref != NULL);
    const struct {uint32_t palabra; int32_t esperado;} casos[] = {
        {UINT32_C(0x00000000), 0},
        {UINT32_C(0x00002000), ref->codigo512},
        {UINT32_C(0x00008000), ref->codigo2048}
    };
    const uint32_t campos[] = {0U, UINT32_C(0x80000000), UINT32_C(0x40000000),
        UINT32_C(0x07000000), UINT32_C(0x0000000F), UINT32_C(0xC700000F)};
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        for (size_t j = 0U; j < sizeof(campos) / sizeof(campos[0]); ++j) {
            const uint32_t palabra = casos[i].palabra | campos[j];
            ADC_DMA_LM35 = palabra;
            CHECK(LM35_Read() == casos[i].esperado);
            CHECK(ADC_DMA_LM35 == palabra);
        }
    }
    return true;
}

static bool EscalaConfigurada(void)
{
    const Referencia *ref = ReferenciaActual();
    CHECK(ref != NULL);
    ADC_DMA_LM35 = UINT32_C(0x00004000);
    CHECK(LM35_Read() == ref->codigo1024);
    CHECK(ADC_DMA_LM35 == UINT32_C(0x00004000));
    CHECK(LM35_CENTESIMAS_POR_GRADO == 100U);
    return true;
}

static bool Redondeo(void)
{
    const Referencia *ref = ReferenciaActual();
    CHECK(ref != NULL);
    ADC_DMA_LM35 = UINT32_C(0x00000800);
    CHECK(LM35_Read() == ref->codigo128);
    CHECK(ADC_DMA_LM35 == UINT32_C(0x00000800));
    return true;
}

static bool ActualizacionYRepeticion(void)
{
    const Referencia *ref = ReferenciaActual();
    CHECK(ref != NULL);
    const struct {uint32_t palabra; int32_t esperado;} casos[] = {
        {UINT32_C(0xC700200F), ref->codigo512},
        {UINT32_C(0xC300800F), ref->codigo2048},
        {UINT32_C(0x00008000), ref->codigo2048},
        {UINT32_C(0xC700000F), 0}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        ADC_DMA_LM35 = casos[i].palabra;
        CHECK(LM35_Read() == casos[i].esperado);
        CHECK(LM35_Read() == casos[i].esperado);
        CHECK(ADC_DMA_LM35 == casos[i].palabra);
    }
    return true;
}

static int EmitirTabla(void)
{
    printf("VREF %" PRIu32 "\n", (uint32_t)ADC_VREF_MV);
    for (uint32_t codigo = 0U; codigo < 4096U; ++codigo) {
        const uint32_t palabra = (codigo << 4U) | UINT32_C(0xC700000F);
        ADC_DMA_LM35 = palabra;
        const int32_t temperatura = LM35_Read();
        if (ADC_DMA_LM35 != palabra) {
            return 1;
        }
        printf("%" PRIu32 " %" PRId32 "\n", codigo, temperatura);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--tabla") == 0) {
        return EmitirTabla();
    }
    if (argc != 1) {
        return 2;
    }
    const struct {const char *nombre; bool (*ejecutar)(void);} casos[] = {
        {"S1 RESULT y campos ajenos", ValoresYCampos},
        {"S2 referencia configurada", EscalaConfigurada},
        {"S3 redondeo B05", Redondeo},
        {"S6 actualizacion/repeticion", ActualizacionYRepeticion}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
