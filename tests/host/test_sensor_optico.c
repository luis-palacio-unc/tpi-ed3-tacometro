#include "Sensor_Optico.h"
#include "Medicion_Velocidad.h"
#include "Medicion.h"
#include "lpc17xx_timer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static uint32_t cuenta_base, cuenta_eventos;
static unsigned consultas_base, consultas_eventos;
static bool error_modelo;

uint32_t TIM_ReadTimer(LPC_TIM_TypeDef *timer)
{
    if (timer == LPC_TIM1) { ++consultas_eventos; return cuenta_eventos; }
    if (timer == LPC_TIM0) { ++consultas_base; return cuenta_base; }
    error_modelo = true;
    return 0U;
}

static void Preparar(void)
{
    cuenta_base = UINT32_C(0xA5A5A5A5);
    cuenta_eventos = 0U;
    consultas_base = 0U; consultas_eventos = 0U; error_modelo = false;
}

static bool LecturasExactasYBaseIndependiente(void)
{
    Preparar();
    const uint32_t valores[] = {0U, 1U, 60U, 185U, 200U, UINT32_MAX};
    for (size_t i = 0U; i < sizeof(valores) / sizeof(valores[0]); ++i) {
        cuenta_eventos = valores[i];
        CHECK(SensorOptico_Read() == valores[i]);
        CHECK(cuenta_eventos == valores[i] && cuenta_base == UINT32_C(0xA5A5A5A5));
    }
    CHECK(consultas_eventos == 6U && consultas_base == 0U && !error_modelo);
    return true;
}

static bool RepeticionYAvance(void)
{
    Preparar();
    const uint32_t valores[] = {10U, 10U, 70U, 70U, 71U};
    for (size_t i = 0U; i < sizeof(valores) / sizeof(valores[0]); ++i) {
        cuenta_eventos = valores[i];
        CHECK(SensorOptico_Read() == valores[i]);
        CHECK(SensorOptico_Read() == valores[i] && cuenta_eventos == valores[i]);
    }
    CHECK(consultas_eventos == 10U && consultas_base == 0U && !error_modelo);
    return true;
}

static bool VentanaUnSegundo(void)
{
    const struct { uint32_t pulsos; uint32_t rpm_milesimas; } casos[] = {
        {0U, 0U}, {60U, 60000U}, {185U, 185000U}, {200U, 200000U}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        Preparar();
        Medicion_Velocidad_Lectura lectura;
        CHECK(Medicion_Velocidad_Inicializar(&lectura));
        cuenta_eventos = 12345U;
        const uint32_t anterior = SensorOptico_Read();
        CHECK(lectura.estado == MEDICION_VELOCIDAD_SIN_DATOS);
        cuenta_eventos = 12345U + casos[i].pulsos;
        const uint32_t actual = SensorOptico_Read();
        const uint32_t pulsos = actual - anterior;
        CHECK(pulsos == casos[i].pulsos);
        CHECK(Medicion_Velocidad_Actualizar(&lectura, pulsos, 1000000U,
            TPI_MEDICION_PULSOS_POR_VUELTA) == MEDICION_VELOCIDAD_VALIDA);
        CHECK(lectura.estado == MEDICION_VELOCIDAD_VALIDA && lectura.rpm_milesimas == casos[i].rpm_milesimas);
        CHECK(cuenta_eventos == actual && consultas_eventos == 2U && consultas_base == 0U && !error_modelo);
    }
    return true;
}

static bool VentanaCruzaRollover(void)
{
    Preparar();
    cuenta_eventos = UINT32_MAX - 29U;
    const uint32_t anterior = SensorOptico_Read();
    cuenta_eventos = 30U;
    const uint32_t actual = SensorOptico_Read();
    const uint32_t pulsos = actual - anterior;
    Medicion_Velocidad_Lectura lectura;
    CHECK(pulsos == 60U);
    CHECK(Medicion_Velocidad_Actualizar(&lectura, pulsos, 1000000U,
        TPI_MEDICION_PULSOS_POR_VUELTA) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(lectura.estado == MEDICION_VELOCIDAD_VALIDA && lectura.rpm_milesimas == 60000U);
    CHECK(cuenta_eventos == 30U && cuenta_base == UINT32_C(0xA5A5A5A5));
    CHECK(consultas_eventos == 2U && consultas_base == 0U && !error_modelo);
    return true;
}

static bool NuevaEpocaReferenciaExterna(void)
{
    Preparar();
    cuenta_eventos = 700U;
    CHECK(SensorOptico_Read() == 700U);
    cuenta_eventos = 0U;
    const uint32_t nueva_referencia = SensorOptico_Read();
    cuenta_eventos = 60U;
    const uint32_t actual = SensorOptico_Read();
    const uint32_t pulsos = actual - nueva_referencia;
    Medicion_Velocidad_Lectura lectura;
    CHECK(pulsos == 60U);
    CHECK(Medicion_Velocidad_Actualizar(&lectura, pulsos, 1000000U,
        TPI_MEDICION_PULSOS_POR_VUELTA) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(lectura.rpm_milesimas == 60000U && cuenta_eventos == 60U);
    CHECK(consultas_eventos == 3U && consultas_base == 0U && !error_modelo);
    return true;
}

static bool DuracionYFactorExplicitos(void)
{
    Preparar();
    cuenta_eventos = 100U;
    const uint32_t anterior = SensorOptico_Read();
    cuenta_eventos = 112U;
    const uint32_t actual = SensorOptico_Read();
    const uint32_t pulsos = actual - anterior;
    Medicion_Velocidad_Lectura lectura;
    CHECK(pulsos == 12U);
    CHECK(Medicion_Velocidad_Actualizar(&lectura, pulsos, 2000000U, 12U) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(lectura.rpm_milesimas == 30000U && cuenta_eventos == 112U);
    CHECK(consultas_eventos == 2U && consultas_base == 0U && !error_modelo);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S1 lecturas exactas/base independiente", LecturasExactasYBaseIndependiente},
        {"S2 repeticion/avance", RepeticionYAvance},
        {"S3 ventana 1s/B03", VentanaUnSegundo}, {"S4 rollover/B03", VentanaCruzaRollover},
        {"S2 nueva referencia externa", NuevaEpocaReferenciaExterna},
        {"S3 duracion/factor explicitos", DuracionYFactorExplicitos}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
