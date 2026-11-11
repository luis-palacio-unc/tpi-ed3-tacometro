#include "LM35.h"
#include "Temperatura.h"
#include "Adquisicion_ADC_DMA.h"
#include <stdio.h>
#include <stddef.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static bool PropagacionDirecta(void)
{
    Temperatura_Lectura servicio;
    CHECK(Temperatura_Inicializar(&servicio));
    ADC_DMA_LM35 = UINT32_C(0xC700200F);
    CHECK(Temperatura_Actualizar(&servicio, LM35_Read(), TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(servicio.centesimas_c == 2500 && ADC_DMA_LM35 == UINT32_C(0xC700200F));
    ADC_DMA_LM35 = UINT32_C(0xC300800F);
    CHECK(Temperatura_Actualizar(&servicio, LM35_Read(), TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(servicio.centesimas_c == 10000 && ADC_DMA_LM35 == UINT32_C(0xC300800F));
    return true;
}

static bool FuentesIndependientes(void)
{
    Temperatura_Lectura sensor, otra;
    ADC_DMA_LM35 = UINT32_C(0xC700200F);
    CHECK(Temperatura_Actualizar(&sensor, LM35_Read(), TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Actualizar(&otra, -725, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(otra.centesimas_c == -725 && sensor.centesimas_c == 2500);
    CHECK(Temperatura_Actualizar(&otra, 0, TEMPERATURA_FUENTE_ERROR) == TEMPERATURA_ERROR_FUENTE);
    CHECK(sensor.estado == TEMPERATURA_VALIDA && sensor.centesimas_c == 2500);
    CHECK(ADC_DMA_LM35 == UINT32_C(0xC700200F));
    return true;
}

static bool DisponibilidadExternaYCero(void)
{
    Temperatura_Lectura servicio;
    CHECK(Temperatura_Inicializar(&servicio));
    CHECK(servicio.estado == TEMPERATURA_SIN_DATOS);
    CHECK(Temperatura_Actualizar(&servicio, 0, TEMPERATURA_FUENTE_SIN_DATOS) == TEMPERATURA_SIN_DATOS);
    ADC_DMA_LM35 = 0U;
    CHECK(Temperatura_Actualizar(&servicio, LM35_Read(), TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(servicio.centesimas_c == 0 && ADC_DMA_LM35 == 0U);
    return true;
}

int main(void)
{
    const struct {const char *nombre; bool (*ejecutar)(void);} casos[] = {
        {"S7 centesimas directas", PropagacionDirecta},
        {"S7 fuentes independientes", FuentesIndependientes},
        {"TE2 disponibilidad externa/cero", DisponibilidadExternaYCero}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
