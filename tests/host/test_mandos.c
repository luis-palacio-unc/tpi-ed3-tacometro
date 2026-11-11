/* Pruebas host del servicio Mandos (B18/I2) con dispositivos simulados. */
#include "Mandos.h"
#include "fakes_dispositivos.h"

static bool S1_PerillaAPorMil(void)
{
    const struct { uint16_t pot; uint16_t mando; } casos[] = {
        {0U, 0U}, {1U, 0U}, {2U, 0U}, {3U, 1U}, {2047U, 500U}, {2048U, 500U}, {2050U, 501U},
        {4092U, 999U}, {4093U, 1000U}, {4095U, 1000U}
    };
    Fakes_Reiniciar();
    for (unsigned i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        fakes.pot = casos[i].pot;
        CHECK(Mandos_LeerManual() == casos[i].mando);
    }
    uint16_t anterior = 0U;
    for (uint16_t pot = 0U; pot <= 4095U; ++pot) {
        fakes.pot = pot;
        const uint16_t mando = Mandos_LeerManual();
        CHECK(mando >= anterior && mando <= anterior + 1U && mando <= 1000U);
        anterior = mando;
    }
    return true;
}

static bool S2_LlaveAModo(void)
{
    static const char *const orden[] = {"Selector_Init", NULL};
    Fakes_Reiniciar();
    Mandos_Init();
    CHECK(Fakes_LlamadasSon(orden));
    fakes.selector = true;
    CHECK(Mandos_LeerModo() == MANDOS_MODO_MANUAL);
    fakes.selector = false;
    CHECK(Mandos_LeerModo() == MANDOS_MODO_AUTOMATICO);
    return true;
}

int main(void)
{
    const Caso casos[] = {
        {"S1 perilla a por mil", S1_PerillaAPorMil},
        {"S2 llave a modo", S2_LlaveAModo}
    };
    EJECUTAR_CASOS(casos);
}
