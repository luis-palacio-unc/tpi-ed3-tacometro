/*
 * Prueba de sistema extremo a extremo (B18/I2): main.c, Procesamiento.c y todos
 * los servicios reales, con los dispositivos simulados de fakes_dispositivos.
 * main.c se compila con main renombrado; la prueba llama Aplicacion_Inicializar
 * y Aplicacion_Iterar, sin depender del lazo infinito.
 * Ganancias por defecto de Control.h: Kp = 4/1000 e Ki = 1/1000 por mil por
 * milesima de RPM. Con consigna 100 RPM y eje detenido el primer mando es 400 y
 * cada periodo suma 100 por mil de integral.
 */
#include "main.h"
#include "Procesamiento.h"
#include "Temperatura.h"
#include "fakes_dispositivos.h"
#include <string.h>

/* Lleva el filtro de temperatura y la alarma a un estado conocido (25 grados, apagada). */
static void Arrancar(bool llave_en_manual)
{
    Fakes_Reiniciar();
    fakes.lm35 = 2500;
    for (unsigned i = 0U; i < TEMPERATURA_MUESTRAS; ++i) {
        (void)Procesamiento_EvaluarAlarma(Temperatura_Leer());
    }
    fakes.selector = llave_en_manual;
    (void)Aplicacion_Inicializar();
}

/* Cierra una ventana de 500 ms con esos pulsos y ejecuta una vuelta del lazo. */
static void Periodo(uint32_t pulsos)
{
    Fakes_VaciarTx();
    fakes.hay_ventana = true;
    fakes.pulsos = pulsos;
    Aplicacion_Iterar();
}

static bool Tx(const char *esperada)
{
    return strcmp(fakes.tx, esperada) == 0;
}

/* Debe ser el primer caso: no prepara el filtro ni la alarma. */
static bool S1_ArranqueEnAutomatico(void)
{
    static const char *const orden[] = {
        "Reloj_Init", "Adquisicion_ADC_DMA_Init", "Motor_Init", "LED_Init", "Buzzer_Init",
        "Selector_Init", "UART0_Init", "SensorOptico_Init", "Timer_Init", "Timer_Start", NULL
    };
    Fakes_Reiniciar();
    fakes.lm35 = 2500;
    fakes.pot = 4095U;
    fakes.motor = 777U;
    fakes.led = true;
    fakes.buzzer = true;
    CHECK(Aplicacion_Inicializar());
    CHECK(Fakes_LlamadasSon(orden));
    CHECK(fakes.motor == 0U && fakes.motor_escrituras == 1U && !fakes.led && !fakes.buzzer);
    for (unsigned i = 0U; i < 3U; ++i) {
        Aplicacion_Iterar();
    }
    CHECK(fakes.motor_escrituras == 1U && fakes.tx_bytes == 0U);
    Periodo(0U);
    CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=AUTO_ESPERA\n") && fakes.motor == 0U);
    CHECK(!fakes.led && !fakes.buzzer);
    Fakes_Recibir("RPM 100\n");
    Aplicacion_Iterar();
    CHECK(fakes.motor == 0U);
    Periodo(0U);
    CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=AUTO_ACTIVO\n") && fakes.motor == 400U);
    Periodo(50U);
    CHECK(Tx("RPM=100.000;TEMP=25.00;ESTADO=AUTO_ACTIVO\n") && fakes.motor == 100U);
    Periodo(1U);
    CHECK(Tx("RPM=2.000;TEMP=25.00;ESTADO=AUTO_ACTIVO\n"));
    return true;
}

static bool S2_ConsignaDelCambioDeModoSeDescarta(void)
{
    Arrancar(true);
    fakes.pot = 2048U;
    Fakes_Recibir("RPM 50\n");
    Periodo(0U);
    CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=MANUAL\n") && fakes.motor == 500U);
    fakes.selector = false;
    Fakes_Recibir("RPM 100\nRPM 1");
    Periodo(0U);
    CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=AUTO_ESPERA\n") && fakes.motor == 0U);
    Fakes_Recibir("20\n");
    Periodo(0U);
    CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=AUTO_ESPERA\n") && fakes.motor == 0U);
    fakes.pot = 4095U;
    Aplicacion_Iterar();
    CHECK(fakes.motor == 0U);
    Fakes_Recibir("RPM 100\n");
    Periodo(0U);
    CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=AUTO_ACTIVO\n") && fakes.motor == 400U);
    return true;
}

static bool S3_ManualSigueAlPotenciometro(void)
{
    const struct { uint16_t pot; uint16_t mando; } casos[] = {
        {0U, 0U}, {2048U, 500U}, {4095U, 1000U}, {1U, 0U}, {3U, 1U}
    };
    Arrancar(true);
    for (unsigned i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        const uint32_t escrituras = fakes.motor_escrituras;
        fakes.pot = casos[i].pot;
        Aplicacion_Iterar();
        CHECK(fakes.motor == casos[i].mando && fakes.motor_escrituras == escrituras + 1U);
    }
    CHECK(fakes.tx_bytes == 0U);
    fakes.pot = 4095U;
    Fakes_Recibir("RPM 100\n");
    Periodo(50U);
    CHECK(Tx("RPM=100.000;TEMP=25.00;ESTADO=MANUAL\n") && fakes.motor == 1000U);
    Periodo(92U);
    CHECK(Tx("RPM=184.000;TEMP=25.00;ESTADO=MANUAL\n") && fakes.motor == 1000U);
    CHECK(fakes.motor_rechazos == 0U);
    return true;
}

static bool S4_VolverAAutomaticoEsperaConsignaNueva(void)
{
    Arrancar(false);
    Fakes_Recibir("RPM 100\n");
    Periodo(0U);
    CHECK(fakes.motor == 400U);
    Periodo(0U);
    CHECK(fakes.motor == 500U);
    Periodo(0U);
    CHECK(fakes.motor == 600U);
    fakes.selector = true;
    fakes.pot = 2048U;
    Periodo(0U);
    CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=MANUAL\n") && fakes.motor == 500U);
    fakes.selector = false;
    Periodo(0U);
    CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=AUTO_ESPERA\n") && fakes.motor == 0U);
    Aplicacion_Iterar();
    Periodo(0U);
    CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=AUTO_ESPERA\n") && fakes.motor == 0U);
    Fakes_Recibir("RPM 100\n");
    Periodo(0U);
    CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=AUTO_ACTIVO\n") && fakes.motor == 400U);
    Periodo(0U);
    CHECK(fakes.motor == 500U);
    return true;
}

/* Mantiene la temperatura del sensor durante los 4 periodos del filtro. */
static bool Mantener(int32_t lm35, bool activa_antes_del_cuarto, bool activa_al_final)
{
    fakes.lm35 = lm35;
    for (unsigned i = 0U; i < TEMPERATURA_MUESTRAS; ++i) {
        const bool esperada = i + 1U < TEMPERATURA_MUESTRAS ? activa_antes_del_cuarto : activa_al_final;
        Periodo(0U);
        CHECK(fakes.led == esperada && fakes.buzzer == esperada);
    }
    return true;
}

static bool S5_AlarmaConHisteresis(void)
{
    Arrancar(true);
    fakes.pot = 2048U;
    CHECK(Mantener(4500, false, false));
    CHECK(Mantener(4501, false, true));
    CHECK(Tx("RPM=0.000;TEMP=45.01;ESTADO=MANUAL\n") && fakes.motor == 500U);
    CHECK(Mantener(4400, true, true));
    CHECK(Mantener(4304, true, true));
    CHECK(Mantener(4300, true, false));
    CHECK(Tx("RPM=0.000;TEMP=43.00;ESTADO=MANUAL\n"));
    CHECK(Mantener(4400, false, false));
    CHECK(Mantener(4500, false, false));
    fakes.selector = false;
    CHECK(Mantener(4600, true, true));
    CHECK(Tx("RPM=0.000;TEMP=46.00;ESTADO=AUTO_ESPERA\n") && fakes.motor == 0U);
    return true;
}

static bool S6_ConsignasInvalidasYCero(void)
{
    static const char *const invalidas[] = {
        "RPM 186\n", "RPM x\n", "RPM 100 con texto de sobra\n", "RPM 1000\n", "rpm 100\n"
    };
    Arrancar(false);
    for (unsigned i = 0U; i < sizeof(invalidas) / sizeof(invalidas[0]); ++i) {
        Fakes_Recibir(invalidas[i]);
        Periodo(0U);
        CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=AUTO_ESPERA\n") && fakes.motor == 0U);
    }
    Fakes_Recibir("RPM 0\n");
    Periodo(0U);
    CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=AUTO_ACTIVO\n") && fakes.motor == 0U);
    Fakes_Recibir("RPM 50\r\nRPM 186\nRPM 100\n");
    Periodo(0U);
    CHECK(fakes.motor == 400U);
    Fakes_Recibir("RPM 186\n");
    Periodo(0U);
    CHECK(Tx("RPM=0.000;TEMP=25.00;ESTADO=AUTO_ACTIVO\n") && fakes.motor == 500U);
    Fakes_Recibir("RPM 0\n");
    Periodo(0U);
    CHECK(fakes.motor == 0U);
    Fakes_Recibir("RPM 185\n");
    Periodo(0U);
    CHECK(fakes.motor == 740U);
    return true;
}

static bool S7_MandoAutomaticoSaturado(void)
{
    bool llego_al_maximo = false;
    Arrancar(false);
    Fakes_Recibir("RPM 185\n");
    for (unsigned i = 0U; i < 30U; ++i) {
        Periodo(0U);
        CHECK(fakes.motor <= 1000U);
        llego_al_maximo = llego_al_maximo || fakes.motor == 1000U;
    }
    CHECK(llego_al_maximo && fakes.motor == 1000U && fakes.motor_rechazos == 0U);
    return true;
}

static bool S8_FalloDeArranque(void)
{
    static const char *const orden[] = {
        "Reloj_Init", "Adquisicion_ADC_DMA_Init", "Motor_Init", "LED_Init", "Buzzer_Init", NULL
    };
    Fakes_Reiniciar();
    fakes.adc_init_ok = false;
    fakes.motor = 777U;
    CHECK(!Aplicacion_Inicializar());
    CHECK(Fakes_LlamadasSon(orden));
    CHECK(fakes.motor == 0U && fakes.motor_escrituras == 1U && fakes.led && fakes.buzzer);
    return true;
}

int main(void)
{
    const Caso casos[] = {
        {"S1 arranque en automatico, consigna y regulacion", S1_ArranqueEnAutomatico},
        {"S2 la consigna del cambio de modo se descarta", S2_ConsignaDelCambioDeModoSeDescarta},
        {"S3 manual sigue al potenciometro entre periodos", S3_ManualSigueAlPotenciometro},
        {"S4 volver a automatico espera consigna nueva", S4_VolverAAutomaticoEsperaConsignaNueva},
        {"S5 alarma con histeresis 45/43", S5_AlarmaConHisteresis},
        {"S6 consignas invalidas y consigna cero", S6_ConsignasInvalidasYCero},
        {"S7 mando automatico saturado en 1000", S7_MandoAutomaticoSaturado},
        {"S8 fallo de arranque", S8_FalloDeArranque}
    };
    EJECUTAR_CASOS(casos);
}
