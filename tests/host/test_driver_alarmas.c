#include "Driver_Alarmas.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

typedef struct { unsigned orden[8]; size_t cantidad; } Registro;
typedef struct {
    unsigned llamadas;
    bool ultimo_solicitado;
    bool acepta;
    unsigned id;
    Registro *registro;
} SalidaEmulada;

static bool Escribir(void *contexto, bool activar)
{
    SalidaEmulada *salida = contexto;
    if (salida == NULL) {
        return false;
    }
    ++salida->llamadas;
    salida->ultimo_solicitado = activar;
    if (salida->registro != NULL) {
        if (salida->registro->cantidad >= 8U) {
            return false;
        }
        salida->registro->orden[salida->registro->cantidad] = salida->id;
        ++salida->registro->cantidad;
    }
    return salida->acepta;
}

static Driver_Alarmas_Salidas Crear(SalidaEmulada *led, SalidaEmulada *buzzer)
{
    const Driver_Alarmas_Salidas salidas = {Escribir, led, Escribir, buzzer};
    return salidas;
}

static bool ActivarAmbas(void)
{
    SalidaEmulada led = {0U, false, true, 1U, NULL}, buzzer = {0U, false, true, 2U, NULL};
    const Driver_Alarmas_Salidas salidas = Crear(&led, &buzzer);
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(led.llamadas == 1U && buzzer.llamadas == 1U);
    CHECK(led.ultimo_solicitado && buzzer.ultimo_solicitado);
    return true;
}

static bool DesactivarAmbas(void)
{
    SalidaEmulada led = {0U, true, true, 1U, NULL}, buzzer = {0U, true, true, 2U, NULL};
    const Driver_Alarmas_Salidas salidas = Crear(&led, &buzzer);
    CHECK(Driver_Alarmas_Aplicar(&salidas, false) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(led.llamadas == 1U && buzzer.llamadas == 1U);
    CHECK(!led.ultimo_solicitado && !buzzer.ultimo_solicitado);
    return true;
}

static bool RepeticionSinCache(void)
{
    SalidaEmulada led = {0U, false, true, 1U, NULL}, buzzer = {0U, false, true, 2U, NULL};
    const Driver_Alarmas_Salidas salidas = Crear(&led, &buzzer);
    const bool pedidos[] = {true, true, false, false};
    for (size_t i = 0U; i < sizeof(pedidos) / sizeof(pedidos[0]); ++i) {
        CHECK(Driver_Alarmas_Aplicar(&salidas, pedidos[i]) == DRIVER_ALARMAS_ACEPTADA);
        CHECK(led.llamadas == i + 1U && buzzer.llamadas == i + 1U);
        CHECK(led.ultimo_solicitado == pedidos[i] && buzzer.ultimo_solicitado == pedidos[i]);
    }
    return true;
}

static bool OrdenDeCanales(void)
{
    Registro registro = {{0U}, 0U};
    SalidaEmulada led = {0U, false, true, 1U, &registro}, buzzer = {0U, false, true, 2U, &registro};
    const Driver_Alarmas_Salidas salidas = Crear(&led, &buzzer);
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(registro.cantidad == 2U && registro.orden[0] == 1U && registro.orden[1] == 2U);
    return true;
}

static bool InterfazInvalidaSinLlamadas(void)
{
    SalidaEmulada led = {0U, false, true, 1U, NULL}, buzzer = {0U, false, true, 2U, NULL};
    Driver_Alarmas_Salidas salidas = Crear(&led, &buzzer);
    CHECK(Driver_Alarmas_Aplicar(NULL, true) == DRIVER_ALARMAS_ERROR_ARGUMENTO);
    salidas.escribir_buzzer = NULL;
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ERROR_ARGUMENTO);
    salidas = Crear(&led, &buzzer);
    salidas.escribir_led = NULL;
    CHECK(Driver_Alarmas_Aplicar(&salidas, false) == DRIVER_ALARMAS_ERROR_ARGUMENTO);
    salidas.escribir_buzzer = NULL;
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ERROR_ARGUMENTO);
    CHECK(led.llamadas == 0U && buzzer.llamadas == 0U);
    CHECK(!led.ultimo_solicitado && !buzzer.ultimo_solicitado);
    return true;
}

static bool SinContexto(void *contexto, bool activar)
{
    (void)activar;
    return contexto == NULL;
}

static bool ContextosNulosPermitidos(void)
{
    const Driver_Alarmas_Salidas salidas = {SinContexto, NULL, SinContexto, NULL};
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(Driver_Alarmas_Aplicar(&salidas, false) == DRIVER_ALARMAS_ACEPTADA);
    return true;
}

static bool FalloLED(void)
{
    Registro registro = {{0U}, 0U};
    SalidaEmulada led = {0U, false, false, 1U, &registro}, buzzer = {0U, false, true, 2U, &registro};
    const Driver_Alarmas_Salidas salidas = Crear(&led, &buzzer);
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ERROR_LED);
    CHECK(led.llamadas == 1U && buzzer.llamadas == 1U);
    CHECK(registro.cantidad == 2U && registro.orden[0] == 1U && registro.orden[1] == 2U);
    CHECK(led.ultimo_solicitado && buzzer.ultimo_solicitado);
    return true;
}

static bool FalloBuzzer(void)
{
    SalidaEmulada led = {0U, true, true, 1U, NULL}, buzzer = {0U, true, false, 2U, NULL};
    const Driver_Alarmas_Salidas salidas = Crear(&led, &buzzer);
    CHECK(Driver_Alarmas_Aplicar(&salidas, false) == DRIVER_ALARMAS_ERROR_BUZZER);
    CHECK(led.llamadas == 1U && buzzer.llamadas == 1U);
    CHECK(!led.ultimo_solicitado && !buzzer.ultimo_solicitado);
    return true;
}

static bool FallanAmbos(void)
{
    SalidaEmulada led = {0U, false, false, 1U, NULL}, buzzer = {0U, false, false, 2U, NULL};
    const Driver_Alarmas_Salidas salidas = Crear(&led, &buzzer);
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ERROR_AMBOS);
    CHECK(led.llamadas == 1U && buzzer.llamadas == 1U);
    return true;
}

static bool RecuperacionSinEstadoRetenido(void)
{
    SalidaEmulada led = {0U, false, false, 1U, NULL}, buzzer = {0U, false, false, 2U, NULL};
    const Driver_Alarmas_Salidas salidas = Crear(&led, &buzzer);
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ERROR_AMBOS);
    led.acepta = true;
    buzzer.acepta = true;
    CHECK(Driver_Alarmas_Aplicar(&salidas, false) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(led.llamadas == 2U && buzzer.llamadas == 2U);
    CHECK(!led.ultimo_solicitado && !buzzer.ultimo_solicitado);
    buzzer.acepta = false;
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ERROR_BUZZER);
    CHECK(led.llamadas == 3U && buzzer.llamadas == 3U);
    return true;
}

static bool InstanciasIndependientes(void)
{
    SalidaEmulada la = {0U, false, false, 1U, NULL}, ba = {0U, false, true, 2U, NULL};
    SalidaEmulada lb = {0U, true, true, 1U, NULL}, bb = {0U, true, true, 2U, NULL};
    const Driver_Alarmas_Salidas a = Crear(&la, &ba), b = Crear(&lb, &bb);
    CHECK(Driver_Alarmas_Aplicar(&a, true) == DRIVER_ALARMAS_ERROR_LED);
    CHECK(lb.llamadas == 0U && bb.llamadas == 0U);
    CHECK(Driver_Alarmas_Aplicar(&b, false) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(la.llamadas == 1U && ba.llamadas == 1U && la.ultimo_solicitado && ba.ultimo_solicitado);
    CHECK(lb.llamadas == 1U && bb.llamadas == 1U && !lb.ultimo_solicitado && !bb.ultimo_solicitado);
    return true;
}

static bool InterfazInmutable(void)
{
    SalidaEmulada led = {0U, false, true, 1U, NULL}, buzzer = {0U, false, true, 2U, NULL};
    const Driver_Alarmas_Salidas salidas = Crear(&led, &buzzer);
    const Driver_Alarmas_Salidas copia = salidas;
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(salidas.escribir_led == copia.escribir_led && salidas.contexto_led == copia.contexto_led);
    CHECK(salidas.escribir_buzzer == copia.escribir_buzzer && salidas.contexto_buzzer == copia.contexto_buzzer);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S1 activar", ActivarAmbas}, {"S1 desactivar", DesactivarAmbas},
        {"S1 repeticion", RepeticionSinCache}, {"S2 orden", OrdenDeCanales},
        {"S3 interfaz invalida", InterfazInvalidaSinLlamadas}, {"S5 contextos NULL", ContextosNulosPermitidos},
        {"S4 fallo LED", FalloLED}, {"S4 fallo buzzer", FalloBuzzer}, {"S4 ambos fallan", FallanAmbos},
        {"S4 recuperacion", RecuperacionSinEstadoRetenido}, {"S2 instancias", InstanciasIndependientes},
        {"S5 interfaz const", InterfazInmutable}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
