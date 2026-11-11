/**
 * @file Adquisicion_ADC_DMA.c
 * @brief ADC en rafaga de dos canales y DMA circular hacia las muestras.
 * @details Potenciometro.c y LM35.c poseen las muestras; este archivo configura
 * ADC y GPDMA con la biblioteca original y atiende la interrupcion de DMA.
 * @note Soporte de ESW.2.1.11/ESW.2.1.12. Unico acceso directo a registros:
 * DMACC0SrcAddr y DMACC0Control, que la biblioteca no permite fijar por descriptor.
 */
#include "Adquisicion_ADC_DMA.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"

/** @brief Tasa pedida a ADC_Init; con PCLK_ADC 25 MHz produce CLKDIV = 255. */
#define ADQUISICION_ADC_TASA_HZ 1500U
/** @brief Prioridad NVIC de DMA, la mas baja de las tres ISR propias. */
#define ADQUISICION_DMA_PRIORIDAD_IRQ 3U
/** @brief Direccion de 32 bits de un registro o variable para el GPDMA. */
#define ADQUISICION_DIRECCION(objeto) ((uint32_t)(uintptr_t)&(objeto))
/**
 * @brief DMACCxControl de cada descriptor: 1 transferencia, rafagas de 1,
 * origen y destino de 32 bits, sin incrementos (UM10360 tabla 564): 0x00480001.
 */
#define ADQUISICION_DMA_CONTROL ((uint32_t)(GPDMA_DMACCxControl_TransferSize(1U) | \
    GPDMA_DMACCxControl_SBSize(GPDMA_BSIZE_1) | GPDMA_DMACCxControl_DBSize(GPDMA_BSIZE_1) | \
    GPDMA_DMACCxControl_SWidth(GPDMA_WORD) | GPDMA_DMACCxControl_DWidth(GPDMA_WORD)))

/**
 * @brief Lista enlazada circular: [0] AD0DR0 -> potenciometro, [1] AD0DR1 -> LM35.
 * @details En SRAM local, alineada a 4 bytes; el GPDMA la relee al terminar cada
 * descriptor. Se escribe solo en Init, antes de habilitar el canal.
 */
static GPDMA_LLI_T Adquisicion_Lista[2];
/** @brief true desde el primer par completo; escrito por el ISR de DMA. */
static volatile bool Adquisicion_ParDisponible;
/** @brief Pares completos desde Init, modulo 2^32; escrito por el ISR de DMA. */
static volatile uint32_t Adquisicion_Pares;

bool Adquisicion_ADC_DMA_Init(void)
{
    const GPDMA_Channel_CFG_T canal = {
        .channelNum = GPDMA_CH_0,
        .transferSize = 1U,
        .type = GPDMA_P2M,
        .srcMemAddr = 0U,
        .dstMemAddr = ADQUISICION_DIRECCION(ADC_DMA_Pot),
        .srcConn = GPDMA_ADC,
        .dstConn = GPDMA_ADC,
        .src = {GPDMA_WORD, GPDMA_BSIZE_1, DISABLE},
        .dst = {GPDMA_WORD, GPDMA_BSIZE_1, DISABLE},
        .intTC = ENABLE,
        .intErr = DISABLE,
        .linkedList = ADQUISICION_DIRECCION(Adquisicion_Lista[1])
    };
    NVIC_DisableIRQ(ADC_IRQn);
    NVIC_DisableIRQ(DMA_IRQn);
    Adquisicion_ParDisponible = false;
    Adquisicion_Pares = 0U;
    ADC_Init(ADQUISICION_ADC_TASA_HZ);
    ADC_PinConfig(ADC_CHANNEL_0);
    ADC_PinConfig(ADC_CHANNEL_1);
    ADC_ChannelEnable(ADC_CHANNEL_0);
    ADC_ChannelEnable(ADC_CHANNEL_1);
    ADC_IntDisable(ADC_INT_GLOBAL);
    ADC_IntEnable(ADC_INT_CH0);
    ADC_IntEnable(ADC_INT_CH1);
    GPDMA_Init();
    Adquisicion_Lista[0].srcAddr = ADQUISICION_DIRECCION(LPC_ADC->ADDR0);
    Adquisicion_Lista[0].dstAddr = ADQUISICION_DIRECCION(ADC_DMA_Pot);
    Adquisicion_Lista[0].nextLLI = ADQUISICION_DIRECCION(Adquisicion_Lista[1]);
    Adquisicion_Lista[0].control = ADQUISICION_DMA_CONTROL;
    Adquisicion_Lista[1].srcAddr = ADQUISICION_DIRECCION(LPC_ADC->ADDR1);
    Adquisicion_Lista[1].dstAddr = ADQUISICION_DIRECCION(ADC_DMA_LM35);
    Adquisicion_Lista[1].nextLLI = ADQUISICION_DIRECCION(Adquisicion_Lista[0]);
    Adquisicion_Lista[1].control = ADQUISICION_DMA_CONTROL | (uint32_t)GPDMA_DMACCxControl_I;
    if (GPDMA_SetupChannel(&canal) != SUCCESS) {
        return false;
    }
    LPC_GPDMACH0->DMACCSrcAddr = Adquisicion_Lista[0].srcAddr;
    LPC_GPDMACH0->DMACCControl = Adquisicion_Lista[0].control;
    NVIC_SetPriority(DMA_IRQn, ADQUISICION_DMA_PRIORIDAD_IRQ);
    NVIC_EnableIRQ(DMA_IRQn);
    GPDMA_ChannelStart(GPDMA_CH_0);
    ADC_BurstEnable();
    return true;
}

bool Adquisicion_ADC_DMA_HayDatos(void)
{
    return Adquisicion_ParDisponible;
}

uint32_t Adquisicion_ADC_DMA_ContarPares(void)
{
    return Adquisicion_Pares;
}

/**
 * @brief ISR de GPDMA: registrar cada par de muestras completo.
 * @details Si el canal 0 tiene activa la interrupcion de fin de cuenta
 * (DMACIntTCStat, UM10360 tabla 547), que solo pide el descriptor del LM35, la
 * limpia con GPDMA_ClearIntPending (DMACIntTCClear, tabla 548), incrementa el
 * contador de pares y marca dato disponible. No toca las muestras: el GPDMA ya
 * recargo el descriptor del potenciometro y sigue transfiriendo.
 * @note Soporte de ESW.2.1.11/ESW.2.1.12. Nombre exigido por la tabla de vectores.
 * Prioridad 3; sin lazos ni esperas. Unico escritor de la bandera y del contador.
 */
void DMA_IRQHandler(void)
{
    if (GPDMA_IntGetStatus(GPDMA_INTTC, GPDMA_CH_0) == SET) {
        GPDMA_ClearIntPending(GPDMA_CLR_INTTC, GPDMA_CH_0);
        Adquisicion_Pares = Adquisicion_Pares + 1U;
        Adquisicion_ParDisponible = true;
    }
}
