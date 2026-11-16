/*
 * Composicion real Temperatura + LM335 (B18/I2): el servicio promedia lo que el
 * dispositivo convierte desde la palabra del ADC. Se compila con ADC_VREF_MV=3300:
 * codigo 3701 = 25,03 grados y codigo 3949 = 45,01 grados.
 */
#include "Temperatura.h"
#include "Adquisicion_ADC_DMA.h"
#include <stdbool.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static bool S3_ServicioSobreLm335Real(void)
{
    ADC_DMA_LM335 = UINT32_C(0xC700E75F);
    CHECK(Temperatura_Leer() == 2503 && ADC_DMA_LM335 == UINT32_C(0xC700E75F));
    ADC_DMA_LM335 = UINT32_C(0xC300F6DF);
    CHECK(Temperatura_Leer() == 3502);
    CHECK(Temperatura_Leer() == 3835);
    CHECK(Temperatura_Leer() == 4001);
    CHECK(Temperatura_Leer() == 4501 && ADC_DMA_LM335 == UINT32_C(0xC300F6DF));
    return true;
}

int main(void)
{
    if (!S3_ServicioSobreLm335Real()) { fprintf(stderr, "FAIL S3 servicio sobre LM335 real\n"); return 1; }
    printf("PASS S3 servicio sobre LM335 real\n1/1 casos correctos\n");
    return 0;
}
