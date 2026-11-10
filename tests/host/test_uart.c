#include "UART.h"
#include "lpc17xx_uart.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)
#define MODEL_CAPACITY 512U
static uint8_t rx_data[MODEL_CAPACITY], tx_data[MODEL_CAPACITY];
static uint32_t rx_count, rx_offset, tx_count, tx_space;
static unsigned lecturas, escrituras;
static bool error_modelo;

uint32_t UART_Receive(LPC_UART_TypeDef *uart, uint8_t *destino, uint32_t capacidad,
                      TRANSFER_BLOCK_Type modo)
{
    ++lecturas;
    if (uart != UART0 || modo != NONE_BLOCKING || destino == NULL || capacidad == 0U) {
        error_modelo = true;
        return 0U;
    }
    const uint32_t disponibles = rx_count - rx_offset;
    const uint32_t n = capacidad < disponibles ? capacidad : disponibles;
    memcpy(destino, rx_data + rx_offset, n);
    rx_offset += n;
    return n;
}

uint32_t UART_Send(LPC_UART_TypeDef *uart, const uint8_t *datos, uint32_t longitud,
                   TRANSFER_BLOCK_Type modo)
{
    ++escrituras;
    if (uart != UART0 || modo != NONE_BLOCKING || datos == NULL || longitud == 0U) {
        error_modelo = true;
        return 0U;
    }
    const uint32_t n = longitud < tx_space ? longitud : tx_space;
    if (n > MODEL_CAPACITY - tx_count) {
        error_modelo = true;
        return 0U;
    }
    memcpy(tx_data + tx_count, datos, n);
    tx_count += n;
    tx_space -= n;
    return n;
}

static void Preparar(void)
{
    memset(rx_data, 0xA5, sizeof(rx_data));
    memset(tx_data, 0x5A, sizeof(tx_data));
    rx_count = 0U; rx_offset = 0U; tx_count = 0U; tx_space = 0U;
    lecturas = 0U; escrituras = 0U; error_modelo = false;
}

static void CargarRx(const uint8_t *datos, uint32_t longitud)
{
    if (longitud > MODEL_CAPACITY) { error_modelo = true; return; }
    memcpy(rx_data, datos, longitud);
    rx_count = longitud; rx_offset = 0U;
}

static bool ArgumentosVacios(void)
{
    Preparar();
    uint8_t buffer[] = {0x17U, 0x00U, 0xFFU, 0xD3U};
    const uint8_t copia[] = {0x17U, 0x00U, 0xFFU, 0xD3U};
    CHECK(UART_Read(NULL, 0U) == 0U && UART_Read(NULL, 1U) == 0U);
    CHECK(UART_Read(NULL, UINT32_MAX) == 0U && UART_Read(buffer, 0U) == 0U);
    CHECK(UART_Write(NULL, 0U) == 0U && UART_Write(NULL, 1U) == 0U);
    CHECK(UART_Write(NULL, UINT32_MAX) == 0U && UART_Write(buffer, 0U) == 0U);
    CHECK(memcmp(buffer, copia, sizeof(buffer)) == 0);
    CHECK(lecturas == 0U && escrituras == 0U && rx_offset == 0U && tx_count == 0U && !error_modelo);
    return true;
}

static bool RxVacio(void)
{
    Preparar();
    uint8_t destino[] = {0x11U, 0x22U, 0x33U, 0x44U};
    const uint8_t esperado[] = {0x11U, 0x22U, 0x33U, 0x44U};
    CHECK(UART_Read(destino + 1, 2U) == 0U);
    CHECK(memcmp(destino, esperado, sizeof(destino)) == 0);
    CHECK(lecturas == 1U && escrituras == 0U && !error_modelo);
    return true;
}

static bool RxCompletaBinaria(void)
{
    Preparar();
    uint8_t fuente[256], destino[258];
    for (unsigned i = 0U; i < 256U; ++i) { fuente[i] = (uint8_t)i; }
    memset(destino, 0xC3, sizeof(destino));
    CargarRx(fuente, 256U);
    CHECK(UART_Read(destino + 1, 256U) == 256U);
    CHECK(memcmp(destino + 1, fuente, sizeof(fuente)) == 0);
    CHECK(destino[0] == 0xC3U && destino[257] == 0xC3U);
    CHECK(rx_offset == 256U && lecturas == 1U && !error_modelo);
    return true;
}

static bool RxParcialYCanarios(void)
{
    Preparar();
    const uint8_t fuente[] = {0x00U, 0xFFU, 0x52U, 0x50U, 0x4DU};
    uint8_t destino[13], esperado[13];
    memset(destino, 0xB6, sizeof(destino));
    memcpy(esperado, destino, sizeof(destino));
    memcpy(esperado + 1, fuente, sizeof(fuente));
    CargarRx(fuente, (uint32_t)sizeof(fuente));
    CHECK(UART_Read(destino + 1, 11U) == sizeof(fuente));
    CHECK(memcmp(destino, esperado, sizeof(destino)) == 0);
    CHECK(lecturas == 1U && !error_modelo);
    return true;
}

static bool RxCapacidadYContinuacion(void)
{
    Preparar();
    const uint8_t fuente[] = {0xA1U, 0U, 0xFFU, 0x10U, 0x20U, 0x30U, 0x40U};
    uint8_t destino[9]; memset(destino, 0xC7, sizeof(destino));
    CargarRx(fuente, (uint32_t)sizeof(fuente));
    CHECK(UART_Read(destino + 1, 3U) == 3U && rx_offset == 3U);
    CHECK(memcmp(destino + 1, fuente, 3U) == 0 && destino[4] == 0xC7U);
    CHECK(UART_Read(destino + 4, 4U) == 4U && rx_offset == sizeof(fuente));
    CHECK(memcmp(destino + 1, fuente, sizeof(fuente)) == 0);
    CHECK(UART_Read(destino + 1, 7U) == 0U);
    CHECK(destino[0] == 0xC7U && destino[8] == 0xC7U && lecturas == 3U && !error_modelo);
    return true;
}

static bool TxNoDisponible(void)
{
    Preparar();
    const uint8_t fuente[] = {0x31U, 0U, 0xFFU};
    uint8_t antes[MODEL_CAPACITY]; memcpy(antes, tx_data, sizeof(antes));
    CHECK(UART_Write(fuente, (uint32_t)sizeof(fuente)) == 0U);
    CHECK(tx_count == 0U && memcmp(antes, tx_data, sizeof(antes)) == 0);
    CHECK(escrituras == 1U && lecturas == 0U && !error_modelo);
    return true;
}

static bool TxCompletaBinaria(void)
{
    Preparar();
    uint8_t fuente[258], copia[258];
    fuente[0] = 0xE3U; fuente[257] = 0xD7U;
    for (unsigned i = 0U; i < 256U; ++i) { fuente[i + 1U] = (uint8_t)i; }
    memcpy(copia, fuente, sizeof(copia)); tx_space = 256U;
    CHECK(UART_Write(fuente + 1, 256U) == 256U);
    CHECK(tx_count == 256U && memcmp(tx_data, fuente + 1, 256U) == 0 && tx_data[256] == 0x5AU);
    CHECK(memcmp(fuente, copia, sizeof(fuente)) == 0);
    CHECK(escrituras == 1U && !error_modelo);
    return true;
}

static bool TxParcialYCanarios(void)
{
    Preparar();
    uint8_t fuente[] = {0xABU, 0x00U, 0xFFU, 0x17U, 0x28U, 0x39U, 0xCDU};
    uint8_t copia[sizeof(fuente)]; memcpy(copia, fuente, sizeof(copia));
    tx_space = 3U;
    CHECK(UART_Write(fuente + 1, 5U) == 3U);
    CHECK(tx_count == 3U && memcmp(tx_data, fuente + 1, 3U) == 0 && tx_data[3] == 0x5AU);
    CHECK(memcmp(fuente, copia, sizeof(fuente)) == 0 && escrituras == 1U && !error_modelo);
    return true;
}

static bool TxContinuacionSinDuplicar(void)
{
    Preparar();
    const uint8_t fuente[] = {0x52U, 0x50U, 0x4DU, 0x3DU, 0U, 0xFFU, 0x31U, 0x38U, 0x35U};
    uint32_t offset = 0U;
    tx_space = 2U;
    offset += UART_Write(fuente + offset, (uint32_t)sizeof(fuente) - offset);
    CHECK(offset == 2U && tx_count == 2U && escrituras == 1U);
    CHECK(UART_Write(fuente + offset, (uint32_t)sizeof(fuente) - offset) == 0U);
    tx_space = 7U;
    offset += UART_Write(fuente + offset, (uint32_t)sizeof(fuente) - offset);
    CHECK(offset == sizeof(fuente) && tx_count == sizeof(fuente));
    CHECK(memcmp(tx_data, fuente, sizeof(fuente)) == 0 && tx_data[sizeof(fuente)] == 0x5AU);
    CHECK(escrituras == 3U && !error_modelo);
    return true;
}

static bool RxTxIntercalados(void)
{
    Preparar();
    const uint8_t entrada[] = {0U, 0xAAU, 0xFFU, 0x55U}, salida[] = {0xFEU, 0U, 0xCEU};
    uint8_t recibidos[4]; CargarRx(entrada, 4U); tx_space = 2U;
    CHECK(UART_Read(recibidos, 2U) == 2U);
    CHECK(UART_Write(salida, 3U) == 2U);
    CHECK(UART_Read(recibidos + 2, 2U) == 2U);
    tx_space = 1U; CHECK(UART_Write(salida + 2, 1U) == 1U);
    CHECK(memcmp(recibidos, entrada, sizeof(entrada)) == 0);
    CHECK(tx_count == 3U && memcmp(tx_data, salida, sizeof(salida)) == 0);
    CHECK(lecturas == 2U && escrituras == 2U && !error_modelo);
    return true;
}

static bool RecuperacionTrasInvalido(void)
{
    Preparar();
    const uint8_t fuente[] = {0x17U, 0U, 0xFFU}; uint8_t destino[3];
    CargarRx(fuente, 3U); tx_space = 3U;
    CHECK(UART_Read(NULL, 3U) == 0U && UART_Write(NULL, 3U) == 0U);
    CHECK(rx_offset == 0U && tx_count == 0U && lecturas == 0U && escrituras == 0U);
    CHECK(UART_Read(destino, 3U) == 3U && memcmp(destino, fuente, 3U) == 0);
    CHECK(UART_Write(fuente, 3U) == 3U && memcmp(tx_data, fuente, 3U) == 0);
    CHECK(lecturas == 1U && escrituras == 1U && !error_modelo);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S1 ArgumentosVacios", ArgumentosVacios}, {"S3 RxVacio", RxVacio},
        {"S2 RxCompletaBinaria", RxCompletaBinaria}, {"S2 RxParcialYCanarios", RxParcialYCanarios},
        {"S2 RxCapacidadYContinuacion", RxCapacidadYContinuacion},
        {"S4 TxNoDisponible", TxNoDisponible}, {"S4 TxCompletaBinaria", TxCompletaBinaria},
        {"S4 TxParcialYCanarios", TxParcialYCanarios}, {"S5 TxContinuacionSinDuplicar", TxContinuacionSinDuplicar},
        {"S2/S4 RxTxIntercalados", RxTxIntercalados}, {"S1 RecuperacionTrasInvalido", RecuperacionTrasInvalido}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
