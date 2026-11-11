/*
 * Composicion real Temperatura + LM35 (B18/I2): el servicio promedia lo que el
 * dispositivo convierte desde la palabra del ADC. Se compila con ADC_VREF_MV=2000.
 */
#include "Temperatura.h"
#include "Adquisicion_ADC_DMA.h"
#include <stdbool.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static bool S3_ServicioSobreLm35Real(void)
{
    ADC_DMA_LM35 = UINT32_C(0xC700200F);
    CHECK(Temperatura_Leer() == 2500 && ADC_DMA_LM35 == UINT32_C(0xC700200F));
    ADC_DMA_LM35 = UINT32_C(0xC300800F);
    CHECK(Temperatura_Leer() == 6250);
    CHECK(Temperatura_Leer() == 7500);
    CHECK(Temperatura_Leer() == 8125);
    CHECK(Temperatura_Leer() == 10000 && ADC_DMA_LM35 == UINT32_C(0xC300800F));
    return true;
}

int main(void)
{
    if (!S3_ServicioSobreLm35Real()) { fprintf(stderr, "FAIL S3 servicio sobre LM35 real\n"); return 1; }
    printf("PASS S3 servicio sobre LM35 real\n1/1 casos correctos\n");
    return 0;
}
