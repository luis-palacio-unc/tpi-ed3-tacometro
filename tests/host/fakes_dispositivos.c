/* Dispositivos simulados; ver fakes_dispositivos.h. */
#include "fakes_dispositivos.h"
#include "Adquisicion_ADC_DMA.h"
#include "Buzzer.h"
#include "LED.h"
#include "LM35.h"
#include "Motor.h"
#include "Potenciometro.h"
#include "Reloj.h"
#include "Selector.h"
#include "Sensor_Optico.h"
#include "Timer.h"
#include "UART.h"
#include <string.h>

Fakes fakes;

static void Registrar(const char *nombre)
{
    if (fakes.llamadas_total < FAKES_LLAMADAS_MAX) {
        fakes.llamadas[fakes.llamadas_total] = nombre;
    }
    ++fakes.llamadas_total;
}

void Fakes_Reiniciar(void)
{
    memset(&fakes, 0, sizeof(fakes));
    fakes.adc_init_ok = true;
    fakes.tx_libres = UART_BUFFER_TX_BYTES;
}

void Fakes_Recibir(const char *texto)
{
    const size_t longitud = strlen(texto);
    if (fakes.rx_bytes + longitud <= FAKES_RX_MAX) {
        memcpy(&fakes.rx[fakes.rx_bytes], texto, longitud);
        fakes.rx_bytes += (uint32_t)longitud;
    }
}

void Fakes_VaciarTx(void)
{
    fakes.tx_bytes = 0U;
    fakes.tx[0] = '\0';
    fakes.tx_libres = UART_BUFFER_TX_BYTES;
}

bool Fakes_LlamadasSon(const char *const *esperadas)
{
    uint32_t i = 0U;
    for (; esperadas[i] != NULL; ++i) {
        if (i >= fakes.llamadas_total || i >= FAKES_LLAMADAS_MAX ||
            strcmp(esperadas[i], fakes.llamadas[i]) != 0) {
            fprintf(stderr, "llamada %u: se esperaba %s\n", (unsigned)i, esperadas[i]);
            return false;
        }
    }
    return i == fakes.llamadas_total;
}

void Reloj_Init(void) { Registrar("Reloj_Init"); }

bool Adquisicion_ADC_DMA_Init(void)
{
    Registrar("Adquisicion_ADC_DMA_Init");
    return fakes.adc_init_ok;
}

bool Adquisicion_ADC_DMA_HayDatos(void)
{
    ++fakes.adc_consultas;
    return fakes.adc_consultas > fakes.adc_consultas_sin_datos;
}

void Motor_Init(void) { Registrar("Motor_Init"); }

bool Motor_Write(uint16_t mando_por_mil)
{
    ++fakes.motor_escrituras;
    if (mando_por_mil > 1000U) {
        ++fakes.motor_rechazos;
        return false;
    }
    fakes.motor = mando_por_mil;
    return true;
}

void SensorOptico_Init(void) { Registrar("SensorOptico_Init"); }
void Timer_Init(void) { Registrar("Timer_Init"); }
void Timer_Start(void) { Registrar("Timer_Start"); }

bool Timer_ConsumirVentana(uint32_t *pulsos)
{
    if (!fakes.hay_ventana) {
        return false;
    }
    fakes.hay_ventana = false;
    *pulsos = fakes.pulsos;
    return true;
}

void UART0_Init(void) { Registrar("UART0_Init"); }

uint32_t UART_Read(uint8_t *destino, uint32_t capacidad)
{
    uint32_t copiados = 0U;
    while (copiados < capacidad && fakes.rx_leidos < fakes.rx_bytes) {
        destino[copiados] = fakes.rx[fakes.rx_leidos];
        ++copiados;
        ++fakes.rx_leidos;
    }
    return copiados;
}

uint32_t UART_Write(const uint8_t *datos, uint32_t longitud)
{
    uint32_t aceptados = 0U;
    while (aceptados < longitud && fakes.tx_libres > 0U && fakes.tx_bytes + 1U < FAKES_TX_MAX) {
        fakes.tx[fakes.tx_bytes] = (char)datos[aceptados];
        ++fakes.tx_bytes;
        ++aceptados;
        --fakes.tx_libres;
    }
    fakes.tx[fakes.tx_bytes] = '\0';
    return aceptados;
}

void Selector_Init(void) { Registrar("Selector_Init"); }
bool Selector_Read(void) { return fakes.selector; }

void LED_Init(void) { Registrar("LED_Init"); }
void LED_Write(bool activar) { fakes.led = activar; ++fakes.led_escrituras; }
void Buzzer_Init(void) { Registrar("Buzzer_Init"); }
void Buzzer_Write(bool activar) { fakes.buzzer = activar; ++fakes.buzzer_escrituras; }

uint16_t Pot_Read(void) { return fakes.pot; }
int32_t LM35_Read(void) { return fakes.lm35; }
