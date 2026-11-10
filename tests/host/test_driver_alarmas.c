#include "Driver_Alarmas.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

typedef struct { unsigned llamadas; bool ultimo; } SalidaEmulada;
typedef struct { unsigned canal; bool activar; } Evento;
static SalidaEmulada salidas_emuladas[4];
static Evento eventos[16];
static size_t cantidad;
static bool desborde;

static void Preparar(void)
{
    memset(salidas_emuladas, 0, sizeof(salidas_emuladas));
    memset(eventos, 0, sizeof(eventos));
    cantidad = 0U;
    desborde = false;
}

static void Registrar(unsigned canal, bool activar)
{
    if (canal >= 4U || cantidad >= sizeof(eventos) / sizeof(eventos[0])) {
        desborde = true;
        return;
    }
    ++salidas_emuladas[canal].llamadas;
    salidas_emuladas[canal].ultimo = activar;
    eventos[cantidad].canal = canal;
    eventos[cantidad].activar = activar;
    ++cantidad;
}

static void LedA(bool activar) { Registrar(0U, activar); }
static void BuzzerA(bool activar) { Registrar(1U, activar); }
static void LedB(bool activar) { Registrar(2U, activar); }
static void BuzzerB(bool activar) { Registrar(3U, activar); }

static bool ActivarAmbas(void)
{
    Preparar();
    const Driver_Alarmas_Salidas salidas = {LedA, BuzzerA};
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(cantidad == 2U && !desborde);
    CHECK(salidas_emuladas[0].llamadas == 1U && salidas_emuladas[1].llamadas == 1U);
    CHECK(salidas_emuladas[0].ultimo && salidas_emuladas[1].ultimo);
    return true;
}

static bool DesactivarAmbas(void)
{
    Preparar();
    salidas_emuladas[0].ultimo = true;
    salidas_emuladas[1].ultimo = true;
    const Driver_Alarmas_Salidas salidas = {LedA, BuzzerA};
    CHECK(Driver_Alarmas_Aplicar(&salidas, false) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(cantidad == 2U && !desborde);
    CHECK(salidas_emuladas[0].llamadas == 1U && salidas_emuladas[1].llamadas == 1U);
    CHECK(!salidas_emuladas[0].ultimo && !salidas_emuladas[1].ultimo);
    return true;
}

static bool RepeticionSinCache(void)
{
    Preparar();
    const Driver_Alarmas_Salidas salidas = {LedA, BuzzerA};
    const bool pedidos[] = {true, true, false, false};
    for (size_t i = 0U; i < sizeof(pedidos) / sizeof(pedidos[0]); ++i) {
        CHECK(Driver_Alarmas_Aplicar(&salidas, pedidos[i]) == DRIVER_ALARMAS_ACEPTADA);
        CHECK(cantidad == 2U * (i + 1U) && !desborde);
        CHECK(eventos[2U * i].canal == 0U && eventos[2U * i + 1U].canal == 1U);
        CHECK(eventos[2U * i].activar == pedidos[i] && eventos[2U * i + 1U].activar == pedidos[i]);
        CHECK(salidas_emuladas[0].llamadas == i + 1U && salidas_emuladas[1].llamadas == i + 1U);
    }
    return true;
}

static bool OrdenDeCanales(void)
{
    Preparar();
    const Driver_Alarmas_Salidas salidas = {LedA, BuzzerA};
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(Driver_Alarmas_Aplicar(&salidas, false) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(cantidad == 4U && !desborde);
    CHECK(eventos[0].canal == 0U && eventos[1].canal == 1U);
    CHECK(eventos[2].canal == 0U && eventos[3].canal == 1U);
    CHECK(eventos[0].activar && eventos[1].activar && !eventos[2].activar && !eventos[3].activar);
    return true;
}

static bool InterfazInvalidaSinLlamadas(void)
{
    Preparar();
    const Driver_Alarmas_Salidas incompletas[] = {{NULL, BuzzerA}, {LedA, NULL}, {NULL, NULL}};
    for (unsigned valor = 0U; valor < 2U; ++valor) {
        CHECK(Driver_Alarmas_Aplicar(NULL, valor != 0U) == DRIVER_ALARMAS_ERROR_ARGUMENTO);
        for (size_t i = 0U; i < sizeof(incompletas) / sizeof(incompletas[0]); ++i) {
            CHECK(Driver_Alarmas_Aplicar(&incompletas[i], valor != 0U) == DRIVER_ALARMAS_ERROR_ARGUMENTO);
            CHECK(incompletas[i].escribir_led == (i == 1U ? LedA : NULL));
            CHECK(incompletas[i].escribir_buzzer == (i == 0U ? BuzzerA : NULL));
        }
    }
    CHECK(cantidad == 0U && !desborde);
    for (size_t i = 0U; i < 4U; ++i) { CHECK(salidas_emuladas[i].llamadas == 0U); }
    return true;
}

static bool RecuperacionTrasArgumentoInvalido(void)
{
    Preparar();
    const Driver_Alarmas_Salidas valida = {LedA, BuzzerA}, invalida = {LedA, NULL};
    CHECK(Driver_Alarmas_Aplicar(&invalida, true) == DRIVER_ALARMAS_ERROR_ARGUMENTO);
    CHECK(cantidad == 0U);
    CHECK(Driver_Alarmas_Aplicar(&valida, true) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(Driver_Alarmas_Aplicar(NULL, false) == DRIVER_ALARMAS_ERROR_ARGUMENTO);
    CHECK(cantidad == 2U);
    CHECK(Driver_Alarmas_Aplicar(&valida, false) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(cantidad == 4U && !desborde);
    CHECK(eventos[0].activar && eventos[1].activar && !eventos[2].activar && !eventos[3].activar);
    return true;
}

static bool InterfacesIndependientes(void)
{
    Preparar();
    const Driver_Alarmas_Salidas a = {LedA, BuzzerA}, b = {LedB, BuzzerB};
    CHECK(Driver_Alarmas_Aplicar(&a, true) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(salidas_emuladas[2].llamadas == 0U && salidas_emuladas[3].llamadas == 0U);
    CHECK(Driver_Alarmas_Aplicar(&b, false) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(salidas_emuladas[0].ultimo && salidas_emuladas[1].ultimo);
    CHECK(Driver_Alarmas_Aplicar(&a, false) == DRIVER_ALARMAS_ACEPTADA);
    const unsigned esperado[] = {0U, 1U, 2U, 3U, 0U, 1U};
    CHECK(cantidad == 6U && !desborde);
    for (size_t i = 0U; i < cantidad; ++i) {
        CHECK(eventos[i].canal == esperado[i] && eventos[i].activar == (i < 2U));
    }
    CHECK(salidas_emuladas[0].llamadas == 2U && salidas_emuladas[1].llamadas == 2U);
    CHECK(salidas_emuladas[2].llamadas == 1U && salidas_emuladas[3].llamadas == 1U);
    return true;
}

static bool InterfazInmutable(void)
{
    Preparar();
    const Driver_Alarmas_Salidas salidas = {LedA, BuzzerA};
    const Driver_Alarmas_Salidas copia = salidas;
    CHECK(Driver_Alarmas_Aplicar(&salidas, true) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(Driver_Alarmas_Aplicar(&salidas, false) == DRIVER_ALARMAS_ACEPTADA);
    CHECK(salidas.escribir_led == copia.escribir_led && salidas.escribir_buzzer == copia.escribir_buzzer);
    CHECK(cantidad == 4U && !desborde);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S1 activar", ActivarAmbas}, {"S1 desactivar", DesactivarAmbas},
        {"S1 repeticion", RepeticionSinCache}, {"S1 orden", OrdenDeCanales},
        {"S3 interfaz invalida", InterfazInvalidaSinLlamadas},
        {"S4 recuperacion argumento", RecuperacionTrasArgumentoInvalido},
        {"S2 interfaces", InterfacesIndependientes}, {"S5 interfaz const", InterfazInmutable}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
