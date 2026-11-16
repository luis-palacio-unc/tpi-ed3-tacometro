/* El fixture escribe la muestra mediante extern como productor de prueba.
 * Enlaza solo con LM335.c, sin Adquisicion_ADC_DMA.c ni funciones del periferico. */
#include "LM335.h"
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

/* Centesimas de kelvin del modelo nominal -> centesimas Celsius informadas. */
#define CELSIUS(kelvin_centesimas) ((kelvin_centesimas) - 27315 + (LM335_AJUSTE_CENTESIMAS))

typedef struct {
    uint32_t referencia;
    int32_t codigo512, codigo1024, codigo2048, codigo128;
} Referencia;

static const Referencia referencias[] = {
    {1000U, CELSIUS(1250), CELSIUS(2500), CELSIUS(5000), CELSIUS(313)},
    {2000U, CELSIUS(2500), CELSIUS(5000), CELSIUS(10000), CELSIUS(625)},
    {2540U, CELSIUS(3175), CELSIUS(6350), CELSIUS(12700), CELSIUS(794)},
    {3300U, CELSIUS(4125), CELSIUS(8250), CELSIUS(16500), CELSIUS(1031)},
    {5000U, CELSIUS(6250), CELSIUS(12500), CELSIUS(25000), CELSIUS(1563)}
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
        {UINT32_C(0x00000000), CELSIUS(0)},
        {UINT32_C(0x00002000), ref->codigo512},
        {UINT32_C(0x00008000), ref->codigo2048}
    };
    const uint32_t campos[] = {0U, UINT32_C(0x80000000), UINT32_C(0x40000000),
        UINT32_C(0x07000000), UINT32_C(0x0000000F), UINT32_C(0xC700000F),
        UINT32_C(0x00FF0000), UINT32_C(0x38000000), UINT32_C(0xFFFF000F)};
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        for (size_t j = 0U; j < sizeof(campos) / sizeof(campos[0]); ++j) {
            const uint32_t palabra = casos[i].palabra | campos[j];
            ADC_DMA_LM335 = palabra;
            CHECK(LM335_Read() == casos[i].esperado);
            CHECK(ADC_DMA_LM335 == palabra);
        }
    }
    return true;
}

static bool EscalaConfigurada(void)
{
    const Referencia *ref = ReferenciaActual();
    CHECK(ref != NULL);
    ADC_DMA_LM335 = UINT32_C(0x00004000);
    CHECK(LM335_Read() == ref->codigo1024);
    CHECK(ADC_DMA_LM335 == UINT32_C(0x00004000));
    CHECK(LM335_CENTESIMAS_POR_GRADO == 100U);
    return true;
}

static bool Redondeo(void)
{
    const Referencia *ref = ReferenciaActual();
    CHECK(ref != NULL);
    ADC_DMA_LM335 = UINT32_C(0x00000800);
    CHECK(LM335_Read() == ref->codigo128);
    CHECK(ADC_DMA_LM335 == UINT32_C(0x00000800));
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
        {UINT32_C(0xC700000F), CELSIUS(0)}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        ADC_DMA_LM335 = casos[i].palabra;
        CHECK(LM335_Read() == casos[i].esperado);
        CHECK(LM335_Read() == casos[i].esperado);
        CHECK(ADC_DMA_LM335 == casos[i].palabra);
    }
    return true;
}

/* Valores literales del montaje real: referencia 3300 mV, sin ajuste. */
static bool PuntosFisicos(void)
{
#if ADC_VREF_MV == 3300 && LM335_AJUSTE_CENTESIMAS == 0
    const struct {uint32_t codigo; int32_t esperado;} casos[] = {
        {0U, -27315},     /* 0 V: sensor sin polarizar o en corto. */
        {3390U, -3},      /* 2,731 V: 0 grados. */
        {3701U, 2503},    /* 2,982 V: ambiente. */
        {3949U, 4501},    /* 3,182 V: umbral de alarma. */
        {4095U, 5677}     /* Fondo de escala. */
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        ADC_DMA_LM335 = casos[i].codigo << 4U;
        CHECK(LM335_Read() == casos[i].esperado);
    }
#else
    ADC_DMA_LM335 = 0U;
    CHECK(LM335_Read() == -27315 + (LM335_AJUSTE_CENTESIMAS));
#endif
    return true;
}

static int EmitirTabla(void)
{
    printf("VREF %" PRIu32 "\n", (uint32_t)ADC_VREF_MV);
    for (uint32_t codigo = 0U; codigo < 4096U; ++codigo) {
        const uint32_t palabra = (codigo << 4U) | UINT32_C(0xFFFF000F);
        ADC_DMA_LM335 = palabra;
        const int32_t temperatura = LM335_Read();
        if (ADC_DMA_LM335 != palabra) {
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
        {"S6 actualizacion/repeticion", ActualizacionYRepeticion},
        {"S12 kelvin a Celsius y ajuste", PuntosFisicos}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
