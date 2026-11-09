#include "Driver_Motor.h"
#include "Mandos.h"
#include "Procesamiento.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

typedef struct { unsigned llamadas; uint16_t ultimo_solicitado; bool acepta; } SalidaEmulada;

static bool Escribir(void *contexto, uint16_t mando_por_mil)
{
    SalidaEmulada *salida = contexto;
    ++salida->llamadas;
    salida->ultimo_solicitado = mando_por_mil;
    return salida->acepta;
}

/* Composicion de prueba: el servicio no conoce modos ni la consigna RPM. */
static bool EmitirSeleccion(const Driver_Motor_Salida *motor,
                           const Procesamiento_Salida *seleccion, Driver_Motor_Estado *resultado)
{
    if (seleccion->fuente == PROCESAMIENTO_FUENTE_CERO) {
        *resultado = Driver_Motor_SolicitarCero(motor);
        return true;
    }
    if (seleccion->fuente == PROCESAMIENTO_FUENTE_MANUAL) {
        *resultado = Driver_Motor_Aplicar(motor, seleccion->mando_manual_por_mil);
        return true;
    }
    return false;
}

/* Sólo un mando B04 valido habilita esta adaptacion manual de prueba. */
static bool PrepararManual(Procesamiento_Contexto *contexto, const Mandos_MandoManual *mando,
                          uint64_t orden, Procesamiento_Salida *seleccion)
{
    if (mando->estado != MANDOS_VALIDO) {
        return false;
    }
    const Procesamiento_Entradas entradas = {PROCESAMIENTO_MODO_MANUAL, mando->por_mil, orden, NULL};
    return Procesamiento_Actualizar(contexto, &entradas, seleccion);
}

static bool ManualDesdeEscalas(void)
{
    const uint32_t lecturas[] = {0U, 2048U, 341U, 4095U};
    const uint32_t escalas[] = {4095U, 4095U, 1023U, 4095U};
    const uint16_t esperados[] = {0U, 500U, 333U, 1000U};
    CHECK(DRIVER_MOTOR_MANDO_MAX == MANDOS_MANDO_MAX && DRIVER_MOTOR_MANDO_MAX == PROCESAMIENTO_MANDO_MAX);
    Procesamiento_Contexto contexto;
    CHECK(Procesamiento_Inicializar(&contexto, PROCESAMIENTO_MODO_MANUAL, 0U));
    SalidaEmulada emulada = {0U, 777U, true};
    const Driver_Motor_Salida motor = {Escribir, &emulada};
    for (size_t i = 0U; i < sizeof(lecturas) / sizeof(lecturas[0]); ++i) {
        Mandos_MandoManual mando;
        CHECK(Mandos_NormalizarManual(&mando, lecturas[i], escalas[i]) == MANDOS_VALIDO);
        Procesamiento_Salida seleccion;
        CHECK(PrepararManual(&contexto, &mando, i + 1U, &seleccion));
        CHECK(seleccion.fuente == PROCESAMIENTO_FUENTE_MANUAL && seleccion.consigna_rpm == 0U);
        Driver_Motor_Estado resultado = DRIVER_MOTOR_ERROR_ARGUMENTO;
        CHECK(EmitirSeleccion(&motor, &seleccion, &resultado));
        CHECK(resultado == DRIVER_MOTOR_ACEPTADA);
        CHECK(emulada.llamadas == i + 1U && emulada.ultimo_solicitado == esperados[i]);
    }
    return true;
}

static bool EntradaAutoSolicitaCero(void)
{
    Procesamiento_Contexto contexto;
    CHECK(Procesamiento_Inicializar(&contexto, PROCESAMIENTO_MODO_MANUAL, 0U));
    SalidaEmulada emulada = {0U, 0U, true};
    const Driver_Motor_Salida motor = {Escribir, &emulada};
    Procesamiento_Salida seleccion;
    Procesamiento_Entradas entradas = {PROCESAMIENTO_MODO_MANUAL, 900U, 1U, NULL};
    Driver_Motor_Estado resultado = DRIVER_MOTOR_ERROR_ARGUMENTO;
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &seleccion));
    CHECK(EmitirSeleccion(&motor, &seleccion, &resultado) && resultado == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 1U && emulada.ultimo_solicitado == 900U);
    const Procesamiento_Consigna antigua = {185U, 1U, true};
    entradas = (Procesamiento_Entradas){PROCESAMIENTO_MODO_AUTOMATICO, UINT16_MAX, 2U, &antigua};
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &seleccion));
    CHECK(seleccion.fuente == PROCESAMIENTO_FUENTE_CERO && seleccion.estado == PROCESAMIENTO_AUTO_ESPERA_CONSIGNA);
    CHECK(EmitirSeleccion(&motor, &seleccion, &resultado) && resultado == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 2U && emulada.ultimo_solicitado == 0U);
    entradas.orden_actual = 3U;
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &seleccion));
    CHECK(seleccion.fuente == PROCESAMIENTO_FUENTE_CERO);
    CHECK(EmitirSeleccion(&motor, &seleccion, &resultado) && resultado == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 3U && emulada.ultimo_solicitado == 0U);
    return true;
}

static bool AutomaticoNoUsaRPMComoPWM(void)
{
    Procesamiento_Contexto contexto;
    CHECK(Procesamiento_Inicializar(&contexto, PROCESAMIENTO_MODO_AUTOMATICO, 0U));
    SalidaEmulada emulada = {0U, 999U, true};
    const Driver_Motor_Salida motor = {Escribir, &emulada};
    Procesamiento_Salida seleccion;
    Procesamiento_Entradas entradas = {PROCESAMIENTO_MODO_AUTOMATICO, UINT16_MAX, 1U, NULL};
    Driver_Motor_Estado resultado = DRIVER_MOTOR_ERROR_ARGUMENTO;
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &seleccion));
    CHECK(seleccion.fuente == PROCESAMIENTO_FUENTE_CERO);
    CHECK(EmitirSeleccion(&motor, &seleccion, &resultado) && resultado == DRIVER_MOTOR_ACEPTADA);
    const Procesamiento_Consigna nueva = {185U, 2U, true};
    entradas.orden_actual = 2U;
    entradas.consigna = &nueva;
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &seleccion));
    CHECK(seleccion.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA && seleccion.consigna_rpm == 185U);
    CHECK(seleccion.mando_manual_por_mil == 0U);
    resultado = DRIVER_MOTOR_ERROR_SALIDA;
    CHECK(!EmitirSeleccion(&motor, &seleccion, &resultado));
    CHECK(resultado == DRIVER_MOTOR_ERROR_SALIDA && emulada.llamadas == 1U && emulada.ultimo_solicitado == 0U);
    return true;
}

static bool MandoNoValidoNoSeConsume(void)
{
    Procesamiento_Contexto contexto;
    CHECK(Procesamiento_Inicializar(&contexto, PROCESAMIENTO_MODO_MANUAL, 0U));
    SalidaEmulada emulada = {0U, 777U, true};
    const Driver_Motor_Salida motor = {Escribir, &emulada};
    Procesamiento_Salida seleccion;
    Mandos_MandoManual mando;
    CHECK(Mandos_Inicializar(&mando));
    const Mandos_Estado estados[] = {MANDOS_SIN_DATOS, MANDOS_ERROR_ARGUMENTO, MANDOS_ERROR_ESCALA, MANDOS_ERROR_LECTURA};
    Driver_Motor_Estado resultado = DRIVER_MOTOR_ERROR_ARGUMENTO;
    for (size_t i = 0U; i < sizeof(estados) / sizeof(estados[0]); ++i) {
        mando.estado = estados[i];
        mando.por_mil = 500U;
        if (PrepararManual(&contexto, &mando, i + 1U, &seleccion)) {
            CHECK(EmitirSeleccion(&motor, &seleccion, &resultado));
        }
        CHECK(resultado == DRIVER_MOTOR_ERROR_ARGUMENTO && emulada.llamadas == 0U);
    }
    CHECK(Mandos_NormalizarManual(&mando, 2048U, 4095U) == MANDOS_VALIDO);
    CHECK(PrepararManual(&contexto, &mando, 5U, &seleccion));
    CHECK(EmitirSeleccion(&motor, &seleccion, &resultado) && resultado == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 1U && emulada.ultimo_solicitado == 500U);
    return true;
}

static bool FalloDelProveedorObservable(void)
{
    Procesamiento_Contexto contexto;
    CHECK(Procesamiento_Inicializar(&contexto, PROCESAMIENTO_MODO_MANUAL, 0U));
    SalidaEmulada emulada = {0U, 0U, false};
    const Driver_Motor_Salida motor = {Escribir, &emulada};
    Procesamiento_Salida seleccion;
    Procesamiento_Entradas entradas = {PROCESAMIENTO_MODO_MANUAL, 500U, 1U, NULL};
    Driver_Motor_Estado resultado = DRIVER_MOTOR_ACEPTADA;
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &seleccion));
    CHECK(EmitirSeleccion(&motor, &seleccion, &resultado) && resultado == DRIVER_MOTOR_ERROR_SALIDA);
    CHECK(seleccion.fuente == PROCESAMIENTO_FUENTE_MANUAL && emulada.llamadas == 1U && emulada.ultimo_solicitado == 500U);
    emulada.acepta = true;
    CHECK(EmitirSeleccion(&motor, &seleccion, &resultado) && resultado == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 2U && emulada.ultimo_solicitado == 500U);
    emulada.acepta = false;
    entradas = (Procesamiento_Entradas){PROCESAMIENTO_MODO_AUTOMATICO, 500U, 2U, NULL};
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &seleccion));
    CHECK(EmitirSeleccion(&motor, &seleccion, &resultado) && resultado == DRIVER_MOTOR_ERROR_SALIDA);
    CHECK(emulada.llamadas == 3U && emulada.ultimo_solicitado == 0U);
    emulada.acepta = true;
    CHECK(EmitirSeleccion(&motor, &seleccion, &resultado) && resultado == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 4U && emulada.ultimo_solicitado == 0U);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S6 manual B04/B01", ManualDesdeEscalas}, {"S6 entrada auto/espera", EntradaAutoSolicitaCero},
        {"S7 RPM no es PWM", AutomaticoNoUsaRPMComoPWM}, {"S7 mando invalido", MandoNoValidoNoSeConsume},
        {"S4/S6 fallo de salida", FalloDelProveedorObservable}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
