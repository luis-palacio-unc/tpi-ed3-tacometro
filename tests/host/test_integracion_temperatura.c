#include "LM35.h"
#include "Temperatura.h"
#include <stdio.h>
#include <stddef.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static Temperatura_Estado Adaptar(Temperatura_Lectura *destino, const LM35_Lectura *origen)
{
    Temperatura_EstadoFuente fuente = TEMPERATURA_FUENTE_ERROR;
    int32_t valor = 0;
    if (origen->estado == LM35_SIN_DATOS) {
        fuente = TEMPERATURA_FUENTE_SIN_DATOS;
    } else if (origen->estado == LM35_VALIDA && origen->centesimas_c <= (uint32_t)INT32_MAX) {
        fuente = TEMPERATURA_FUENTE_VALIDA;
        valor = (int32_t)origen->centesimas_c;
    }
    return Temperatura_Actualizar(destino, valor, fuente);
}

static bool ConversionYPropagacion(void)
{
    LM35_Lectura sensor;
    Temperatura_Lectura servicio;
    CHECK(LM35_ConvertirMuestra(&sensor, 125U, 1500U, 3000000U) == LM35_VALIDA);
    CHECK(Adaptar(&servicio, &sensor) == TEMPERATURA_VALIDA);
    CHECK(servicio.centesimas_c == 2500);
    CHECK(LM35_ConvertirMuestra(&sensor, UINT32_MAX - 1U, UINT32_MAX, UINT32_MAX) == LM35_VALIDA);
    CHECK(Adaptar(&servicio, &sensor) == TEMPERATURA_VALIDA);
    CHECK(servicio.centesimas_c == 42949673);
    return true;
}

static bool ErrorYRecuperacion(void)
{
    LM35_Lectura sensor;
    Temperatura_Lectura servicio;
    CHECK(LM35_ConvertirMuestra(&sensor, 125U, 1500U, 3000000U) == LM35_VALIDA);
    CHECK(Adaptar(&servicio, &sensor) == TEMPERATURA_VALIDA);
    CHECK(LM35_ConvertirMuestra(&sensor, 1U, 0U, 0U) == LM35_ERROR_ESCALA);
    CHECK(Adaptar(&servicio, &sensor) == TEMPERATURA_ERROR_FUENTE);
    CHECK(servicio.centesimas_c == 0 && servicio.estado == TEMPERATURA_ERROR_FUENTE);
    CHECK(LM35_ConvertirMuestra(&sensor, 500U, 1500U, 3000000U) == LM35_VALIDA);
    CHECK(Adaptar(&servicio, &sensor) == TEMPERATURA_VALIDA);
    CHECK(servicio.centesimas_c == 10000);
    return true;
}

static bool AusenciaYCeroValido(void)
{
    LM35_Lectura sensor;
    Temperatura_Lectura servicio;
    CHECK(LM35_InicializarLectura(&sensor));
    CHECK(Adaptar(&servicio, &sensor) == TEMPERATURA_SIN_DATOS);
    CHECK(servicio.centesimas_c == 0);
    CHECK(LM35_ConvertirMuestra(&sensor, 0U, 4096U, 3000000U) == LM35_VALIDA);
    CHECK(Adaptar(&servicio, &sensor) == TEMPERATURA_VALIDA);
    CHECK(servicio.centesimas_c == 0 && servicio.estado == TEMPERATURA_VALIDA);
    return true;
}

int main(void)
{
    const struct {const char *nombre; bool (*ejecutar)(void);} casos[] = {
        {"S1/S6 conversion y adaptacion", ConversionYPropagacion},
        {"S5 error y recuperacion", ErrorYRecuperacion}, {"S4 disponibilidad", AusenciaYCeroValido}
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
