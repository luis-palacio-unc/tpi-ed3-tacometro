#include "Mandos.h"
#include "Procesamiento.h"
#include "Driver_Motor.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)
#define MAN MANDOS_SELECTOR_MODO_MANUAL
#define AUT MANDOS_SELECTOR_MODO_AUTOMATICO
#define VAL MANDOS_SELECTOR_FUENTE_VALIDA

typedef struct { unsigned llamadas; uint16_t ultimo; } MotorEmulado;
static bool Escribir(void *contexto, uint16_t mando)
{
    MotorEmulado *motor = contexto;++motor->llamadas;motor->ultimo = mando;return true;
}

/* Adaptacion de prueba, no agregar dependencia de Mandos hacia Aplicacion. */
static bool ActualizarModo(const Mandos_Selector *selector, Procesamiento_Contexto *app,
    uint16_t manual, uint64_t orden, const Procesamiento_Consigna *consigna, Procesamiento_Salida *salida)
{
    if (selector->estado != MANDOS_SELECTOR_VALIDO) { return false; }
    const Procesamiento_Modo modo = selector->modo == MAN ? PROCESAMIENTO_MODO_MANUAL : PROCESAMIENTO_MODO_AUTOMATICO;
    const Procesamiento_Entradas entradas = {modo, manual, orden, consigna};
    return Procesamiento_Actualizar(app, &entradas, salida);
}

static bool Emitir(const Driver_Motor_Salida *motor, const Procesamiento_Salida *seleccion, Driver_Motor_Estado *resultado)
{
    if (seleccion->fuente == PROCESAMIENTO_FUENTE_CERO) { *resultado = Driver_Motor_SolicitarCero(motor);return true; }
    if (seleccion->fuente == PROCESAMIENTO_FUENTE_MANUAL) { *resultado = Driver_Motor_Aplicar(motor, seleccion->mando_manual_por_mil);return true; }
    return false;
}

static bool TransicionConfirmada(void)
{
    Mandos_Selector selector;CHECK(Mandos_InicializarSelector(&selector, 10U));
    CHECK(Mandos_ActualizarSelector(&selector, MAN, VAL, 0U) == MANDOS_SELECTOR_SIN_DATOS);
    CHECK(Mandos_ActualizarSelector(&selector, MAN, VAL, 10U) == MANDOS_SELECTOR_VALIDO);
    Mandos_MandoManual manual;CHECK(Mandos_NormalizarManual(&manual, 1U, 2U) == MANDOS_VALIDO);
    Procesamiento_Contexto app;CHECK(Procesamiento_Inicializar(&app, PROCESAMIENTO_MODO_MANUAL, 0U));
    Procesamiento_Salida salida;MotorEmulado emulado = {0U, 0U};const Driver_Motor_Salida motor = {Escribir, &emulado};
    Driver_Motor_Estado resultado = DRIVER_MOTOR_ERROR_ARGUMENTO;
    CHECK(ActualizarModo(&selector, &app, manual.por_mil, 1U, NULL, &salida));CHECK(Emitir(&motor, &salida, &resultado));
    CHECK(resultado == DRIVER_MOTOR_ACEPTADA && emulado.ultimo == 500U);
    const struct { uint32_t tiempo; Mandos_ModoSelector modo; } rebotes[] = {{11U, AUT}, {14U, MAN}, {20U, AUT}, {29U, AUT}};
    for (size_t i = 0U; i < sizeof(rebotes) / sizeof(rebotes[0]); ++i) {
        CHECK(Mandos_ActualizarSelector(&selector, rebotes[i].modo, VAL, rebotes[i].tiempo) == MANDOS_SELECTOR_VALIDO);
        CHECK(ActualizarModo(&selector, &app, manual.por_mil, i + 2U, NULL, &salida));
        CHECK(salida.fuente == PROCESAMIENTO_FUENTE_MANUAL);CHECK(Emitir(&motor, &salida, &resultado));
        CHECK(emulado.ultimo == 500U);
    }
    CHECK(Mandos_ActualizarSelector(&selector, AUT, VAL, 30U) == MANDOS_SELECTOR_VALIDO);
    const Procesamiento_Consigna antigua = {185U, 5U, true};
    CHECK(ActualizarModo(&selector, &app, manual.por_mil, 6U, &antigua, &salida));
    CHECK(salida.fuente == PROCESAMIENTO_FUENTE_CERO);CHECK(Emitir(&motor, &salida, &resultado));
    CHECK(emulado.ultimo == 0U && emulado.llamadas == 6U);
    CHECK(Mandos_ActualizarSelector(&selector, AUT, VAL, 31U) == MANDOS_SELECTOR_VALIDO);
    const Procesamiento_Consigna nueva = {185U, 7U, true};
    CHECK(ActualizarModo(&selector, &app, manual.por_mil, 7U, &nueva, &salida));
    CHECK(salida.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA && salida.consigna_rpm == 185U);
    CHECK(!Emitir(&motor, &salida, &resultado) && emulado.llamadas == 6U);return true;
}

static bool ReboteAutoNoInvalidaConsigna(void)
{
    Mandos_Selector selector;CHECK(Mandos_InicializarSelector(&selector, 10U));
    CHECK(Mandos_ActualizarSelector(&selector, AUT, VAL, 0U) == MANDOS_SELECTOR_SIN_DATOS);
    CHECK(Mandos_ActualizarSelector(&selector, AUT, VAL, 10U) == MANDOS_SELECTOR_VALIDO);
    Procesamiento_Contexto app;CHECK(Procesamiento_Inicializar(&app, PROCESAMIENTO_MODO_AUTOMATICO, 0U));
    Procesamiento_Salida salida;CHECK(ActualizarModo(&selector, &app, 500U, 1U, NULL, &salida));
    const Procesamiento_Consigna nueva = {100U, 2U, true};CHECK(ActualizarModo(&selector, &app, 500U, 2U, &nueva, &salida));
    CHECK(salida.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA);
    const uint32_t tiempos[] = {11U, 19U, 20U};const Mandos_ModoSelector modos[] = {MAN, MAN, AUT};
    for (size_t i = 0U; i < 3U; ++i) {
        CHECK(Mandos_ActualizarSelector(&selector, modos[i], VAL, tiempos[i]) == MANDOS_SELECTOR_VALIDO);
        CHECK(ActualizarModo(&selector, &app, 500U, i + 3U, NULL, &salida));
        CHECK(salida.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA && salida.consigna_rpm == 100U);
    }
    return true;
}

static bool IndisponibilidadNoEmiteOrden(void)
{
    Mandos_Selector selector;CHECK(Mandos_InicializarSelector(&selector, 5U));
    Procesamiento_Contexto app;CHECK(Procesamiento_Inicializar(&app, PROCESAMIENTO_MODO_MANUAL, 0U));
    Procesamiento_Salida salida;MotorEmulado emulado = {0U, 777U};const Driver_Motor_Salida motor = {Escribir, &emulado};
    Driver_Motor_Estado resultado = DRIVER_MOTOR_ERROR_SALIDA;
    const Mandos_FuenteSelector fuentes[] = {MANDOS_SELECTOR_FUENTE_SIN_DATOS, MANDOS_SELECTOR_FUENTE_ERROR, (Mandos_FuenteSelector)99};
    CHECK(!ActualizarModo(&selector, &app, 500U, 1U, NULL, &salida));
    for (size_t i = 0U; i < 3U; ++i) {
        (void)Mandos_ActualizarSelector(&selector, MAN, fuentes[i], (uint32_t)i);
        if (ActualizarModo(&selector, &app, 500U, i + 2U, NULL, &salida)) { CHECK(Emitir(&motor, &salida, &resultado)); }
        CHECK(emulado.llamadas == 0U && emulado.ultimo == 777U && resultado == DRIVER_MOTOR_ERROR_SALIDA);
    }
    CHECK(Mandos_ActualizarSelector(&selector, MAN, VAL, 10U) == MANDOS_SELECTOR_SIN_DATOS);
    CHECK(!ActualizarModo(&selector, &app, 500U, 5U, NULL, &salida));
    CHECK(Mandos_ActualizarSelector(&selector, MAN, VAL, 15U) == MANDOS_SELECTOR_VALIDO);
    CHECK(ActualizarModo(&selector, &app, 500U, 6U, NULL, &salida));CHECK(Emitir(&motor, &salida, &resultado));
    CHECK(resultado == DRIVER_MOTOR_ACEPTADA && emulado.llamadas == 1U && emulado.ultimo == 500U);return true;
}

static bool RolloverNoEsOrdenB01(void)
{
    Mandos_Selector selector;CHECK(Mandos_InicializarSelector(&selector, 5U));
    CHECK(Mandos_ActualizarSelector(&selector, MAN, VAL, UINT32_MAX - 2U) == MANDOS_SELECTOR_SIN_DATOS);
    CHECK(Mandos_ActualizarSelector(&selector, MAN, VAL, 2U) == MANDOS_SELECTOR_VALIDO);
    Procesamiento_Contexto app;CHECK(Procesamiento_Inicializar(&app, PROCESAMIENTO_MODO_MANUAL, 1000U));
    Procesamiento_Salida salida;CHECK(ActualizarModo(&selector, &app, 500U, 1001U, NULL, &salida));
    CHECK(Mandos_ActualizarSelector(&selector, AUT, VAL, 3U) == MANDOS_SELECTOR_VALIDO);
    CHECK(ActualizarModo(&selector, &app, 500U, 1002U, NULL, &salida));CHECK(salida.fuente == PROCESAMIENTO_FUENTE_MANUAL);
    CHECK(Mandos_ActualizarSelector(&selector, AUT, VAL, 8U) == MANDOS_SELECTOR_VALIDO);
    CHECK(ActualizarModo(&selector, &app, 500U, 1003U, NULL, &salida));CHECK(salida.fuente == PROCESAMIENTO_FUENTE_CERO);
    MotorEmulado emulado = {0U, 500U};const Driver_Motor_Salida motor = {Escribir, &emulado};Driver_Motor_Estado resultado;
    CHECK(Emitir(&motor, &salida, &resultado) && resultado == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulado.llamadas == 1U && emulado.ultimo == 0U);return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S6 transicion confirmada/cero/nueva consigna", TransicionConfirmada},
        {"S6 rebote auto activo", ReboteAutoNoInvalidaConsigna},
        {"S6 indisponibilidad", IndisponibilidadNoEmiteOrden}, {"S6 tiempo/orden independientes", RolloverNoEsOrdenB01}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre);return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));return 0;
}
