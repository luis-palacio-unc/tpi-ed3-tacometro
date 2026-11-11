/* Pruebas host del servicio Comunicacion (B18/I2) con dispositivos simulados. */
#include "Comunicacion.h"
#include "fakes_dispositivos.h"
#include <string.h>

static void Preparar(void)
{
    Fakes_Reiniciar();
    Comunicacion_Init();
}

/* Recibe texto y exige exactamente una consigna con el valor dado. */
static bool UnaConsigna(const char *texto, uint32_t esperada)
{
    uint32_t rpm = 999U;
    Fakes_Recibir(texto);
    CHECK(Comunicacion_RecibirConsigna(&rpm) && rpm == esperada);
    CHECK(!Comunicacion_RecibirConsigna(&rpm) && rpm == esperada);
    return true;
}

/* Recibe texto y exige que no produzca ninguna consigna ni toque el destino. */
static bool Ninguna(const char *texto)
{
    uint32_t rpm = 999U;
    Fakes_Recibir(texto);
    CHECK(!Comunicacion_RecibirConsigna(&rpm) && rpm == 999U);
    CHECK(fakes.rx_leidos == fakes.rx_bytes);
    return true;
}

static bool S1_ConsignasValidas(void)
{
    static const char *const orden[] = {"UART0_Init", NULL};
    Preparar();
    CHECK(Fakes_LlamadasSon(orden));
    CHECK(UnaConsigna("RPM 0\n", 0U));
    CHECK(UnaConsigna("RPM 7\r\n", 7U));
    CHECK(UnaConsigna("RPM 100\n", 100U));
    CHECK(UnaConsigna("RPM 184\n", 184U));
    CHECK(UnaConsigna("RPM 185\r\n", 185U));
    CHECK(UnaConsigna("RPM 007\n", 7U));
    return true;
}

static bool S2_LineasInvalidas(void)
{
    static const char *const invalidas[] = {
        "RPM 186\n", "RPM 999\n", "RPM 1000\n", "RPM x\n", "RPM 1x\n", "RPM \n", "RPM\n",
        "RPM  10\n", "rpm 10\n", "RPM=10\n", "RPM -1\n", "RPM 10 \n", " RPM 10\n", "\n", "\r\n",
        "RPM 10\r\r\n", "RPM 0100\n", "RPM 1850\r\n", "RPM 100 y mucho texto de mas\n", "XXXXXXXXXRPM 1\n"
    };
    Preparar();
    for (unsigned i = 0U; i < sizeof(invalidas) / sizeof(invalidas[0]); ++i) {
        CHECK(Ninguna(invalidas[i]));
        CHECK(UnaConsigna("RPM 42\n", 42U));
    }
    return true;
}

static bool S3_LineaPartidaYVarias(void)
{
    uint32_t rpm = 999U;
    Preparar();
    CHECK(Ninguna("RP"));
    CHECK(Ninguna("M 1"));
    CHECK(UnaConsigna("23\r\n", 123U));
    Fakes_Recibir("RPM 10\nRPM 300\nRPM 20\r\nRPM 3");
    CHECK(Comunicacion_RecibirConsigna(&rpm) && rpm == 10U);
    CHECK(Comunicacion_RecibirConsigna(&rpm) && rpm == 20U);
    CHECK(!Comunicacion_RecibirConsigna(&rpm) && rpm == 20U);
    CHECK(UnaConsigna("0\n", 30U));
    return true;
}

static bool S4_Descartar(void)
{
    Preparar();
    CHECK(Ninguna("RPM 1"));
    Comunicacion_Descartar();
    CHECK(Ninguna("50\n"));
    Fakes_Recibir("RPM 60\nRPM 7");
    Comunicacion_Descartar();
    CHECK(fakes.rx_leidos == fakes.rx_bytes);
    CHECK(Ninguna("0\n"));
    CHECK(UnaConsigna("RPM 80\n", 80U));
    CHECK(Ninguna("RPM 9"));
    Comunicacion_Init();
    CHECK(UnaConsigna("RPM 5\n", 5U));
    return true;
}

static bool Trama(uint32_t rpm, int32_t temperatura, Comunicacion_Estado estado,
                  const char *esperada)
{
    Fakes_VaciarTx();
    CHECK(Comunicacion_EnviarTelemetria(rpm, temperatura, estado));
    CHECK(strcmp(fakes.tx, esperada) == 0 && fakes.tx_bytes == strlen(esperada));
    return true;
}

static bool S5_TelemetriaExacta(void)
{
    Preparar();
    CHECK(Trama(0U, 0, COMUNICACION_ESTADO_MANUAL, "RPM=0.000;TEMP=0.00;ESTADO=MANUAL\n"));
    CHECK(Trama(100000U, 2500, COMUNICACION_ESTADO_AUTO_ACTIVO,
                "RPM=100.000;TEMP=25.00;ESTADO=AUTO_ACTIVO\n"));
    CHECK(Trama(2000U, 4501, COMUNICACION_ESTADO_AUTO_ESPERA,
                "RPM=2.000;TEMP=45.01;ESTADO=AUTO_ESPERA\n"));
    CHECK(Trama(185007U, 5, COMUNICACION_ESTADO_MANUAL, "RPM=185.007;TEMP=0.05;ESTADO=MANUAL\n"));
    CHECK(Trama(1U, -50, COMUNICACION_ESTADO_MANUAL, "RPM=0.001;TEMP=-0.50;ESTADO=MANUAL\n"));
    CHECK(Trama(12340U, -1234, COMUNICACION_ESTADO_MANUAL, "RPM=12.340;TEMP=-12.34;ESTADO=MANUAL\n"));
    CHECK(Trama(UINT32_MAX, INT32_MIN, COMUNICACION_ESTADO_AUTO_ACTIVO,
                "RPM=4294967.295;TEMP=-21474836.48;ESTADO=AUTO_ACTIVO\n"));
    CHECK(fakes.tx_bytes == COMUNICACION_TELEMETRIA_MAX);
    CHECK(Trama(0U, INT32_MAX, COMUNICACION_ESTADO_MANUAL, "RPM=0.000;TEMP=21474836.47;ESTADO=MANUAL\n"));
    return true;
}

static bool S6_SinLugarEnTransmision(void)
{
    Preparar();
    fakes.tx_libres = 33U;
    CHECK(!Comunicacion_EnviarTelemetria(0U, 0, COMUNICACION_ESTADO_MANUAL));
    CHECK(fakes.tx_bytes == 33U);
    fakes.tx_libres = 34U;
    fakes.tx_bytes = 0U;
    CHECK(Comunicacion_EnviarTelemetria(0U, 0, COMUNICACION_ESTADO_MANUAL));
    CHECK(fakes.tx_bytes == 34U);
    return true;
}

int main(void)
{
    const Caso casos[] = {
        {"S1 consignas validas", S1_ConsignasValidas},
        {"S2 lineas invalidas, fuera de rango o largas", S2_LineasInvalidas},
        {"S3 linea partida y varias consignas", S3_LineaPartidaYVarias},
        {"S4 descartar", S4_Descartar},
        {"S5 telemetria exacta", S5_TelemetriaExacta},
        {"S6 sin lugar en transmision", S6_SinLugarEnTransmision}
    };
    EJECUTAR_CASOS(casos);
}
