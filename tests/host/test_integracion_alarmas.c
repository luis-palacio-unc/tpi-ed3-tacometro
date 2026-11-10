#include "Driver_Alarmas.h"
#include "Procesamiento.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

typedef struct { unsigned llamadas; bool ultimo_solicitado; } SalidaEmulada;
static SalidaEmulada led, buzzer;

static void Preparar(void)
{
    led = (SalidaEmulada){0U, false};
    buzzer = (SalidaEmulada){0U, false};
}

static void EscribirLed(bool activar)
{
    ++led.llamadas;
    led.ultimo_solicitado = activar;
}

static void EscribirBuzzer(bool activar)
{
    ++buzzer.llamadas;
    buzzer.ultimo_solicitado = activar;
}

/* Adaptacion de composicion para prueba; no introducir Aplicacion en el servicio. */
static bool EmitirDecision(const Driver_Alarmas_Salidas *salidas,
                           Procesamiento_EstadoAlarma decision, Driver_Alarmas_Estado *resultado)
{
    if (decision != PROCESAMIENTO_ALARMA_ACTIVA && decision != PROCESAMIENTO_ALARMA_INACTIVA) {
        return false;
    }
    *resultado = Driver_Alarmas_Aplicar(salidas, decision == PROCESAMIENTO_ALARMA_ACTIVA);
    return true;
}

static bool DecisionesEvaluables(void)
{
    Preparar();
    const Driver_Alarmas_Salidas salidas = {EscribirLed, EscribirBuzzer};
    Temperatura_Lectura lectura;
    Driver_Alarmas_Estado resultado = DRIVER_ALARMAS_ERROR_ARGUMENTO;
    CHECK(Temperatura_Actualizar(&lectura, 2534, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    Procesamiento_EstadoAlarma decision = Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2500);
    CHECK(decision == PROCESAMIENTO_ALARMA_ACTIVA);
    CHECK(EmitirDecision(&salidas, decision, &resultado) && resultado == DRIVER_ALARMAS_ACEPTADA);
    CHECK(led.llamadas == 1U && buzzer.llamadas == 1U && led.ultimo_solicitado && buzzer.ultimo_solicitado);
    CHECK(Temperatura_Actualizar(&lectura, 2500, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    decision = Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2500);
    CHECK(decision == PROCESAMIENTO_ALARMA_INACTIVA);
    CHECK(EmitirDecision(&salidas, decision, &resultado) && resultado == DRIVER_ALARMAS_ACEPTADA);
    CHECK(led.llamadas == 2U && buzzer.llamadas == 2U && !led.ultimo_solicitado && !buzzer.ultimo_solicitado);
    return true;
}

static bool EstadosNoEvaluables(void)
{
    Preparar();
    const Driver_Alarmas_Salidas salidas = {EscribirLed, EscribirBuzzer};
    const Temperatura_EstadoFuente fuentes[] = {TEMPERATURA_FUENTE_SIN_DATOS, TEMPERATURA_FUENTE_ERROR, (Temperatura_EstadoFuente)99};
    const Procesamiento_EstadoAlarma esperados[] = {PROCESAMIENTO_ALARMA_SIN_DATOS, PROCESAMIENTO_ALARMA_ERROR_FUENTE, PROCESAMIENTO_ALARMA_ERROR_ARGUMENTO};
    Driver_Alarmas_Estado resultado = DRIVER_ALARMAS_ERROR_ARGUMENTO;
    for (size_t i = 0U; i < sizeof(fuentes) / sizeof(fuentes[0]); ++i) {
        Temperatura_Lectura lectura;
        (void)Temperatura_Actualizar(&lectura, INT32_MAX, fuentes[i]);
        const Procesamiento_EstadoAlarma decision = Procesamiento_EvaluarAlarmaTemperatura(&lectura, INT32_MIN);
        CHECK(decision == esperados[i]);
        CHECK(!EmitirDecision(&salidas, decision, &resultado));
        CHECK(resultado == DRIVER_ALARMAS_ERROR_ARGUMENTO && led.llamadas == 0U && buzzer.llamadas == 0U);
    }
    CHECK(!EmitirDecision(&salidas, Procesamiento_EvaluarAlarmaTemperatura(NULL, 0), &resultado));
    CHECK(!EmitirDecision(&salidas, (Procesamiento_EstadoAlarma)99, &resultado));
    CHECK(led.llamadas == 0U && buzzer.llamadas == 0U);
    return true;
}

static bool ArgumentoInvalidoYRecuperacion(void)
{
    Preparar();
    const Driver_Alarmas_Salidas valida = {EscribirLed, EscribirBuzzer};
    const Driver_Alarmas_Salidas invalida = {EscribirLed, NULL};
    const Temperatura_Lectura lectura = {5000, TEMPERATURA_VALIDA};
    const Procesamiento_EstadoAlarma decision = Procesamiento_EvaluarAlarmaTemperatura(&lectura, 4000);
    Driver_Alarmas_Estado resultado = DRIVER_ALARMAS_ACEPTADA;
    CHECK(EmitirDecision(&invalida, decision, &resultado) && resultado == DRIVER_ALARMAS_ERROR_ARGUMENTO);
    CHECK(decision == PROCESAMIENTO_ALARMA_ACTIVA && led.llamadas == 0U && buzzer.llamadas == 0U);
    CHECK(EmitirDecision(&valida, decision, &resultado) && resultado == DRIVER_ALARMAS_ACEPTADA);
    CHECK(led.llamadas == 1U && buzzer.llamadas == 1U && led.ultimo_solicitado && buzzer.ultimo_solicitado);
    return true;
}

static bool PerdidaDeLecturaNoEmiteApagado(void)
{
    Preparar();
    const Driver_Alarmas_Salidas salidas = {EscribirLed, EscribirBuzzer};
    Temperatura_Lectura lectura;
    Driver_Alarmas_Estado resultado = DRIVER_ALARMAS_ERROR_ARGUMENTO;
    CHECK(Temperatura_Actualizar(&lectura, 5000, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(EmitirDecision(&salidas, Procesamiento_EvaluarAlarmaTemperatura(&lectura, 4000), &resultado));
    CHECK(resultado == DRIVER_ALARMAS_ACEPTADA);
    CHECK(Temperatura_Actualizar(&lectura, 0, TEMPERATURA_FUENTE_ERROR) == TEMPERATURA_ERROR_FUENTE);
    const Procesamiento_EstadoAlarma decision = Procesamiento_EvaluarAlarmaTemperatura(&lectura, 4000);
    CHECK(decision == PROCESAMIENTO_ALARMA_ERROR_FUENTE);
    CHECK(!EmitirDecision(&salidas, decision, &resultado));
    CHECK(led.llamadas == 1U && buzzer.llamadas == 1U && led.ultimo_solicitado && buzzer.ultimo_solicitado);
    CHECK(Temperatura_Actualizar(&lectura, 3000, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(EmitirDecision(&salidas, Procesamiento_EvaluarAlarmaTemperatura(&lectura, 4000), &resultado));
    CHECK(resultado == DRIVER_ALARMAS_ACEPTADA);
    CHECK(led.llamadas == 2U && buzzer.llamadas == 2U && !led.ultimo_solicitado && !buzzer.ultimo_solicitado);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S6 decisiones evaluables", DecisionesEvaluables}, {"S7 no evaluables", EstadosNoEvaluables},
        {"S4/S6 argumento/recuperacion", ArgumentoInvalidoYRecuperacion},
        {"S7 no apagado ante lectura invalida", PerdidaDeLecturaNoEmiteApagado}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
