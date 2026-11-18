/* Pruebas host del servicio Medicion_Velocidad (B18/I2) con dispositivos simulados. */
#include "Medicion_Velocidad.h"
#include "fakes_dispositivos.h"

static bool S1_InicioEnOrden(void)
{
    static const char *const orden[] = {"SensorOptico_Init", "Timer_Init", "Timer_Start", NULL};
    Fakes_Reiniciar();
    Medicion_Velocidad_Init();
    CHECK(Fakes_LlamadasSon(orden));
    return true;
}

static bool S2_PulsosAMilesimas(void)
{
    const struct { uint32_t pulsos; uint32_t rpm_milesimas; } casos[] = {
        {0U, 0U}, {1U, 2000U}, {50U, 100000U}, {92U, 184000U}, {93U, 186000U},
        {2147483U, UINT32_C(4294966000)}, {2147484U, UINT32_MAX}, {UINT32_MAX, UINT32_MAX}
    };
    Fakes_Reiniciar();
    for (unsigned i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        uint32_t rpm = 7U;
        fakes.hay_ventana = true;
        fakes.pulsos = casos[i].pulsos;
        CHECK(Medicion_Velocidad_Actualizar(&rpm) && rpm == casos[i].rpm_milesimas);
    }
    return true;
}

static bool S3_UnaVezPorVentana(void)
{
    uint32_t rpm = 7U;
    Fakes_Reiniciar();
    fakes.pulsos = 50U;
    CHECK(!Medicion_Velocidad_Actualizar(&rpm) && rpm == 7U);
    fakes.hay_ventana = true;
    CHECK(Medicion_Velocidad_Actualizar(&rpm) && rpm == 100000U);
    rpm = 7U;
    CHECK(!Medicion_Velocidad_Actualizar(&rpm) && rpm == 7U);
    return true;
}

int main(void)
{
    const Caso casos[] = {
        {"S1 inicio en orden", S1_InicioEnOrden},
        {"S2 pulsos a milesimas de RPM", S2_PulsosAMilesimas},
        {"S3 una medicion por ventana", S3_UnaVezPorVentana}
    };
    EJECUTAR_CASOS(casos);
}
