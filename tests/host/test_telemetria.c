#include "Comunicacion.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static Comunicacion_Telemetria Ejemplo(void)
{
    const Comunicacion_Telemetria datos = {123456U, 2534, COMUNICACION_DATO_VALIDO,
        COMUNICACION_DATO_VALIDO, COMUNICACION_OPERACION_MANUAL};
    return datos;
}

static bool Esperar(const Comunicacion_Telemetria *datos, const char *texto)
{
    char buffer[COMUNICACION_TELEMETRIA_CAPACIDAD];
    memset(buffer, '#', sizeof(buffer));
    size_t longitud = SIZE_MAX;
    CHECK(Comunicacion_CodificarTelemetria(datos, buffer, sizeof(buffer), &longitud) == COMUNICACION_TELEMETRIA_OK);
    CHECK(strcmp(buffer, texto) == 0 && longitud == strlen(texto));
    CHECK(buffer[longitud] == '\0' && longitud > 0U && buffer[longitud - 1U] == '\n');
    return true;
}

static bool EjemploYEstados(void)
{
    Comunicacion_Telemetria datos = Ejemplo();
    CHECK(Esperar(&datos, "RPM=123.456;TEMP=25.34;ESTADO=MANUAL\n"));
    datos.estado_operacion = COMUNICACION_OPERACION_AUTO_ESPERA;
    CHECK(Esperar(&datos, "RPM=123.456;TEMP=25.34;ESTADO=AUTO_ESPERA\n"));
    datos.estado_operacion = COMUNICACION_OPERACION_AUTO_ACTIVO;
    CHECK(Esperar(&datos, "RPM=123.456;TEMP=25.34;ESTADO=AUTO_ACTIVO\n"));
    return true;
}

static bool CerosYFracciones(void)
{
    Comunicacion_Telemetria datos = Ejemplo();
    const struct { uint32_t rpm; int32_t temperatura; const char *texto; } casos[] = {
        {0U, 0, "RPM=0.000;TEMP=0.00;ESTADO=MANUAL\n"},
        {1U, 1, "RPM=0.001;TEMP=0.01;ESTADO=MANUAL\n"},
        {10U, -1, "RPM=0.010;TEMP=-0.01;ESTADO=MANUAL\n"},
        {100U, -10, "RPM=0.100;TEMP=-0.10;ESTADO=MANUAL\n"},
        {999U, -99, "RPM=0.999;TEMP=-0.99;ESTADO=MANUAL\n"},
        {1000U, -100, "RPM=1.000;TEMP=-1.00;ESTADO=MANUAL\n"},
        {12005U, -2505, "RPM=12.005;TEMP=-25.05;ESTADO=MANUAL\n"}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        datos.rpm_milesimas = casos[i].rpm;
        datos.temperatura_centesimas_c = casos[i].temperatura;
        CHECK(Esperar(&datos, casos[i].texto));
    }
    return true;
}

static bool FronterasNumericas(void)
{
    Comunicacion_Telemetria datos = Ejemplo();
    datos.rpm_milesimas = UINT32_MAX;
    datos.temperatura_centesimas_c = INT32_MIN;
    datos.estado_operacion = COMUNICACION_OPERACION_AUTO_ACTIVO;
    const char maxima[] = "RPM=4294967.295;TEMP=-21474836.48;ESTADO=AUTO_ACTIVO\n";
    CHECK(sizeof(maxima) == 54U && COMUNICACION_TELEMETRIA_CAPACIDAD == sizeof(maxima));
    CHECK(Esperar(&datos, maxima));
    datos.temperatura_centesimas_c = INT32_MAX;
    CHECK(Esperar(&datos, "RPM=4294967.295;TEMP=21474836.47;ESTADO=AUTO_ACTIVO\n"));
    datos.rpm_milesimas = 186001U;
    CHECK(Esperar(&datos, "RPM=186.001;TEMP=21474836.47;ESTADO=AUTO_ACTIVO\n"));
    return true;
}

static bool Disponibilidad(void)
{
    const Comunicacion_ValidezDato estados[] = {COMUNICACION_DATO_VALIDO, COMUNICACION_DATO_SIN_DATOS, COMUNICACION_DATO_ERROR};
    const char *rpm[] = {"4294967.295", "ND", "ERR"};
    const char *temperatura[] = {"-21474836.48", "ND", "ERR"};
    Comunicacion_Telemetria datos = Ejemplo();
    datos.rpm_milesimas = UINT32_MAX;datos.temperatura_centesimas_c = INT32_MIN;
    for (size_t r = 0U; r < 3U; ++r) {
        for (size_t t = 0U; t < 3U; ++t) {
            char esperado[80];
            const int usados = snprintf(esperado, sizeof(esperado), "RPM=%s;TEMP=%s;ESTADO=MANUAL\n", rpm[r], temperatura[t]);
            CHECK(usados > 0 && (size_t)usados < sizeof(esperado));
            datos.validez_rpm = estados[r];datos.validez_temperatura = estados[t];
            CHECK(Esperar(&datos, esperado));
        }
    }
    return true;
}

static bool ArgumentosNulosYPrioridad(void)
{
    const Comunicacion_Telemetria datos = Ejemplo();
    for (unsigned mascara = 1U; mascara < 8U; ++mascara) {
        char buffer[54];memset(buffer, '#', sizeof(buffer));size_t longitud = 123U;
        CHECK(Comunicacion_CodificarTelemetria((mascara & 1U) ? NULL : &datos,
            (mascara & 2U) ? NULL : buffer, sizeof(buffer), (mascara & 4U) ? NULL : &longitud)
            == COMUNICACION_TELEMETRIA_ERROR_ARGUMENTO);
        CHECK(longitud == ((mascara & 4U) ? 123U : 0U));
        CHECK(buffer[0] == ((mascara & 2U) ? '#' : '\0'));
        for (size_t i = 1U; i < sizeof(buffer); ++i) { CHECK(buffer[i] == '#'); }
    }
    Comunicacion_Telemetria invalida = datos;
    invalida.estado_operacion = (Comunicacion_EstadoOperacion)99;
    size_t longitud = 9U;
    CHECK(Comunicacion_CodificarTelemetria(&invalida, NULL, 0U, &longitud) == COMUNICACION_TELEMETRIA_ERROR_ARGUMENTO);
    CHECK(longitud == 0U);
    return true;
}

static bool EstadosInvalidos(void)
{
    for (unsigned campo = 0U; campo < 3U; ++campo) {
        Comunicacion_Telemetria datos = Ejemplo();
        if (campo == 0U) { datos.validez_rpm = (Comunicacion_ValidezDato)99; }
        if (campo == 1U) { datos.validez_temperatura = (Comunicacion_ValidezDato)99; }
        if (campo == 2U) { datos.estado_operacion = (Comunicacion_EstadoOperacion)99; }
        char buffer[54];memset(buffer, '#', sizeof(buffer));size_t longitud = 9U;
        CHECK(Comunicacion_CodificarTelemetria(&datos, buffer, sizeof(buffer), &longitud) == COMUNICACION_TELEMETRIA_ERROR_ESTADO);
        CHECK(longitud == 0U && buffer[0] == '\0' && buffer[1] == '#');
        buffer[0] = '#';longitud = 9U;
        CHECK(Comunicacion_CodificarTelemetria(&datos, buffer, 0U, &longitud) == COMUNICACION_TELEMETRIA_ERROR_ESTADO);
        CHECK(longitud == 0U && buffer[0] == '#');
    }
    return true;
}

static bool CapacidadesYCanarios(void)
{
    const Comunicacion_Telemetria datos = {UINT32_MAX, INT32_MIN, COMUNICACION_DATO_VALIDO,
        COMUNICACION_DATO_VALIDO, COMUNICACION_OPERACION_AUTO_ACTIVO};
    const char maxima[] = "RPM=4294967.295;TEMP=-21474836.48;ESTADO=AUTO_ACTIVO\n";
    for (size_t capacidad = 0U; capacidad <= 54U; ++capacidad) {
        unsigned char memoria[56];memset(memoria, 0xA5, sizeof(memoria));
        char *buffer = (char *)&memoria[1];size_t longitud = SIZE_MAX;
        const Comunicacion_ResultadoTelemetria resultado = Comunicacion_CodificarTelemetria(&datos, buffer, capacidad, &longitud);
        CHECK(memoria[0] == 0xA5 && memoria[55] == 0xA5);
        if (capacidad < 54U) {
            CHECK(resultado == COMUNICACION_TELEMETRIA_ERROR_CAPACIDAD && longitud == 0U);
            CHECK(memoria[1] == (capacidad == 0U ? 0xA5 : 0U));
            for (size_t i = 2U; i < sizeof(memoria); ++i) { CHECK(memoria[i] == 0xA5); }
        } else {
            CHECK(resultado == COMUNICACION_TELEMETRIA_OK && longitud == 53U);
            CHECK(strcmp(buffer, maxima) == 0 && buffer[53] == '\0');
        }
    }
    return true;
}

static bool CapacidadExactaTramaCorta(void)
{
    Comunicacion_Telemetria datos = Ejemplo();
    datos.validez_rpm = COMUNICACION_DATO_SIN_DATOS;datos.validez_temperatura = COMUNICACION_DATO_ERROR;
    const char esperado[] = "RPM=ND;TEMP=ERR;ESTADO=MANUAL\n";
    char buffer[sizeof(esperado)];size_t longitud = 9U;
    CHECK(Comunicacion_CodificarTelemetria(&datos, buffer, sizeof(buffer) - 1U, &longitud) == COMUNICACION_TELEMETRIA_ERROR_CAPACIDAD);
    CHECK(longitud == 0U && buffer[0] == '\0');
    CHECK(Comunicacion_CodificarTelemetria(&datos, buffer, sizeof(buffer), &longitud) == COMUNICACION_TELEMETRIA_OK);
    CHECK(strcmp(buffer, esperado) == 0 && longitud == sizeof(esperado) - 1U);
    return true;
}

static bool RecuperacionYRepeticion(void)
{
    Comunicacion_Telemetria datos = Ejemplo();char buffer[54];size_t longitud = 0U;
    CHECK(Comunicacion_CodificarTelemetria(&datos, buffer, 1U, &longitud) == COMUNICACION_TELEMETRIA_ERROR_CAPACIDAD);
    CHECK(longitud == 0U && buffer[0] == '\0');
    CHECK(Comunicacion_CodificarTelemetria(&datos, buffer, sizeof(buffer), &longitud) == COMUNICACION_TELEMETRIA_OK);
    datos.validez_rpm = (Comunicacion_ValidezDato)99;
    CHECK(Comunicacion_CodificarTelemetria(&datos, buffer, sizeof(buffer), &longitud) == COMUNICACION_TELEMETRIA_ERROR_ESTADO);
    CHECK(longitud == 0U && buffer[0] == '\0');datos = Ejemplo();
    CHECK(Esperar(&datos, "RPM=123.456;TEMP=25.34;ESTADO=MANUAL\n"));
    CHECK(Esperar(&datos, "RPM=123.456;TEMP=25.34;ESTADO=MANUAL\n"));
    return true;
}

static bool IndependenciaEInmutabilidad(void)
{
    const Comunicacion_Telemetria datos = Ejemplo();const Comunicacion_Telemetria copia = datos;
    char a[54], b[54];size_t na = 0U, nb = 0U;
    CHECK(Comunicacion_CodificarTelemetria(&datos, a, sizeof(a), &na) == COMUNICACION_TELEMETRIA_OK);
    Comunicacion_Telemetria otros = datos;otros.rpm_milesimas = 0U;otros.temperatura_centesimas_c = -1;
    CHECK(Comunicacion_CodificarTelemetria(&otros, b, sizeof(b), &nb) == COMUNICACION_TELEMETRIA_OK);
    CHECK(strcmp(a, "RPM=123.456;TEMP=25.34;ESTADO=MANUAL\n") == 0 && na == strlen(a));
    CHECK(strcmp(b, "RPM=0.000;TEMP=-0.01;ESTADO=MANUAL\n") == 0 && nb == strlen(b));
    CHECK(datos.rpm_milesimas == copia.rpm_milesimas && datos.temperatura_centesimas_c == copia.temperatura_centesimas_c);
    CHECK(datos.validez_rpm == copia.validez_rpm && datos.validez_temperatura == copia.validez_temperatura && datos.estado_operacion == copia.estado_operacion);
    return true;
}

/* Oraculo host: printf de libc/enteros amplios, no el algoritmo manual de digitos. */
static bool CompararConLibc(uint32_t rpm, int32_t temperatura)
{
    Comunicacion_Telemetria datos = Ejemplo();datos.rpm_milesimas = rpm;datos.temperatura_centesimas_c = temperatura;
    const uint64_t magnitud = temperatura < 0 ? (uint64_t)(-(int64_t)temperatura) : (uint64_t)temperatura;
    char esperado[80];
    const int usados = snprintf(esperado, sizeof(esperado), "RPM=%" PRIu32 ".%03" PRIu32 ";TEMP=%s%" PRIu64 ".%02" PRIu64 ";ESTADO=MANUAL\n",
        rpm / 1000U, rpm % 1000U, temperatura < 0 ? "-" : "", magnitud / 100U, magnitud % 100U);
    CHECK(usados > 0 && (size_t)usados < sizeof(esperado));CHECK(Esperar(&datos, esperado));return true;
}

static bool OraculoNumerico(void)
{
    for (uint32_t f = 0U; f < 1000U; ++f) {
        const int32_t t = (int32_t)(f % 100U);
        CHECK(CompararConLibc(123000U + f, f % 2U == 0U ? t : -t));
    }
    uint32_t semilla = 0xB09U;
    for (unsigned i = 0U; i < 4096U; ++i) {
        semilla = semilla * 1664525U + 1013904223U;const uint32_t rpm = semilla;
        semilla = semilla * 1664525U + 1013904223U;
        const int32_t magnitud = (int32_t)(semilla & UINT32_C(0x7FFFFFFF));
        CHECK(CompararConLibc(rpm, semilla & UINT32_C(0x80000000) ? -magnitud : magnitud));
    }
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S1 ejemplo/estados", EjemploYEstados}, {"S1/S2 ceros/fracciones", CerosYFracciones},
        {"S2 extremos", FronterasNumericas}, {"S3 disponibilidad", Disponibilidad},
        {"S4 NULL/prioridad", ArgumentosNulosYPrioridad}, {"S4 estados", EstadosInvalidos},
        {"S5 55 capacidades/canarios", CapacidadesYCanarios}, {"S5 capacidad exacta corta", CapacidadExactaTramaCorta},
        {"S6 recuperacion/repeticion", RecuperacionYRepeticion}, {"S6 independencia/const", IndependenciaEInmutabilidad},
        {"S1/S2 5096 pares contra libc", OraculoNumerico}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));return 0;
}
