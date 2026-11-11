/*
 * Pruebas host de Dispositivos/UART.c (B13 + B18/I1).
 * Incluye el .c bajo prueba para alcanzar sus buffers internos y reemplaza por
 * dobles las funciones de la biblioteca original y del NVIC. Modela U0RBR/U0THR,
 * FIFO de 16 bytes, U0IIR y U0LSR; no ejecuta perifericos reales.
 */
#include "dobles_cmsis.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include <stddef.h>
#include <stdio.h>

#include "../../Dispositivos/UART.c"

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)
#define FIFO_HW 16U
#define LINEA_MAX 1024U
#define EVENTOS_MAX 32U

typedef enum {
    EV_UART_INIT, EV_FIFO, EV_PIN_TX, EV_PIN_RX, EV_TX_ENABLE, EV_INT_RBR, EV_INT_THRE,
    EV_NVIC_PRIORIDAD, EV_NVIC_ENABLE
} Evento;

static uint8_t rx_hw[LINEA_MAX], linea[LINEA_MAX], fifo_tx[FIFO_HW];
static uint32_t rx_hw_total, rx_hw_leidos, linea_total, fifo_tx_ocupados;
static bool int_rbr, int_thre, tx_habilitada, fifo_habilitada, causa_thre, usar_cti;
static bool en_isr, error_modelo, nvic_habilitada_en_init;
static unsigned lecturas_iir, lecturas_lsr, lecturas_rbr, escrituras_thr;
static Evento eventos[EVENTOS_MAX];
static unsigned eventos_total;
static UART_CFG_T formato_visto;
static UART_FIFO_CFG_T fifo_visto;
static PINSEL_CFG_T pines_vistos[2];
static unsigned pines_total;

static void Registrar(Evento evento)
{
    if (eventos_total < EVENTOS_MAX) { eventos[eventos_total] = evento; }
    ++eventos_total;
}

void UART_Init(LPC_UART_TypeDef *uart, const UART_CFG_T *cfg)
{
    if (uart != UART0 || cfg == NULL) { error_modelo = true; return; }
    nvic_habilitada_en_init = nvic.habilitada[UART0_IRQn];
    formato_visto = *cfg;
    tx_habilitada = false; fifo_habilitada = false; int_rbr = false; int_thre = false;
    Registrar(EV_UART_INIT);
}

void UART_FIFOConfig(LPC_UART_TypeDef *uart, const UART_FIFO_CFG_T *cfg)
{
    if (uart != UART0 || cfg == NULL) { error_modelo = true; return; }
    fifo_visto = *cfg;
    fifo_habilitada = true;
    Registrar(EV_FIFO);
}

void PINSEL_ConfigPin(const PINSEL_CFG_T *cfg)
{
    if (cfg == NULL || pines_total >= 2U) { error_modelo = true; return; }
    pines_vistos[pines_total++] = *cfg;
    Registrar(cfg->pin == PIN_2 ? EV_PIN_TX : EV_PIN_RX);
}

void UART_TxEnable(LPC_UART_TypeDef *uart)
{
    if (uart != UART0) { error_modelo = true; return; }
    tx_habilitada = true;
    Registrar(EV_TX_ENABLE);
}

void UART_IntConfig(LPC_UART_TypeDef *uart, UART_INT tipo, FunctionalState estado)
{
    if (uart != UART0 || estado != ENABLE) { error_modelo = true; return; }
    if (tipo == UART_INT_RBR) { int_rbr = true; Registrar(EV_INT_RBR); }
    else if (tipo == UART_INT_THRE) { int_thre = true; Registrar(EV_INT_THRE); }
    else { error_modelo = true; }
}

uint32_t UART_GetIntId(LPC_UART_TypeDef *uart)
{
    if (uart != UART0 || !en_isr) { error_modelo = true; return UART_IIR_INTSTAT_PEND; }
    ++lecturas_iir;
    if (int_rbr && rx_hw_leidos != rx_hw_total) {
        return usar_cti ? UART_IIR_INTID_CTI : UART_IIR_INTID_RDA;
    }
    if (int_thre && causa_thre) { causa_thre = false; return UART_IIR_INTID_THRE; }
    return UART_IIR_INTSTAT_PEND;
}

uint8_t UART_GetLineStatus(LPC_UART_TypeDef *uart)
{
    if (uart != UART0 || !en_isr) { error_modelo = true; return 0U; }
    ++lecturas_lsr;
    return (uint8_t)((rx_hw_leidos != rx_hw_total ? UART_LSR_RDR : 0U) |
                     (fifo_tx_ocupados == 0U ? UART_LSR_THRE : 0U));
}

uint8_t UART_ReceiveByte(LPC_UART_TypeDef *uart)
{
    if (uart != UART0 || !en_isr || rx_hw_leidos == rx_hw_total) { error_modelo = true; return 0U; }
    ++lecturas_rbr;
    return rx_hw[rx_hw_leidos++];
}

void UART_SendByte(LPC_UART_TypeDef *uart, uint8_t dato)
{
    if (uart != UART0 || !en_isr || !tx_habilitada || fifo_tx_ocupados >= FIFO_HW) {
        error_modelo = true;
        return;
    }
    ++escrituras_thr;
    causa_thre = false;
    fifo_tx[fifo_tx_ocupados++] = dato;
}

static void Preparar(void)
{
    NvicReiniciar();
    memset(rx_hw, 0xA5, sizeof(rx_hw)); memset(linea, 0x5A, sizeof(linea));
    rx_hw_total = 0U; rx_hw_leidos = 0U; linea_total = 0U; fifo_tx_ocupados = 0U;
    causa_thre = false; usar_cti = false; en_isr = false; error_modelo = false;
    nvic_habilitada_en_init = true;
    lecturas_iir = 0U; lecturas_lsr = 0U; lecturas_rbr = 0U; escrituras_thr = 0U;
    eventos_total = 0U; pines_total = 0U;
    memset(&formato_visto, 0xFF, sizeof(formato_visto));
    memset(&fifo_visto, 0xFF, sizeof(fifo_visto));
    UART0_Init();
}

static void LlegarBytes(const uint8_t *datos, uint32_t cantidad)
{
    if (cantidad > LINEA_MAX - rx_hw_total) { error_modelo = true; return; }
    memcpy(rx_hw + rx_hw_total, datos, cantidad);
    rx_hw_total += cantidad;
}

static bool CausaActiva(void)
{
    return (int_rbr && rx_hw_leidos != rx_hw_total) || (int_thre && causa_thre);
}

static unsigned AtenderIRQ(void)
{
    unsigned entradas = 0U;
    while (nvic.habilitada[UART0_IRQn] && (nvic.pendiente[UART0_IRQn] || CausaActiva())) {
        if (++entradas > 256U) { error_modelo = true; break; }
        nvic.pendiente[UART0_IRQn] = false;
        en_isr = true;
        UART0_IRQHandler();
        en_isr = false;
    }
    return entradas;
}

static void VaciarFifoALinea(void)
{
    if (fifo_tx_ocupados == 0U || fifo_tx_ocupados > LINEA_MAX - linea_total) { return; }
    memcpy(linea + linea_total, fifo_tx, fifo_tx_ocupados);
    linea_total += fifo_tx_ocupados;
    fifo_tx_ocupados = 0U;
    causa_thre = true;
}

static void TransmitirTodo(void)
{
    for (unsigned i = 0U; i < 64U; ++i) {
        (void)AtenderIRQ();
        if (fifo_tx_ocupados == 0U) { break; }
        VaciarFifoALinea();
    }
}

static void Secuencia(uint8_t *destino, uint32_t cantidad, uint8_t inicio)
{
    for (uint32_t i = 0U; i < cantidad; ++i) { destino[i] = (uint8_t)(inicio + i); }
}

static bool InicializacionAcordada(void)
{
    Preparar();
    const Evento esperado[] = {EV_UART_INIT, EV_FIFO, EV_PIN_TX, EV_PIN_RX, EV_TX_ENABLE,
                               EV_INT_RBR, EV_INT_THRE};
    CHECK(UART_BAUDRATE == 9600U && UART_BUFFER_RX_BYTES == 64U && UART_BUFFER_TX_BYTES == 128U);
    CHECK(eventos_total == sizeof(esperado) / sizeof(esperado[0]));
    CHECK(memcmp(eventos, esperado, sizeof(esperado)) == 0);
    CHECK(formato_visto.baudRate == 9600U && formato_visto.parity == UART_PARITY_NONE);
    CHECK(formato_visto.dataBits == UART_DBITS_8 && formato_visto.stopBits == UART_STOPBIT_1);
    CHECK(fifo_visto.level == UART_FIFO_TRGLEV0 && fifo_visto.dmaMode == DISABLE);
    CHECK(fifo_visto.resetRxBuf == ENABLE && fifo_visto.resetTxBuf == ENABLE);
    CHECK(pines_vistos[0].port == PORT_0 && pines_vistos[0].pin == PIN_2);
    CHECK(pines_vistos[0].func == PINSEL_FUNC_01 && pines_vistos[0].mode == PINSEL_TRISTATE);
    CHECK(pines_vistos[0].openDrain == DISABLE);
    CHECK(pines_vistos[1].port == PORT_0 && pines_vistos[1].pin == PIN_3);
    CHECK(pines_vistos[1].func == PINSEL_FUNC_01 && pines_vistos[1].mode == PINSEL_PULLUP);
    CHECK(pines_vistos[1].openDrain == DISABLE);
    CHECK(tx_habilitada && fifo_habilitada && int_rbr && int_thre);
    CHECK(!nvic_habilitada_en_init && nvic.deshabilitaciones == 1U);
    CHECK(nvic.habilitada[UART0_IRQn] && nvic.prioridad[UART0_IRQn] == 2U);
    CHECK(nvic.habilitaciones == 1U && nvic.prioridades == 1U && nvic.pedidos == 0U);
    for (unsigned i = 0U; i < NVIC_MODELO_IRQS; ++i) {
        CHECK(i == (unsigned)UART0_IRQn || (!nvic.habilitada[i] && nvic.prioridad[i] == UINT32_MAX));
    }
    CHECK(escrituras_thr == 0U && lecturas_rbr == 0U && !nvic.error && !error_modelo);
    return true;
}

static bool ArgumentosVacios(void)
{
    Preparar();
    uint8_t buffer[] = {0x17U, 0x00U, 0xFFU, 0xD3U};
    const uint8_t copia[] = {0x17U, 0x00U, 0xFFU, 0xD3U};
    LlegarBytes(copia, 2U); (void)AtenderIRQ();
    const unsigned lsr = lecturas_lsr, iir = lecturas_iir;
    CHECK(UART_Read(NULL, 0U) == 0U && UART_Read(NULL, 1U) == 0U);
    CHECK(UART_Read(NULL, UINT32_MAX) == 0U && UART_Read(buffer, 0U) == 0U);
    CHECK(UART_Write(NULL, 0U) == 0U && UART_Write(NULL, 1U) == 0U);
    CHECK(UART_Write(NULL, UINT32_MAX) == 0U && UART_Write(buffer, 0U) == 0U);
    CHECK(memcmp(buffer, copia, sizeof(buffer)) == 0);
    CHECK(nvic.pedidos == 0U && escrituras_thr == 0U && lecturas_lsr == lsr && lecturas_iir == iir);
    CHECK(UART_Read(buffer, 4U) == 2U && buffer[0] == 0x17U && buffer[1] == 0x00U);
    CHECK(UART_Write(copia, 4U) == 4U);
    TransmitirTodo();
    CHECK(linea_total == 4U && memcmp(linea, copia, 4U) == 0 && !nvic.error && !error_modelo);
    return true;
}

static bool RxVacio(void)
{
    Preparar();
    uint8_t destino[] = {0x11U, 0x22U, 0x33U, 0x44U};
    const uint8_t esperado[] = {0x11U, 0x22U, 0x33U, 0x44U};
    CHECK(UART_Read(destino + 1, 2U) == 0U);
    CHECK(memcmp(destino, esperado, sizeof(destino)) == 0);
    CHECK(lecturas_rbr == 0U && lecturas_lsr == 0U && lecturas_iir == 0U && !error_modelo);
    return true;
}

static bool RxPorIsrParcialYContinuacion(void)
{
    Preparar();
    const uint8_t fuente[] = {0xA1U, 0U, 0xFFU, 0x10U, 0x20U, 0x30U, 0x40U};
    uint8_t destino[9]; memset(destino, 0xC7, sizeof(destino));
    LlegarBytes(fuente, (uint32_t)sizeof(fuente));
    CHECK(UART_Read(destino + 1, 7U) == 0U && destino[1] == 0xC7U);
    CHECK(AtenderIRQ() == 1U && lecturas_rbr == sizeof(fuente) && rx_hw_leidos == rx_hw_total);
    CHECK(UART_Read(destino + 1, 3U) == 3U);
    CHECK(memcmp(destino + 1, fuente, 3U) == 0 && destino[4] == 0xC7U);
    CHECK(UART_Read(destino + 4, 11U) == 4U);
    CHECK(memcmp(destino + 1, fuente, sizeof(fuente)) == 0);
    CHECK(UART_Read(destino + 1, 7U) == 0U);
    CHECK(destino[0] == 0xC7U && destino[8] == 0xC7U && escrituras_thr == 0U && !error_modelo);
    return true;
}

static bool RxTimeoutDeCaracter(void)
{
    Preparar();
    const uint8_t fuente[] = {0x52U, 0x50U, 0x4DU};
    uint8_t destino[3];
    usar_cti = true;
    LlegarBytes(fuente, 3U);
    CHECK(AtenderIRQ() == 1U);
    CHECK(UART_Read(destino, 3U) == 3U && memcmp(destino, fuente, 3U) == 0 && !error_modelo);
    return true;
}

static bool RxIsrAcotadaADieciseis(void)
{
    Preparar();
    uint8_t fuente[40], destino[40];
    Secuencia(fuente, 40U, 1U);
    LlegarBytes(fuente, 40U);
    nvic.pendiente[UART0_IRQn] = false;
    en_isr = true; UART0_IRQHandler(); en_isr = false;
    CHECK(lecturas_rbr == 16U && rx_hw_leidos == 16U);
    CHECK(AtenderIRQ() == 2U && lecturas_rbr == 40U);
    CHECK(UART_Read(destino, 40U) == 40U && memcmp(destino, fuente, 40U) == 0 && !error_modelo);
    return true;
}

static bool RxVueltaDelBufferEIndices(void)
{
    Preparar();
    UART_RxEscritos = UINT32_MAX - 20U; UART_RxLeidos = UINT32_MAX - 20U;
    uint8_t fuente[48], destino[48];
    for (unsigned vuelta = 0U; vuelta < 5U; ++vuelta) {
        Secuencia(fuente, 48U, (uint8_t)(vuelta * 50U));
        rx_hw_total = 0U; rx_hw_leidos = 0U;
        LlegarBytes(fuente, 48U);
        CHECK(AtenderIRQ() == 3U);
        memset(destino, 0, sizeof(destino));
        CHECK(UART_Read(destino, 48U) == 48U && memcmp(destino, fuente, 48U) == 0);
        CHECK(UART_Read(destino, 48U) == 0U);
    }
    CHECK(UART_RxEscritos == UART_RxLeidos && UART_RxLeidos == 219U && !error_modelo);
    return true;
}

static bool RxLlenoDescartaNuevos(void)
{
    Preparar();
    uint8_t fuente[80], destino[80];
    Secuencia(fuente, 80U, 3U);
    LlegarBytes(fuente, 80U);
    CHECK(AtenderIRQ() == 5U);
    CHECK(lecturas_rbr == 80U && rx_hw_leidos == rx_hw_total);
    memset(destino, 0xEE, sizeof(destino));
    CHECK(UART_Read(destino, 80U) == 64U && memcmp(destino, fuente, 64U) == 0);
    CHECK(destino[64] == 0xEEU && UART_Read(destino, 80U) == 0U);
    const uint8_t posteriores[] = {0xFEU, 0x00U, 0xCEU};
    LlegarBytes(posteriores, 3U);
    CHECK(AtenderIRQ() == 1U && UART_Read(destino, 80U) == 3U);
    CHECK(memcmp(destino, posteriores, 3U) == 0 && !error_modelo);
    return true;
}

static bool TxTramaDe53Completa(void)
{
    Preparar();
    uint8_t trama[55], copia[55];
    Secuencia(trama, 55U, 0x20U);
    memcpy(copia, trama, sizeof(copia));
    CHECK(UART_Write(trama + 1, 53U) == 53U);
    CHECK(escrituras_thr == 0U && lecturas_lsr == 0U);
    CHECK(nvic.pedidos == 1U && nvic.pendiente[UART0_IRQn]);
    CHECK(AtenderIRQ() == 1U && fifo_tx_ocupados == 16U && escrituras_thr == 16U);
    VaciarFifoALinea(); CHECK(AtenderIRQ() == 1U && fifo_tx_ocupados == 16U);
    VaciarFifoALinea(); CHECK(AtenderIRQ() == 1U && fifo_tx_ocupados == 16U);
    VaciarFifoALinea(); CHECK(AtenderIRQ() == 1U && fifo_tx_ocupados == 5U);
    VaciarFifoALinea(); CHECK(AtenderIRQ() == 1U && fifo_tx_ocupados == 0U);
    CHECK(linea_total == 53U && memcmp(linea, trama + 1, 53U) == 0 && linea[53] == 0x5AU);
    CHECK(escrituras_thr == 53U && !CausaActiva() && AtenderIRQ() == 0U);
    CHECK(memcmp(trama, copia, sizeof(trama)) == 0 && lecturas_rbr == 0U && !error_modelo);
    return true;
}

static bool TxCebadoTrasReposoYConFifoOcupada(void)
{
    Preparar();
    const uint8_t primera[] = {0x31U, 0U, 0xFFU}, segunda[] = {0x41U, 0x42U}, tercera[] = {0x43U};
    CHECK(UART_Write(primera, 3U) == 3U);
    TransmitirTodo();
    CHECK(linea_total == 3U && !CausaActiva() && !nvic.pendiente[UART0_IRQn]);
    CHECK(UART_Write(segunda, 2U) == 2U && nvic.pendiente[UART0_IRQn]);
    CHECK(AtenderIRQ() == 1U && fifo_tx_ocupados == 2U);
    const unsigned enviados = escrituras_thr;
    CHECK(UART_Write(tercera, 1U) == 1U);
    CHECK(AtenderIRQ() == 1U && fifo_tx_ocupados == 2U && escrituras_thr == enviados);
    TransmitirTodo();
    const uint8_t esperado[] = {0x31U, 0U, 0xFFU, 0x41U, 0x42U, 0x43U};
    CHECK(linea_total == 6U && memcmp(linea, esperado, 6U) == 0 && !error_modelo);
    return true;
}

static bool TxLlenoParcialYContinuacion(void)
{
    Preparar();
    uint8_t fuente[160];
    Secuencia(fuente, 160U, 7U);
    uint32_t offset = UART_Write(fuente, 100U);
    CHECK(offset == 100U);
    const uint32_t parcial = UART_Write(fuente + offset, 53U);
    CHECK(parcial == 28U);
    offset += parcial;
    CHECK(UART_Write(fuente + offset, 1U) == 0U && UART_Write(fuente + offset, 32U) == 0U);
    CHECK(escrituras_thr == 0U);
    CHECK(AtenderIRQ() == 1U && fifo_tx_ocupados == 16U);
    CHECK(UART_Write(fuente + offset, 32U) == 16U);
    offset += 16U;
    TransmitirTodo();
    CHECK(linea_total == 144U && memcmp(linea, fuente, 144U) == 0);
    CHECK(UART_Write(fuente + offset, 160U - offset) == 16U);
    TransmitirTodo();
    CHECK(linea_total == 160U && memcmp(linea, fuente, 160U) == 0 && linea[160] == 0x5AU);
    CHECK(!error_modelo);
    return true;
}

static bool TxVueltaDeIndices(void)
{
    Preparar();
    UART_TxEncolados = UINT32_MAX - 10U; UART_TxEnviados = UINT32_MAX - 10U;
    uint8_t trama[53];
    for (unsigned vuelta = 0U; vuelta < 6U; ++vuelta) {
        Secuencia(trama, 53U, (uint8_t)(vuelta * 40U));
        linea_total = 0U;
        CHECK(UART_Write(trama, 53U) == 53U);
        TransmitirTodo();
        CHECK(linea_total == 53U && memcmp(linea, trama, 53U) == 0);
    }
    CHECK(UART_TxEncolados == UART_TxEnviados && UART_TxEnviados == 307U && !error_modelo);
    return true;
}

static bool RxTxIntercaladosEIsrSinCausa(void)
{
    Preparar();
    const uint8_t entrada[] = {0U, 0xAAU, 0xFFU, 0x55U}, salida[] = {0xFEU, 0U, 0xCEU};
    uint8_t recibidos[4];
    en_isr = true; UART0_IRQHandler(); en_isr = false;
    CHECK(escrituras_thr == 0U && lecturas_rbr == 0U && lecturas_iir == 1U && lecturas_lsr == 1U);
    LlegarBytes(entrada, 4U);
    CHECK(UART_Write(salida, 3U) == 3U);
    CHECK(AtenderIRQ() == 1U && fifo_tx_ocupados == 3U);
    CHECK(UART_Read(recibidos, 2U) == 2U && UART_Read(recibidos + 2, 2U) == 2U);
    TransmitirTodo();
    CHECK(memcmp(recibidos, entrada, sizeof(entrada)) == 0);
    CHECK(linea_total == 3U && memcmp(linea, salida, sizeof(salida)) == 0 && !error_modelo);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"I1 InicializacionAcordada", InicializacionAcordada},
        {"S1 ArgumentosVacios", ArgumentosVacios}, {"S3 RxVacio", RxVacio},
        {"S2 RxPorIsrParcialYContinuacion", RxPorIsrParcialYContinuacion},
        {"S2 RxTimeoutDeCaracter", RxTimeoutDeCaracter},
        {"I1 RxIsrAcotadaADieciseis", RxIsrAcotadaADieciseis},
        {"I1 RxVueltaDelBufferEIndices", RxVueltaDelBufferEIndices},
        {"I1 RxLlenoDescartaNuevos", RxLlenoDescartaNuevos},
        {"S4 TxTramaDe53Completa", TxTramaDe53Completa},
        {"I1 TxCebadoTrasReposoYConFifoOcupada", TxCebadoTrasReposoYConFifoOcupada},
        {"S4/S5 TxLlenoParcialYContinuacion", TxLlenoParcialYContinuacion},
        {"I1 TxVueltaDeIndices", TxVueltaDeIndices},
        {"S2/S4 RxTxIntercaladosEIsrSinCausa", RxTxIntercaladosEIsrSinCausa}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
