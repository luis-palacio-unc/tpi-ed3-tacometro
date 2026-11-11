/**
 * @file UART.c
 * @brief UART0 por interrupcion con buffers circulares de recepcion y transmision.
 * @details Usa la biblioteca original para configurar y para cada acceso a U0RBR,
 * U0THR, U0IIR, U0LSR y U0IER. El lazo principal y el ISR comparten dos buffers
 * circulares de un productor y un consumidor, sin secciones criticas: cada indice
 * tiene un unico escritor y solo el ISR toca las FIFO del periferico.
 * @note ESW.2.1.16. No modifica Drivers ni implementa protocolo; sin acceso
 * directo a registros.
 */
#include "UART.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include <stddef.h>

/** @brief Mascara de indice del buffer de recepcion. */
#define UART_RX_MASCARA (UART_BUFFER_RX_BYTES - 1U)
/** @brief Mascara de indice del buffer de transmision. */
#define UART_TX_MASCARA (UART_BUFFER_TX_BYTES - 1U)
/** @brief Profundidad de la FIFO de recepcion de UART0 (UM10360 cap. 14). */
#define UART_FIFO_RX_BYTES 16U
/** @brief Prioridad NVIC de UART0, por debajo de TIMER0 y por encima de DMA. */
#define UART_PRIORIDAD_IRQ 2U

_Static_assert((UART_BUFFER_RX_BYTES & UART_RX_MASCARA) == 0U,
               "UART_BUFFER_RX_BYTES debe ser potencia de dos.");
_Static_assert((UART_BUFFER_TX_BYTES & UART_TX_MASCARA) == 0U,
               "UART_BUFFER_TX_BYTES debe ser potencia de dos.");

/** @brief Almacenamiento de recepcion; el ISR escribe, UART_Read lee. */
static volatile uint8_t UART_RxDatos[UART_BUFFER_RX_BYTES];
/** @brief Total de bytes guardados por el ISR, modulo 2^32. */
static volatile uint32_t UART_RxEscritos;
/** @brief Total de bytes retirados por UART_Read, modulo 2^32. */
static volatile uint32_t UART_RxLeidos;
/** @brief Almacenamiento de transmision; UART_Write escribe, la carga de FIFO lee. */
static volatile uint8_t UART_TxDatos[UART_BUFFER_TX_BYTES];
/** @brief Total de bytes encolados por UART_Write, modulo 2^32. */
static volatile uint32_t UART_TxEncolados;
/** @brief Total de bytes entregados a U0THR, modulo 2^32. */
static volatile uint32_t UART_TxEnviados;

/**
 * @brief Pasar del buffer circular a la FIFO de transmision hasta 16 bytes.
 * @details Llama UART_SendByte por cada byte pendiente, como maximo
 * UART_TX_FIFO_SIZE, y avanza el indice de enviados. No consulta estado.
 * @pre FIFO de transmision vacia (U0LSR.THRE = 1). Solo se llama desde el ISR.
 * @note USW.2.1.16.5. Unidad interna; unico escritor de U0THR y de UART_TxEnviados.
 */
static void UART_CargarFifoTx(void)
{
    const uint32_t encolados = UART_TxEncolados;
    uint32_t enviados = UART_TxEnviados;
    for (uint32_t i = 0U; i < UART_TX_FIFO_SIZE && enviados != encolados; ++i) {
        UART_SendByte(UART0, UART_TxDatos[enviados & UART_TX_MASCARA]);
        ++enviados;
    }
    UART_TxEnviados = enviados;
}

/**
 * @brief Pasar de la FIFO de recepcion al buffer circular hasta 16 bytes.
 * @details Mientras U0LSR.RDR = 1, y como maximo UART_FIFO_RX_BYTES veces, lee
 * U0RBR con UART_ReceiveByte. Si el buffer circular esta lleno el byte leido se
 * descarta: se conservan los mas antiguos y la lectura retira la causa.
 * @pre Solo se llama desde el ISR, con causa RDA o CTI identificada.
 * @note USW.2.1.16.6. Unidad interna; unico escritor de UART_RxEscritos.
 */
static void UART_GuardarRecibidos(void)
{
    uint32_t escritos = UART_RxEscritos;
    for (uint32_t i = 0U; i < UART_FIFO_RX_BYTES &&
                          (UART_GetLineStatus(UART0) & UART_LSR_RDR) != 0U; ++i) {
        const uint8_t dato = UART_ReceiveByte(UART0);
        if ((escritos - UART_RxLeidos) < UART_BUFFER_RX_BYTES) {
            UART_RxDatos[escritos & UART_RX_MASCARA] = dato;
            ++escritos;
        }
    }
    UART_RxEscritos = escritos;
}

void UART0_Init(void)
{
    const UART_CFG_T formato = {
        .baudRate = UART_BAUDRATE,
        .parity = UART_PARITY_NONE,
        .dataBits = UART_DBITS_8,
        .stopBits = UART_STOPBIT_1
    };
    const UART_FIFO_CFG_T fifo = {
        .resetRxBuf = ENABLE,
        .resetTxBuf = ENABLE,
        .dmaMode = DISABLE,
        .level = UART_FIFO_TRGLEV0
    };
    const PINSEL_CFG_T pin_tx = {PORT_0, PIN_2, PINSEL_FUNC_01, PINSEL_TRISTATE, DISABLE};
    const PINSEL_CFG_T pin_rx = {PORT_0, PIN_3, PINSEL_FUNC_01, PINSEL_PULLUP, DISABLE};
    NVIC_DisableIRQ(UART0_IRQn);
    UART_RxEscritos = 0U;
    UART_RxLeidos = 0U;
    UART_TxEncolados = 0U;
    UART_TxEnviados = 0U;
    UART_Init(UART0, &formato);
    UART_FIFOConfig(UART0, &fifo);
    PINSEL_ConfigPin(&pin_tx);
    PINSEL_ConfigPin(&pin_rx);
    UART_TxEnable(UART0);
    UART_IntConfig(UART0, UART_INT_RBR, ENABLE);
    UART_IntConfig(UART0, UART_INT_THRE, ENABLE);
    NVIC_SetPriority(UART0_IRQn, UART_PRIORIDAD_IRQ);
    NVIC_EnableIRQ(UART0_IRQn);
}

uint32_t UART_Read(uint8_t *destino, uint32_t capacidad)
{
    if (destino == NULL || capacidad == 0U) {
        return 0U;
    }
    const uint32_t leidos = UART_RxLeidos;
    const uint32_t disponibles = UART_RxEscritos - leidos;
    const uint32_t cantidad = capacidad < disponibles ? capacidad : disponibles;
    for (uint32_t i = 0U; i < cantidad; ++i) {
        destino[i] = UART_RxDatos[(leidos + i) & UART_RX_MASCARA];
    }
    UART_RxLeidos = leidos + cantidad;
    return cantidad;
}

uint32_t UART_Write(const uint8_t *datos, uint32_t longitud)
{
    if (datos == NULL || longitud == 0U) {
        return 0U;
    }
    const uint32_t encolados = UART_TxEncolados;
    const uint32_t libres = UART_BUFFER_TX_BYTES - (encolados - UART_TxEnviados);
    const uint32_t cantidad = longitud < libres ? longitud : libres;
    for (uint32_t i = 0U; i < cantidad; ++i) {
        UART_TxDatos[(encolados + i) & UART_TX_MASCARA] = datos[i];
    }
    UART_TxEncolados = encolados + cantidad;
    NVIC_SetPendingIRQ(UART0_IRQn);
    return cantidad;
}

/**
 * @brief ISR de UART0: mover bytes entre las FIFO del periferico y los buffers.
 * @details Lee U0IIR una vez con UART_GetIntId (UM10360 tablas 277/278). Si hay
 * causa RDA o CTI guarda los bytes recibidos. Despues, con cualquier causa o sin
 * ella (pedido por software de UART_Write), lee U0LSR con UART_GetLineStatus y, si
 * THRE = 1, carga hasta 16 bytes pendientes en la FIFO de transmision. La causa
 * THRE se limpia al leer U0IIR o al escribir U0THR; RLS (no habilitada) al leer
 * U0LSR. Cuando no queda nada por enviar la transmision se detiene hasta el
 * proximo UART_Write. Si queda otra causa activa, el NVIC reingresa.
 * @note USW.2.1.16.4. Nombre exigido por la tabla de vectores. Prioridad 2; lazos
 * acotados a 16 iteraciones, sin esperas. No valida paridad/trama/desborde.
 */
void UART0_IRQHandler(void)
{
    const uint32_t identificacion = UART_GetIntId(UART0);
    if ((identificacion & UART_IIR_INTSTAT_PEND) == 0U) {
        const uint32_t causa = identificacion & UART_IIR_INTID_MASK;
        if (causa == UART_IIR_INTID_RDA || causa == UART_IIR_INTID_CTI) {
            UART_GuardarRecibidos();
        }
    }
    if ((UART_GetLineStatus(UART0) & UART_LSR_THRE) != 0U) {
        UART_CargarFifoTx();
    }
}
