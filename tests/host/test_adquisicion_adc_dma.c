/*
 * Pruebas host de Dispositivos/soporte/Adquisicion_ADC_DMA.c (B18/I1).
 * Incluye el .c bajo prueba, redirige LPC_ADC y LPC_GPDMACH0 a modelos en RAM y
 * reemplaza por dobles las funciones de la biblioteca original y del NVIC.
 * Las direcciones se comparan truncadas a 32 bits, como las entrega el soporte.
 * No ejecuta ADC ni DMA reales.
 */
#include "dobles_cmsis.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"
#include "LM35.h"
#include "Potenciometro.h"
#include <stddef.h>
#include <stdio.h>

static LPC_ADC_TypeDef adc_modelo;
static LPC_GPDMACH_TypeDef canal0_modelo;
#undef LPC_ADC
#define LPC_ADC (&adc_modelo)
#undef LPC_GPDMACH0
#define LPC_GPDMACH0 (&canal0_modelo)

#include "../../Dispositivos/soporte/Adquisicion_ADC_DMA.c"

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)
#define DIR32(objeto) ((uint32_t)(uintptr_t)&(objeto))
#define CONTROL_SIN_IRQ UINT32_C(0x00480001)
#define CONTROL_CON_IRQ UINT32_C(0x80480001)
#define EVENTOS_MAX 32U

typedef enum {
    EV_ADC_INIT, EV_PIN0, EV_PIN1, EV_CANAL0, EV_CANAL1, EV_INT_GLOBAL_OFF, EV_INT0, EV_INT1,
    EV_DMA_INIT, EV_DMA_SETUP, EV_DMA_START, EV_RAFAGA
} Evento;

static Evento eventos[EVENTOS_MAX];
static unsigned eventos_total, limpiezas_tc, consultas_tc;
static uint32_t tasa_vista;
static GPDMA_Channel_CFG_T canal_visto;
static GPDMA_LLI_T lista_vista[2];
static Status resultado_setup;
static bool tc_activo, error_modelo, irq_adc_en_setup, irq_dma_en_arranque;
static bool nvic_adc_en_rafaga, registros_cargados_en_arranque;

static void Registrar(Evento evento)
{
    if (eventos_total < EVENTOS_MAX) { eventos[eventos_total] = evento; }
    ++eventos_total;
}

void ADC_Init(uint32_t tasa) { tasa_vista = tasa; Registrar(EV_ADC_INIT); }

void ADC_PinConfig(ADC_CHANNEL canal)
{
    if (canal == ADC_CHANNEL_0) { Registrar(EV_PIN0); }
    else if (canal == ADC_CHANNEL_1) { Registrar(EV_PIN1); }
    else { error_modelo = true; }
}

void ADC_ChannelEnable(ADC_CHANNEL canal)
{
    if (canal == ADC_CHANNEL_0) { Registrar(EV_CANAL0); }
    else if (canal == ADC_CHANNEL_1) { Registrar(EV_CANAL1); }
    else { error_modelo = true; }
}

void ADC_IntDisable(ADC_INT_SOURCE fuente)
{
    if (fuente == ADC_INT_GLOBAL) { Registrar(EV_INT_GLOBAL_OFF); } else { error_modelo = true; }
}

void ADC_IntEnable(ADC_INT_SOURCE fuente)
{
    if (fuente == ADC_INT_CH0) { Registrar(EV_INT0); }
    else if (fuente == ADC_INT_CH1) { Registrar(EV_INT1); }
    else { error_modelo = true; }
}

void ADC_BurstEnable(void)
{
    nvic_adc_en_rafaga = nvic.habilitada[ADC_IRQn];
    Registrar(EV_RAFAGA);
}

void GPDMA_Init(void) { Registrar(EV_DMA_INIT); }

Status GPDMA_SetupChannel(const GPDMA_Channel_CFG_T *cfg)
{
    if (cfg == NULL) { error_modelo = true; return ERROR; }
    canal_visto = *cfg;
    memcpy(lista_vista, Adquisicion_Lista, sizeof(lista_vista));
    irq_adc_en_setup = nvic.habilitada[ADC_IRQn] || nvic.habilitada[DMA_IRQn];
    Registrar(EV_DMA_SETUP);
    if (resultado_setup == SUCCESS) {
        canal0_modelo.DMACCSrcAddr = DIR32(adc_modelo.ADGDR);
        canal0_modelo.DMACCDestAddr = cfg->dstMemAddr;
        canal0_modelo.DMACCLLI = cfg->linkedList;
        canal0_modelo.DMACCControl = CONTROL_CON_IRQ;
    }
    return resultado_setup;
}

void GPDMA_ChannelStart(GPDMA_CH canal)
{
    if (canal != GPDMA_CH_0) { error_modelo = true; return; }
    irq_dma_en_arranque = nvic.habilitada[DMA_IRQn] && nvic.prioridad[DMA_IRQn] == 3U;
    registros_cargados_en_arranque = canal0_modelo.DMACCSrcAddr == DIR32(adc_modelo.ADDR0) &&
                                     canal0_modelo.DMACCControl == CONTROL_SIN_IRQ;
    Registrar(EV_DMA_START);
}

IntStatus GPDMA_IntGetStatus(GPDMA_STATUS_TYPE tipo, GPDMA_CH canal)
{
    if (tipo != GPDMA_INTTC || canal != GPDMA_CH_0) { error_modelo = true; return RESET; }
    ++consultas_tc;
    return tc_activo ? SET : RESET;
}

void GPDMA_ClearIntPending(GPDMA_CLEAR_INT tipo, GPDMA_CH canal)
{
    if (tipo != GPDMA_CLR_INTTC || canal != GPDMA_CH_0) { error_modelo = true; return; }
    tc_activo = false; ++limpiezas_tc;
}

static bool Preparar(Status resultado)
{
    NvicReiniciar();
    nvic.habilitada[ADC_IRQn] = true; nvic.habilitada[DMA_IRQn] = true;
    memset(&canal0_modelo, 0xFF, sizeof(canal0_modelo));
    memset(&canal_visto, 0xFF, sizeof(canal_visto)); memset(lista_vista, 0xFF, sizeof(lista_vista));
    eventos_total = 0U; limpiezas_tc = 0U; consultas_tc = 0U; tasa_vista = 0U;
    tc_activo = false; error_modelo = false; irq_adc_en_setup = true;
    irq_dma_en_arranque = false; nvic_adc_en_rafaga = true; registros_cargados_en_arranque = false;
    resultado_setup = resultado;
    ADC_DMA_Pot = 0U; ADC_DMA_LM35 = 0U;
    return Adquisicion_ADC_DMA_Init();
}

static bool SecuenciaAcordada(void)
{
    CHECK(Preparar(SUCCESS));
    const Evento esperado[] = {EV_ADC_INIT, EV_PIN0, EV_PIN1, EV_CANAL0, EV_CANAL1,
        EV_INT_GLOBAL_OFF, EV_INT0, EV_INT1, EV_DMA_INIT, EV_DMA_SETUP, EV_DMA_START, EV_RAFAGA};
    CHECK(eventos_total == sizeof(esperado) / sizeof(esperado[0]));
    CHECK(memcmp(eventos, esperado, sizeof(esperado)) == 0);
    CHECK(!irq_adc_en_setup && irq_dma_en_arranque && registros_cargados_en_arranque);
    CHECK(!nvic.habilitada[ADC_IRQn] && !nvic_adc_en_rafaga);
    CHECK(nvic.habilitada[DMA_IRQn] && nvic.prioridad[DMA_IRQn] == 3U);
    CHECK(nvic.habilitaciones == 1U && nvic.prioridades == 1U && nvic.pedidos == 0U);
    CHECK(!Adquisicion_ADC_DMA_HayDatos() && Adquisicion_ADC_DMA_ContarPares() == 0U);
    CHECK(!nvic.error && !error_modelo);
    return true;
}

static bool RelojDelAdcDentroDeLimite(void)
{
    CHECK(Preparar(SUCCESS));
    const uint32_t pclk = UINT32_C(25000000), divisor = tasa_vista * 65U;
    uint32_t clkdiv = (pclk + divisor / 2U) / divisor - 1U;
    clkdiv = clkdiv > 255U ? 255U : clkdiv;
    const uint32_t reloj_adc = pclk / (clkdiv + 1U);
    CHECK(tasa_vista == 1500U && tasa_vista > 0U && tasa_vista <= 200000U);
    CHECK(clkdiv == 255U && reloj_adc == 97656U && reloj_adc <= UINT32_C(13000000));
    CHECK(reloj_adc / 64U == 1525U && reloj_adc / 64U / 2U == 762U);
    return true;
}

static bool CanalYListaEnlazada(void)
{
    CHECK(Preparar(SUCCESS));
    CHECK(canal_visto.channelNum == GPDMA_CH_0 && canal_visto.transferSize == 1U);
    CHECK(canal_visto.type == GPDMA_P2M && canal_visto.srcConn == GPDMA_ADC);
    CHECK(canal_visto.dstMemAddr == DIR32(ADC_DMA_Pot));
    CHECK(canal_visto.src.width == GPDMA_WORD && canal_visto.dst.width == GPDMA_WORD);
    CHECK(canal_visto.src.burst == GPDMA_BSIZE_1 && canal_visto.dst.burst == GPDMA_BSIZE_1);
    CHECK(canal_visto.src.increment == DISABLE && canal_visto.dst.increment == DISABLE);
    CHECK(canal_visto.intTC == ENABLE && canal_visto.intErr == DISABLE);
    CHECK(canal_visto.linkedList == DIR32(Adquisicion_Lista[1]));
    CHECK(lista_vista[0].srcAddr == DIR32(adc_modelo.ADDR0));
    CHECK(lista_vista[0].dstAddr == DIR32(ADC_DMA_Pot));
    CHECK(lista_vista[0].nextLLI == DIR32(Adquisicion_Lista[1]));
    CHECK(lista_vista[0].control == CONTROL_SIN_IRQ);
    CHECK(lista_vista[1].srcAddr == DIR32(adc_modelo.ADDR1));
    CHECK(lista_vista[1].dstAddr == DIR32(ADC_DMA_LM35));
    CHECK(lista_vista[1].nextLLI == DIR32(Adquisicion_Lista[0]));
    CHECK(lista_vista[1].control == CONTROL_CON_IRQ);
    CHECK(memcmp(lista_vista, Adquisicion_Lista, sizeof(lista_vista)) == 0);
    CHECK(ADQUISICION_DMA_CONTROL == CONTROL_SIN_IRQ);
    CHECK(GPDMA_DMACCxControl_TransferSize(1U) == 1U && ((CONTROL_SIN_IRQ >> 12) & 0x3FU) == 0U);
    CHECK(((CONTROL_SIN_IRQ >> 18) & 7U) == 2U && ((CONTROL_SIN_IRQ >> 21) & 7U) == 2U);
    CHECK((CONTROL_SIN_IRQ & (UINT32_C(3) << 26)) == 0U && (CONTROL_CON_IRQ >> 31) == 1U);
    CHECK(DIR32(adc_modelo.ADDR1) - DIR32(adc_modelo.ADDR0) == 4U);
    CHECK(offsetof(LPC_ADC_TypeDef, ADDR0) == 0x10U && offsetof(LPC_ADC_TypeDef, ADDR1) == 0x14U);
    CHECK(offsetof(LPC_ADC_TypeDef, ADGDR) == 0x04U && sizeof(GPDMA_LLI_T) == 16U);
    CHECK((DIR32(Adquisicion_Lista[0]) & 3U) == 0U && (DIR32(ADC_DMA_Pot) & 3U) == 0U);
    CHECK((DIR32(ADC_DMA_LM35) & 3U) == 0U && !error_modelo);
    return true;
}

static bool PrimerDescriptorCargadoEnElCanal(void)
{
    CHECK(Preparar(SUCCESS));
    CHECK(canal0_modelo.DMACCSrcAddr == DIR32(adc_modelo.ADDR0));
    CHECK(canal0_modelo.DMACCSrcAddr != DIR32(adc_modelo.ADGDR));
    CHECK(canal0_modelo.DMACCDestAddr == DIR32(ADC_DMA_Pot));
    CHECK(canal0_modelo.DMACCLLI == DIR32(Adquisicion_Lista[1]));
    CHECK(canal0_modelo.DMACCControl == CONTROL_SIN_IRQ);
    CHECK(canal0_modelo.DMACCConfig == UINT32_MAX && !error_modelo);
    return true;
}

static bool CanalOcupadoNoArranca(void)
{
    CHECK(!Preparar(ERROR));
    CHECK(eventos_total == 10U && eventos[9] == EV_DMA_SETUP);
    CHECK(!nvic.habilitada[DMA_IRQn] && !nvic.habilitada[ADC_IRQn]);
    CHECK(nvic.habilitaciones == 0U && nvic.prioridades == 0U);
    CHECK(canal0_modelo.DMACCSrcAddr == UINT32_MAX && canal0_modelo.DMACCControl == UINT32_MAX);
    CHECK(!Adquisicion_ADC_DMA_HayDatos() && Adquisicion_ADC_DMA_ContarPares() == 0U);
    CHECK(!error_modelo);
    return true;
}

static bool IsrMarcaParesCompletos(void)
{
    CHECK(Preparar(SUCCESS));
    DMA_IRQHandler();
    CHECK(consultas_tc == 1U && limpiezas_tc == 0U);
    CHECK(!Adquisicion_ADC_DMA_HayDatos() && Adquisicion_ADC_DMA_ContarPares() == 0U);
    ADC_DMA_Pot = UINT32_C(0x80008000); ADC_DMA_LM35 = UINT32_C(0x81001F40);
    tc_activo = true;
    DMA_IRQHandler();
    CHECK(!tc_activo && limpiezas_tc == 1U);
    CHECK(Adquisicion_ADC_DMA_HayDatos() && Adquisicion_ADC_DMA_ContarPares() == 1U);
    CHECK(Pot_Read() == 2048U && LM35_Read() == 4028);
    CHECK(ADC_DMA_Pot == UINT32_C(0x80008000) && ADC_DMA_LM35 == UINT32_C(0x81001F40));
    for (unsigned i = 0U; i < 5U; ++i) { tc_activo = true; DMA_IRQHandler(); }
    CHECK(Adquisicion_ADC_DMA_HayDatos() && Adquisicion_ADC_DMA_ContarPares() == 6U);
    Adquisicion_Pares = UINT32_MAX;
    tc_activo = true; DMA_IRQHandler();
    CHECK(Adquisicion_ADC_DMA_HayDatos() && Adquisicion_ADC_DMA_ContarPares() == 0U);
    CHECK(limpiezas_tc == 7U && !error_modelo);
    return true;
}

static bool ReinicializarOlvidaDatosPrevios(void)
{
    CHECK(Preparar(SUCCESS));
    tc_activo = true; DMA_IRQHandler();
    CHECK(Adquisicion_ADC_DMA_HayDatos());
    eventos_total = 0U; resultado_setup = SUCCESS;
    CHECK(Adquisicion_ADC_DMA_Init());
    CHECK(!Adquisicion_ADC_DMA_HayDatos() && Adquisicion_ADC_DMA_ContarPares() == 0U);
    CHECK(eventos_total == 12U && nvic.habilitada[DMA_IRQn] && !error_modelo);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"I1 SecuenciaAcordada", SecuenciaAcordada},
        {"I1 RelojDelAdcDentroDeLimite", RelojDelAdcDentroDeLimite},
        {"I1 CanalYListaEnlazada", CanalYListaEnlazada},
        {"I1 PrimerDescriptorCargadoEnElCanal", PrimerDescriptorCargadoEnElCanal},
        {"I1 CanalOcupadoNoArranca", CanalOcupadoNoArranca},
        {"I1 IsrMarcaParesCompletos", IsrMarcaParesCompletos},
        {"I1 ReinicializarOlvidaDatosPrevios", ReinicializarOlvidaDatosPrevios}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
