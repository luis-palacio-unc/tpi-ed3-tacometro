/*
 * Dispositivos simulados para las pruebas host de Servicios y de la aplicacion
 * (B18/I2). Reemplazan a Dispositivos/ completo: cada funcion publica de un
 * dispositivo tiene aqui una version que lee o escribe la estructura fakes.
 * No simulan la biblioteca ni los registros. No forman parte del firmware.
 */
#ifndef TPI_FAKES_DISPOSITIVOS_H
#define TPI_FAKES_DISPOSITIVOS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

#define FAKES_LLAMADAS_MAX 32U
#define FAKES_RX_MAX 512U
#define FAKES_TX_MAX 4096U

typedef struct {
    /* Entradas que fija la prueba. */
    bool adc_init_ok;            /* Retorno de Adquisicion_ADC_DMA_Init. */
    uint32_t adc_consultas_sin_datos; /* Consultas de HayDatos que devuelven false. */
    uint16_t pot;                /* Retorno de Pot_Read, 0..4095. */
    int32_t lm35;                /* Retorno de LM35_Read, centesimas. */
    bool selector;               /* Retorno de Selector_Read: true = llave abierta. */
    bool hay_ventana;            /* Proxima Timer_ConsumirVentana devuelve true. */
    uint32_t pulsos;             /* Pulsos de esa ventana. */
    uint32_t tx_libres;          /* Bytes que UART_Write todavia acepta. */
    /* Salidas que observa la prueba. */
    uint32_t adc_consultas;      /* Llamadas a Adquisicion_ADC_DMA_HayDatos. */
    uint16_t motor;              /* Ultimo mando aceptado por Motor_Write. */
    uint32_t motor_escrituras;   /* Llamadas a Motor_Write. */
    uint32_t motor_rechazos;     /* Llamadas a Motor_Write con mando > 1000. */
    bool led, buzzer;            /* Ultimo valor de LED_Write y Buzzer_Write. */
    uint32_t led_escrituras, buzzer_escrituras;
    char tx[FAKES_TX_MAX];       /* Bytes aceptados por UART_Write, con NUL final. */
    uint32_t tx_bytes;
    uint8_t rx[FAKES_RX_MAX];    /* Bytes pendientes para UART_Read. */
    uint32_t rx_bytes, rx_leidos;
    const char *llamadas[FAKES_LLAMADAS_MAX]; /* Inicializaciones, en orden. */
    uint32_t llamadas_total;
} Fakes;

extern Fakes fakes;

/* Deja todo en cero, con ADC disponible y lugar en el buffer de transmision. */
void Fakes_Reiniciar(void);
/* Agrega texto a los bytes pendientes de recepcion. */
void Fakes_Recibir(const char *texto);
/* Borra lo transmitido hasta ahora. */
void Fakes_VaciarTx(void);
/* Compara la secuencia de inicializaciones con una lista terminada en NULL. */
bool Fakes_LlamadasSon(const char *const *esperadas);

#define EJECUTAR_CASOS(casos) do { \
    for (unsigned i_ = 0U; i_ < sizeof(casos) / sizeof((casos)[0]); ++i_) { \
        if (!(casos)[i_].ejecutar()) { fprintf(stderr, "FAIL %s\n", (casos)[i_].nombre); return 1; } \
        printf("PASS %s\n", (casos)[i_].nombre); \
    } \
    printf("%u/%u casos correctos\n", (unsigned)(sizeof(casos) / sizeof((casos)[0])), \
           (unsigned)(sizeof(casos) / sizeof((casos)[0]))); \
    return 0; \
} while (0)

typedef struct { const char *nombre; bool (*ejecutar)(void); } Caso;

#endif /* TPI_FAKES_DISPOSITIVOS_H */
