#include "Procesamiento.h"
#include "Temperatura.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static bool Esperar(int32_t temperatura, Temperatura_Estado fuente, int32_t umbral,
                    Procesamiento_EstadoAlarma esperado)
{
    const Temperatura_Lectura lectura = {temperatura, fuente};
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, umbral) == esperado);
    CHECK(lectura.centesimas_c == temperatura && lectura.estado == fuente);
    return true;
}

static bool UmbralConfigurable(void)
{
    const int32_t umbrales[] = {-5000, -1, 0, 2500, 10000};
    for (size_t i = 0U; i < sizeof(umbrales) / sizeof(umbrales[0]); ++i) {
        CHECK(Esperar(umbrales[i] - 1, TEMPERATURA_VALIDA, umbrales[i], PROCESAMIENTO_ALARMA_INACTIVA));
        CHECK(Esperar(umbrales[i], TEMPERATURA_VALIDA, umbrales[i], PROCESAMIENTO_ALARMA_INACTIVA));
        CHECK(Esperar(umbrales[i] + 1, TEMPERATURA_VALIDA, umbrales[i], PROCESAMIENTO_ALARMA_ACTIVA));
    }
    return true;
}

static bool ExtremosInt32(void)
{
    CHECK(Esperar(INT32_MIN, TEMPERATURA_VALIDA, INT32_MAX, PROCESAMIENTO_ALARMA_INACTIVA));
    CHECK(Esperar(INT32_MAX, TEMPERATURA_VALIDA, INT32_MIN, PROCESAMIENTO_ALARMA_ACTIVA));
    CHECK(Esperar(INT32_MIN, TEMPERATURA_VALIDA, INT32_MIN, PROCESAMIENTO_ALARMA_INACTIVA));
    CHECK(Esperar(INT32_MAX, TEMPERATURA_VALIDA, INT32_MAX, PROCESAMIENTO_ALARMA_INACTIVA));
    return true;
}

static bool CeroValidoVsAusencia(void)
{
    CHECK(Esperar(0, TEMPERATURA_VALIDA, -1, PROCESAMIENTO_ALARMA_ACTIVA));
    CHECK(Esperar(0, TEMPERATURA_VALIDA, 0, PROCESAMIENTO_ALARMA_INACTIVA));
    CHECK(Esperar(0, TEMPERATURA_SIN_DATOS, -1, PROCESAMIENTO_ALARMA_SIN_DATOS));
    CHECK(Esperar(INT32_MAX, TEMPERATURA_SIN_DATOS, INT32_MIN, PROCESAMIENTO_ALARMA_SIN_DATOS));
    return true;
}

static bool ErroresYArgumentos(void)
{
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(NULL, INT32_MIN) == PROCESAMIENTO_ALARMA_ERROR_ARGUMENTO);
    CHECK(Esperar(INT32_MAX, TEMPERATURA_ERROR_FUENTE, INT32_MIN, PROCESAMIENTO_ALARMA_ERROR_FUENTE));
    CHECK(Esperar(INT32_MIN, TEMPERATURA_ERROR_FUENTE, INT32_MAX, PROCESAMIENTO_ALARMA_ERROR_FUENTE));
    CHECK(Esperar(0, TEMPERATURA_ERROR_ARGUMENTO, 100, PROCESAMIENTO_ALARMA_ERROR_ARGUMENTO));
    CHECK(Esperar(INT32_MAX, (Temperatura_Estado)99, INT32_MIN, PROCESAMIENTO_ALARMA_ERROR_ARGUMENTO));
    return true;
}

static bool SecuenciaInvalidacionYRecuperacion(void)
{
    Temperatura_Lectura lectura = {2534, TEMPERATURA_VALIDA};
    Procesamiento_EstadoAlarma decision = Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2500);
    CHECK(decision == PROCESAMIENTO_ALARMA_ACTIVA);
    lectura.estado = TEMPERATURA_ERROR_FUENTE;
    decision = Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2500);
    CHECK(decision == PROCESAMIENTO_ALARMA_ERROR_FUENTE);
    lectura.estado = TEMPERATURA_SIN_DATOS;
    decision = Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2500);
    CHECK(decision == PROCESAMIENTO_ALARMA_SIN_DATOS);
    lectura.estado = TEMPERATURA_VALIDA;
    lectura.centesimas_c = 2400;
    decision = Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2500);
    CHECK(decision == PROCESAMIENTO_ALARMA_INACTIVA);
    lectura.centesimas_c = 2600;
    decision = Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2500);
    CHECK(decision == PROCESAMIENTO_ALARMA_ACTIVA);
    return true;
}

static bool CambioUmbral(void)
{
    const Temperatura_Lectura lectura = {2534, TEMPERATURA_VALIDA};
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2500) == PROCESAMIENTO_ALARMA_ACTIVA);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2534) == PROCESAMIENTO_ALARMA_INACTIVA);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2535) == PROCESAMIENTO_ALARMA_INACTIVA);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, -725) == PROCESAMIENTO_ALARMA_ACTIVA);
    return true;
}

static bool EntradasInmutables(void)
{
    const Temperatura_Lectura lectura = {-725, TEMPERATURA_VALIDA};
    const Temperatura_Lectura copia = lectura;
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, -1000) == PROCESAMIENTO_ALARMA_ACTIVA);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, 0) == PROCESAMIENTO_ALARMA_INACTIVA);
    CHECK(lectura.estado == copia.estado && lectura.centesimas_c == copia.centesimas_c);
    return true;
}

static bool LecturasIndependientes(void)
{
    const Temperatura_Lectura a = {5000, TEMPERATURA_VALIDA};
    const Temperatura_Lectura b = {INT32_MAX, TEMPERATURA_ERROR_FUENTE};
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&a, 4000) == PROCESAMIENTO_ALARMA_ACTIVA);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&b, 4000) == PROCESAMIENTO_ALARMA_ERROR_FUENTE);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&a, 4000) == PROCESAMIENTO_ALARMA_ACTIVA);
    CHECK(a.centesimas_c == 5000 && a.estado == TEMPERATURA_VALIDA);
    CHECK(b.centesimas_c == INT32_MAX && b.estado == TEMPERATURA_ERROR_FUENTE);
    return true;
}

static bool IntegracionServicioTemperatura(void)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Inicializar(&lectura));
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2500) == PROCESAMIENTO_ALARMA_SIN_DATOS);
    CHECK(Temperatura_Actualizar(&lectura, 2534, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2500) == PROCESAMIENTO_ALARMA_ACTIVA);
    CHECK(Temperatura_Actualizar(&lectura, 2534, TEMPERATURA_FUENTE_ERROR) == TEMPERATURA_ERROR_FUENTE);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2500) == PROCESAMIENTO_ALARMA_ERROR_FUENTE);
    CHECK(Temperatura_Actualizar(&lectura, 0, TEMPERATURA_FUENTE_SIN_DATOS) == TEMPERATURA_SIN_DATOS);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, 2500) == PROCESAMIENTO_ALARMA_SIN_DATOS);
    CHECK(Temperatura_Actualizar(&lectura, -725, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, -725) == PROCESAMIENTO_ALARMA_INACTIVA);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, -726) == PROCESAMIENTO_ALARMA_ACTIVA);
    return true;
}

static bool IntegracionErrorServicio(void)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Actualizar(&lectura, INT32_MAX, (Temperatura_EstadoFuente)99) == TEMPERATURA_ERROR_ARGUMENTO);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, INT32_MIN) == PROCESAMIENTO_ALARMA_ERROR_ARGUMENTO);
    CHECK(Temperatura_Actualizar(&lectura, INT32_MIN, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, INT32_MIN) == PROCESAMIENTO_ALARMA_INACTIVA);
    CHECK(Temperatura_Actualizar(&lectura, INT32_MAX, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Procesamiento_EvaluarAlarmaTemperatura(&lectura, INT32_MIN) == PROCESAMIENTO_ALARMA_ACTIVA);
    return true;
}

static bool ContextosIguales(const Procesamiento_Contexto *a, const Procesamiento_Contexto *b)
{
    return a->estado == b->estado && a->orden_actual == b->orden_actual &&
           a->orden_entrada_auto == b->orden_entrada_auto &&
           a->orden_ultima_consigna == b->orden_ultima_consigna &&
           a->consigna_rpm == b->consigna_rpm && a->requiere_entrada_auto == b->requiere_entrada_auto;
}

static bool ModoNoInterferido(Procesamiento_Modo modo, bool auto_activo)
{
    Procesamiento_Contexto contexto;
    Procesamiento_Salida salida;
    CHECK(Procesamiento_Inicializar(&contexto, modo, 0U));
    Procesamiento_Entradas entradas = {modo, 375U, 1U, NULL};
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &salida));
    const Procesamiento_Consigna consigna = {120U, 2U, true};
    if (auto_activo) {
        entradas.orden_actual = 2U;
        entradas.consigna = &consigna;
        CHECK(Procesamiento_Actualizar(&contexto, &entradas, &salida));
        CHECK(salida.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA && salida.consigna_rpm == 120U);
    }
    const Procesamiento_Contexto previo = contexto;
    const Procesamiento_Salida salida_previa = salida;
    CHECK(Esperar(5000, TEMPERATURA_VALIDA, 4000, PROCESAMIENTO_ALARMA_ACTIVA));
    CHECK(Esperar(5000, TEMPERATURA_SIN_DATOS, 4000, PROCESAMIENTO_ALARMA_SIN_DATOS));
    CHECK(Esperar(5000, TEMPERATURA_ERROR_FUENTE, 4000, PROCESAMIENTO_ALARMA_ERROR_FUENTE));
    CHECK(ContextosIguales(&contexto, &previo));
    CHECK(salida.estado == salida_previa.estado && salida.fuente == salida_previa.fuente);
    CHECK(salida.mando_manual_por_mil == salida_previa.mando_manual_por_mil &&
          salida.consigna_rpm == salida_previa.consigna_rpm);
    entradas.orden_actual += 1U;
    entradas.consigna = NULL;
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &salida));
    if (modo == PROCESAMIENTO_MODO_MANUAL) {
        CHECK(salida.fuente == PROCESAMIENTO_FUENTE_MANUAL && salida.mando_manual_por_mil == 375U);
    } else if (auto_activo) {
        CHECK(salida.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA && salida.consigna_rpm == 120U);
    } else {
        CHECK(salida.fuente == PROCESAMIENTO_FUENTE_CERO && salida.consigna_rpm == 0U);
    }
    return true;
}

static bool ModoManualConAlarma(void) { return ModoNoInterferido(PROCESAMIENTO_MODO_MANUAL, false); }
static bool AutoEsperaConAlarma(void) { return ModoNoInterferido(PROCESAMIENTO_MODO_AUTOMATICO, false); }
static bool AutoActivoConAlarma(void) { return ModoNoInterferido(PROCESAMIENTO_MODO_AUTOMATICO, true); }

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S1 umbral configurable", UmbralConfigurable},
        {"S2 extremos int32", ExtremosInt32},
        {"S3 cero valido/ausencia", CeroValidoVsAusencia},
        {"S3 errores/argumentos", ErroresYArgumentos},
        {"S4 invalidacion/recuperacion", SecuenciaInvalidacionYRecuperacion},
        {"S2 cambio de umbral", CambioUmbral},
        {"S5 entradas inmutables", EntradasInmutables},
        {"S5 lecturas independientes", LecturasIndependientes},
        {"S6 integracion B05", IntegracionServicioTemperatura},
        {"S6 errores B05", IntegracionErrorServicio},
        {"S7 manual", ModoManualConAlarma},
        {"S7 auto espera", AutoEsperaConAlarma},
        {"S7 auto activo", AutoActivoConAlarma}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) {
            fprintf(stderr, "FAIL %s\n", casos[i].nombre);
            return 1;
        }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
