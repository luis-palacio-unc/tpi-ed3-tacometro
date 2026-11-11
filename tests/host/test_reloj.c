/*
 * Prueba host de Dispositivos/soporte/Reloj.c (B18/I1).
 * Incluye el .c bajo prueba, redirige LPC_SC a un modelo en RAM y reemplaza
 * SystemInit/SystemCoreClockUpdate por dobles que registran orden y FLASHCFG.
 * No configura ningun reloj real.
 */
#include "LPC17xx.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static LPC_SC_TypeDef sc_modelo;
#undef LPC_SC
#define LPC_SC (&sc_modelo)

#include "../../Dispositivos/soporte/Reloj.c"

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static unsigned orden, orden_init, orden_update;
static uint32_t flashcfg_en_init, flashcfg_en_update;

void SystemInit(void)
{
    orden_init = ++orden;
    flashcfg_en_init = sc_modelo.FLASHCFG;
    sc_modelo.PLL0CON = 3U;
}

void SystemCoreClockUpdate(void)
{
    orden_update = ++orden;
    flashcfg_en_update = sc_modelo.FLASHCFG;
}

static bool Caso(uint32_t inicial, uint32_t esperado)
{
    memset(&sc_modelo, 0, sizeof(sc_modelo));
    orden = 0U; orden_init = 0U; orden_update = 0U;
    flashcfg_en_init = UINT32_MAX; flashcfg_en_update = UINT32_MAX;
    sc_modelo.FLASHCFG = inicial;
    Reloj_Init();
    CHECK(orden == 2U && orden_init == 1U && orden_update == 2U);
    CHECK(flashcfg_en_init == esperado && flashcfg_en_update == esperado);
    CHECK(sc_modelo.FLASHCFG == esperado && ((esperado >> 12) & 0xFU) == 4U);
    return true;
}

int main(void)
{
    const struct { uint32_t inicial, esperado; } casos[] = {
        {UINT32_C(0x0000303A), UINT32_C(0x0000403A)},
        {UINT32_C(0x0000003A), UINT32_C(0x0000403A)},
        {UINT32_C(0x0000F03A), UINT32_C(0x0000403A)},
        {UINT32_C(0x0000403A), UINT32_C(0x0000403A)},
        {UINT32_C(0xFFFFFFFF), UINT32_C(0xFFFF4FFF)},
        {UINT32_C(0x00000000), UINT32_C(0x00004000)}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!Caso(casos[i].inicial, casos[i].esperado)) { fprintf(stderr, "FAIL caso %zu\n", i); return 1; }
    }
    puts("PASS I1 Reloj_Init: FLASHTIM=4 antes de SystemInit y de SystemCoreClockUpdate, 6 valores iniciales");
    return 0;
}
