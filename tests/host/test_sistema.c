/* Pruebas host del servicio Sistema (B18/I2) con dispositivos simulados. */
#include "Sistema.h"
#include "fakes_dispositivos.h"

static const char *const orden[] = {"Reloj_Init", "Adquisicion_ADC_DMA_Init", NULL};

static bool S1_RelojAdcYPrimerPar(void)
{
    Fakes_Reiniciar();
    fakes.adc_consultas_sin_datos = 1000U;
    CHECK(Sistema_Init());
    CHECK(Fakes_LlamadasSon(orden) && fakes.adc_consultas == 1001U);
    Fakes_Reiniciar();
    CHECK(Sistema_Init() && fakes.adc_consultas == 1U);
    return true;
}

static bool S2_DmaRechazado(void)
{
    Fakes_Reiniciar();
    fakes.adc_init_ok = false;
    CHECK(!Sistema_Init());
    CHECK(Fakes_LlamadasSon(orden) && fakes.adc_consultas == 0U);
    return true;
}

static bool S2_EsperaAcotada(void)
{
    Fakes_Reiniciar();
    fakes.adc_consultas_sin_datos = UINT32_MAX;
    CHECK(!Sistema_Init());
    CHECK(fakes.adc_consultas == 2000000U);
    Fakes_Reiniciar();
    fakes.adc_consultas_sin_datos = 1999999U;
    CHECK(Sistema_Init() && fakes.adc_consultas == 2000000U);
    return true;
}

int main(void)
{
    const Caso casos[] = {
        {"S1 reloj, ADC/DMA y primer par", S1_RelojAdcYPrimerPar},
        {"S2 DMA rechazado", S2_DmaRechazado},
        {"S2 espera acotada", S2_EsperaAcotada}
    };
    EJECUTAR_CASOS(casos);
}
