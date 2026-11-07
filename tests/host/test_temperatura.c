#include "Temperatura.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static bool Esperar(int32_t dato, Temperatura_EstadoFuente fuente,
                    Temperatura_Estado estado, int32_t esperado)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Actualizar(&lectura, dato, fuente) == estado);
    CHECK(lectura.estado == estado && lectura.centesimas_c == esperado);
    return true;
}

static bool Inicializacion(void)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Inicializar(&lectura));
    CHECK(lectura.estado == TEMPERATURA_SIN_DATOS && lectura.centesimas_c == 0);
    CHECK(TEMPERATURA_CENTESIMAS_POR_GRADO == 100U);
    return true;
}

static bool FuentesEnUnidadesFisicas(void)
{
    const int32_t datos[] = {2534, -725, 0, INT32_MIN, INT32_MAX};
    for (size_t i = 0U; i < sizeof(datos) / sizeof(datos[0]); ++i) {
        CHECK(Esperar(datos[i], TEMPERATURA_FUENTE_VALIDA, TEMPERATURA_VALIDA, datos[i]));
    }
    return true;
}

static bool CeroYDisponibilidad(void)
{
    CHECK(Esperar(0, TEMPERATURA_FUENTE_VALIDA, TEMPERATURA_VALIDA, 0));
    CHECK(Esperar(0, TEMPERATURA_FUENTE_SIN_DATOS, TEMPERATURA_SIN_DATOS, 0));
    CHECK(Esperar(9999, TEMPERATURA_FUENTE_SIN_DATOS, TEMPERATURA_SIN_DATOS, 0));
    return true;
}

static bool ErrorInvalidaDatoAnterior(void)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Actualizar(&lectura, 2534, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Actualizar(&lectura, 9999, TEMPERATURA_FUENTE_ERROR) == TEMPERATURA_ERROR_FUENTE);
    CHECK(lectura.estado == TEMPERATURA_ERROR_FUENTE && lectura.centesimas_c == 0);
    return true;
}

static bool AusenciaInvalidaDatoAnterior(void)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Actualizar(&lectura, 2534, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Actualizar(&lectura, 9999, TEMPERATURA_FUENTE_SIN_DATOS) == TEMPERATURA_SIN_DATOS);
    CHECK(lectura.estado == TEMPERATURA_SIN_DATOS && lectura.centesimas_c == 0);
    return true;
}

static bool Recuperacion(void)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Actualizar(&lectura, 2534, TEMPERATURA_FUENTE_ERROR) == TEMPERATURA_ERROR_FUENTE);
    CHECK(Temperatura_Actualizar(&lectura, -725, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(lectura.centesimas_c == -725);
    CHECK(Temperatura_Actualizar(&lectura, 0, TEMPERATURA_FUENTE_SIN_DATOS) == TEMPERATURA_SIN_DATOS);
    CHECK(Temperatura_Actualizar(&lectura, 0, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(lectura.centesimas_c == 0);
    return true;
}

static bool ArgumentosInvalidos(void)
{
    Temperatura_Lectura lectura;
    CHECK(!Temperatura_Inicializar(NULL));
    CHECK(Temperatura_Actualizar(NULL, 0, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_ERROR_ARGUMENTO);
    CHECK(Temperatura_Actualizar(NULL, 0, (Temperatura_EstadoFuente)99) == TEMPERATURA_ERROR_ARGUMENTO);
    CHECK(Temperatura_Actualizar(&lectura, 2534, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Actualizar(&lectura, 2534, (Temperatura_EstadoFuente)99) == TEMPERATURA_ERROR_ARGUMENTO);
    CHECK(lectura.estado == TEMPERATURA_ERROR_ARGUMENTO && lectura.centesimas_c == 0);
    return true;
}

static bool Reinicio(void)
{
    Temperatura_Lectura lectura;
    CHECK(Temperatura_Actualizar(&lectura, 2534, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Inicializar(&lectura));
    CHECK(lectura.estado == TEMPERATURA_SIN_DATOS && lectura.centesimas_c == 0);
    CHECK(Temperatura_Actualizar(&lectura, 2534, TEMPERATURA_FUENTE_ERROR) == TEMPERATURA_ERROR_FUENTE);
    CHECK(Temperatura_Inicializar(&lectura));
    CHECK(lectura.estado == TEMPERATURA_SIN_DATOS && lectura.centesimas_c == 0);
    return true;
}

static bool InstanciasIndependientes(void)
{
    Temperatura_Lectura a, b;
    CHECK(Temperatura_Actualizar(&a, 2534, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Actualizar(&b, -725, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Actualizar(&a, 0, TEMPERATURA_FUENTE_ERROR) == TEMPERATURA_ERROR_FUENTE);
    CHECK(b.centesimas_c == -725 && b.estado == TEMPERATURA_VALIDA);
    CHECK(Temperatura_Inicializar(&a));
    CHECK(b.centesimas_c == -725 && b.estado == TEMPERATURA_VALIDA);
    return true;
}

static bool SecuenciaDeFuentes(void)
{
    const struct {int32_t dato; Temperatura_EstadoFuente fuente; Temperatura_Estado estado; int32_t salida;} casos[] = {
        {2534, TEMPERATURA_FUENTE_VALIDA, TEMPERATURA_VALIDA, 2534},
        {-725, TEMPERATURA_FUENTE_VALIDA, TEMPERATURA_VALIDA, -725},
        {12345, TEMPERATURA_FUENTE_ERROR, TEMPERATURA_ERROR_FUENTE, 0},
        {INT32_MIN, TEMPERATURA_FUENTE_SIN_DATOS, TEMPERATURA_SIN_DATOS, 0},
        {INT32_MAX, TEMPERATURA_FUENTE_VALIDA, TEMPERATURA_VALIDA, INT32_MAX}
    };
    Temperatura_Lectura lectura;
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        CHECK(Temperatura_Actualizar(&lectura, casos[i].dato, casos[i].fuente) == casos[i].estado);
        CHECK(lectura.estado == casos[i].estado && lectura.centesimas_c == casos[i].salida);
    }
    return true;
}

int main(void)
{
    const struct {const char *nombre; bool (*ejecutar)(void);} casos[] = {
        {"S4 inicializacion", Inicializacion}, {"S4/S7 fuentes fisicas genericas", FuentesEnUnidadesFisicas},
        {"S4 cero y disponibilidad", CeroYDisponibilidad}, {"S5 error invalida", ErrorInvalidaDatoAnterior},
        {"S5 ausencia invalida", AusenciaInvalidaDatoAnterior}, {"S5 recuperacion", Recuperacion},
        {"S6 argumentos", ArgumentosInvalidos}, {"S4 reinicio", Reinicio},
        {"S7 instancias", InstanciasIndependientes}, {"S7 secuencia de fuentes", SecuenciaDeFuentes}
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
