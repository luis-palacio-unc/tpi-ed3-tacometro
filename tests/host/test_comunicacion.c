#include "Comunicacion.h"
#include "Procesamiento.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

static Comunicacion_Resultado Enviar(Comunicacion_Contexto *ctx, const char *texto,
                                    uint64_t *orden)
{
    Comunicacion_Resultado resultado = COMUNICACION_OK;
    while (*texto != '\0') {
        ++*orden;
        Comunicacion_Resultado actual = Comunicacion_RecibirByte(ctx, (uint8_t)*texto, *orden);
        if (actual != COMUNICACION_OK) {
            resultado = actual;
        }
        ++texto;
    }
    return resultado;
}

static bool ValidarEvento(Comunicacion_Contexto *ctx, uint32_t rpm, uint64_t orden)
{
    Comunicacion_Consigna evento;
    CHECK(Comunicacion_ExtraerConsigna(ctx, &evento) == COMUNICACION_EXTRAIDA);
    CHECK(evento.rpm == rpm && evento.orden_recepcion == orden);
    return true;
}

static bool TramasValidas(void)
{
    const char *textos[] = {"RPM 0\n", "RPM 001\n", "RPM 184\n", "RPM 185\n", "RPM 000\r\n"};
    const uint32_t valores[] = {0U, 1U, 184U, 185U, 0U};
    Comunicacion_Contexto ctx;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(COMUNICACION_RPM_MAX == 185U && PROCESAMIENTO_RPM_MAX == 185U);
    for (size_t i = 0U; i < sizeof(valores) / sizeof(valores[0]); ++i) {
        CHECK(Enviar(&ctx, textos[i], &orden) == COMUNICACION_ENCOLADA);
        CHECK(ValidarEvento(&ctx, valores[i], orden));
    }
    return true;
}

static bool FragmentadaYCRLF(void)
{
    Comunicacion_Contexto ctx;
    Comunicacion_Consigna evento;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(Enviar(&ctx, "R", &orden) == COMUNICACION_OK);
    CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    CHECK(Enviar(&ctx, "PM 18", &orden) == COMUNICACION_OK);
    CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    CHECK(Enviar(&ctx, "5\r", &orden) == COMUNICACION_OK);
    CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    CHECK(Enviar(&ctx, "\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(orden == 9U);
    CHECK(ValidarEvento(&ctx, 185U, 9U));
    CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    return true;
}

static bool ConsecutivasYMarcasOriginales(void)
{
    Comunicacion_Contexto ctx;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(Enviar(&ctx, "RPM 0\nRPM 185\r\nRPM 25\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(orden == 22U);
    CHECK(ValidarEvento(&ctx, 0U, 6U));
    CHECK(ValidarEvento(&ctx, 185U, 15U));
    CHECK(ValidarEvento(&ctx, 25U, 22U));
    return true;
}

static bool ValoresFueraDeRango(void)
{
    Comunicacion_Contexto ctx;
    Comunicacion_Consigna evento;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(Enviar(&ctx, "RPM 186\n", &orden) == COMUNICACION_ERROR_RANGO);
    CHECK(Enviar(&ctx, "RPM 999\r\n", &orden) == COMUNICACION_ERROR_RANGO);
    CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    CHECK(Enviar(&ctx, "RPM 185\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(ValidarEvento(&ctx, 185U, orden));
    return true;
}

static bool FormatosInvalidos(void)
{
    const char *malas[] = {"\n", "RPM \n", "RPM -1\n", "RPM +1\n", "RPM 1.5\n", "rpm 5\n",
                          " RPM 1\n", "RPM 1 \n", "RPM  1\n", "RPM 1x\n", "RPM 1000\n",
                          "RPM 1\r\r\n", "RPM\t1\n", "RPM 1\r2\n"};
    Comunicacion_Contexto ctx;
    Comunicacion_Consigna evento;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    for (size_t i = 0U; i < sizeof(malas) / sizeof(malas[0]); ++i) {
        CHECK(Enviar(&ctx, malas[i], &orden) == COMUNICACION_ERROR_FORMATO);
        CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    }
    CHECK(Enviar(&ctx, "RPM 10\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(ValidarEvento(&ctx, 10U, orden));
    return true;
}

static bool TodosLosBytesEnCampoNumerico(void)
{
    for (uint16_t valor = 0U; valor <= 255U; ++valor) {
        Comunicacion_Contexto ctx;
        Comunicacion_Consigna evento;
        uint64_t orden = 0U;
        CHECK(Comunicacion_Inicializar(&ctx, orden));
        CHECK(Enviar(&ctx, "RPM ", &orden) == COMUNICACION_OK);
        CHECK(Comunicacion_RecibirByte(&ctx, (uint8_t)valor, ++orden) != COMUNICACION_ENCOLADA);
        Comunicacion_Resultado fin = Enviar(&ctx, "\n", &orden);
        if (valor >= 48U && valor <= 57U) {
            CHECK(fin == COMUNICACION_ENCOLADA);
            CHECK(ValidarEvento(&ctx, (uint32_t)(valor - 48U), orden));
        } else {
            CHECK(fin == COMUNICACION_ERROR_FORMATO);
            CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
        }
    }
    return true;
}

static bool ExcesoLongitudYRecuperacion(void)
{
    Comunicacion_Contexto ctx;
    Comunicacion_Consigna evento;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(Enviar(&ctx, "AAAAAAAA", &orden) == COMUNICACION_OK);
    CHECK(Comunicacion_RecibirByte(&ctx, (uint8_t)'A', ++orden) == COMUNICACION_ERROR_LONGITUD);
    CHECK(Enviar(&ctx, "RPM 185\n", &orden) == COMUNICACION_OK);
    CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    CHECK(Enviar(&ctx, "RPM 185\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(ValidarEvento(&ctx, 185U, orden));
    return true;
}

static bool EnterosExtremosNoDesbordan(void)
{
    Comunicacion_Contexto ctx;
    Comunicacion_Consigna evento;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(Enviar(&ctx, "RPM 4294967295\n", &orden) == COMUNICACION_ERROR_LONGITUD);
    CHECK(Enviar(&ctx, "RPM 18446744073709551616\n", &orden) == COMUNICACION_ERROR_LONGITUD);
    CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    CHECK(Enviar(&ctx, "RPM 0\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(ValidarEvento(&ctx, 0U, orden));
    return true;
}

static bool ColaLlenaConservaFIFOYRecupera(void)
{
    Comunicacion_Contexto ctx;
    Comunicacion_Consigna evento;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(Enviar(&ctx, "RPM 0\nRPM 1\nRPM 2\nRPM 3\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(Enviar(&ctx, "RPM 4\n", &orden) == COMUNICACION_ERROR_COLA_LLENA);
    CHECK(ValidarEvento(&ctx, 0U, 6U));
    CHECK(Enviar(&ctx, "RPM 5\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(ValidarEvento(&ctx, 1U, 12U));
    CHECK(ValidarEvento(&ctx, 2U, 18U));
    CHECK(ValidarEvento(&ctx, 3U, 24U));
    CHECK(ValidarEvento(&ctx, 5U, 36U));
    CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    return true;
}

static bool ColaCircularRepetida(void)
{
    Comunicacion_Contexto ctx;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    for (unsigned int i = 0U; i < 100U; ++i) {
        CHECK(Enviar(&ctx, "RPM 100\n", &orden) == COMUNICACION_ENCOLADA);
        CHECK(ValidarEvento(&ctx, 100U, orden));
    }
    return true;
}

static bool OrdenRepetidoYRegresivo(void)
{
    Comunicacion_Contexto ctx;
    Comunicacion_Consigna evento;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(Enviar(&ctx, "RPM 1\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(Comunicacion_RecibirByte(&ctx, (uint8_t)'R', ++orden) == COMUNICACION_OK);
    CHECK(Comunicacion_RecibirByte(&ctx, (uint8_t)'P', orden) == COMUNICACION_ERROR_ORDEN);
    CHECK(Comunicacion_RecibirByte(&ctx, (uint8_t)'M', 1U) == COMUNICACION_ERROR_ORDEN);
    CHECK(Enviar(&ctx, " 20\n", &orden) == COMUNICACION_OK);
    CHECK(ValidarEvento(&ctx, 1U, 6U));
    CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    CHECK(Enviar(&ctx, "RPM 30\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(ValidarEvento(&ctx, 30U, orden));
    return true;
}

static bool OrdenInvalidoEnTerminador(void)
{
    Comunicacion_Contexto ctx;
    Comunicacion_Consigna evento;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(Enviar(&ctx, "RPM 1", &orden) == COMUNICACION_OK);
    CHECK(Comunicacion_RecibirByte(&ctx, (uint8_t)'\n', orden) == COMUNICACION_ERROR_ORDEN);
    CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    CHECK(Enviar(&ctx, "RPM 2\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(ValidarEvento(&ctx, 2U, orden));
    return true;
}

static bool OrdenMaximoYRollover(void)
{
    Comunicacion_Contexto ctx;
    uint64_t orden = UINT64_MAX - 6U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(Enviar(&ctx, "RPM 0\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(orden == UINT64_MAX);
    CHECK(Comunicacion_RecibirByte(&ctx, (uint8_t)'R', 0U) == COMUNICACION_ERROR_ORDEN);
    CHECK(ValidarEvento(&ctx, 0U, UINT64_MAX));
    return true;
}

static bool ParametrosNulosYVacio(void)
{
    Comunicacion_Contexto ctx;
    Comunicacion_Consigna evento = {185U, 999U};
    uint64_t orden = 0U;
    CHECK(!Comunicacion_Inicializar(NULL, 0U));
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(Comunicacion_RecibirByte(NULL, 0U, 1U) == COMUNICACION_ERROR_ARGUMENTO);
    CHECK(Comunicacion_ExtraerConsigna(NULL, &evento) == COMUNICACION_ERROR_ARGUMENTO);
    CHECK(evento.rpm == 0U && evento.orden_recepcion == 0U);
    CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    CHECK(Comunicacion_ExtraerConsigna(&ctx, NULL) == COMUNICACION_ERROR_ARGUMENTO);
    CHECK(Enviar(&ctx, "RPM 7\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(Comunicacion_ExtraerConsigna(&ctx, NULL) == COMUNICACION_ERROR_ARGUMENTO);
    CHECK(ValidarEvento(&ctx, 7U, orden));
    return true;
}

static bool ReinicioPurgaColaYTrama(void)
{
    Comunicacion_Contexto ctx;
    Comunicacion_Consigna evento;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(Enviar(&ctx, "RPM 1\nRPM 18", &orden) == COMUNICACION_ENCOLADA);
    CHECK(Comunicacion_Inicializar(&ctx, orden));
    CHECK(Comunicacion_ExtraerConsigna(&ctx, &evento) == COMUNICACION_VACIA);
    CHECK(Enviar(&ctx, "5\n", &orden) == COMUNICACION_ERROR_FORMATO);
    CHECK(Enviar(&ctx, "RPM 2\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(ValidarEvento(&ctx, 2U, orden));
    return true;
}

static bool InstanciasIndependientes(void)
{
    Comunicacion_Contexto a, b;
    uint64_t oa = 0U, ob = 100U;
    CHECK(Comunicacion_Inicializar(&a, oa)); CHECK(Comunicacion_Inicializar(&b, ob));
    CHECK(Enviar(&a, "RPM 1", &oa) == COMUNICACION_OK);
    CHECK(Enviar(&b, "RPM 2\n", &ob) == COMUNICACION_ENCOLADA);
    CHECK(Enviar(&a, "\n", &oa) == COMUNICACION_ENCOLADA);
    CHECK(ValidarEvento(&a, 1U, oa)); CHECK(ValidarEvento(&b, 2U, ob));
    return true;
}

static bool IntegracionAntiguaYNuevaConB01(void)
{
    Comunicacion_Contexto rx;
    Procesamiento_Contexto app;
    Comunicacion_Consigna evento;
    Procesamiento_Salida salida;
    uint64_t orden = 0U;
    CHECK(Comunicacion_Inicializar(&rx, orden));
    CHECK(Procesamiento_Inicializar(&app, PROCESAMIENTO_MODO_MANUAL, orden));
    CHECK(Enviar(&rx, "RPM 50\n", &orden) == COMUNICACION_ENCOLADA);
    Procesamiento_Entradas entrada = {PROCESAMIENTO_MODO_AUTOMATICO, 0U, ++orden, NULL};
    CHECK(Procesamiento_Actualizar(&app, &entrada, &salida));
    CHECK(salida.fuente == PROCESAMIENTO_FUENTE_CERO);
    CHECK(Comunicacion_ExtraerConsigna(&rx, &evento) == COMUNICACION_EXTRAIDA);
    Procesamiento_Consigna cmd = {evento.rpm, evento.orden_recepcion, true};
    entrada.consigna = &cmd;
    CHECK(Procesamiento_Actualizar(&app, &entrada, &salida));
    CHECK(salida.fuente == PROCESAMIENTO_FUENTE_CERO);
    CHECK(Enviar(&rx, "RPM 185\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(Comunicacion_ExtraerConsigna(&rx, &evento) == COMUNICACION_EXTRAIDA);
    cmd.rpm = evento.rpm; cmd.orden_recepcion = evento.orden_recepcion;
    entrada.orden_actual = orden;
    CHECK(Procesamiento_Actualizar(&app, &entrada, &salida));
    CHECK(salida.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA && salida.consigna_rpm == 185U);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S1 limites/ceros iniciales", TramasValidas}, {"S3 fragmentada/CRLF", FragmentadaYCRLF},
        {"S3 consecutivas/marcas FIFO", ConsecutivasYMarcasOriginales},
        {"S2 rango", ValoresFueraDeRango}, {"S2 formato", FormatosInvalidos},
        {"S2 todos los bytes", TodosLosBytesEnCampoNumerico},
        {"S5 exceso y resincronizacion", ExcesoLongitudYRecuperacion},
        {"S2 enteros extremos", EnterosExtremosNoDesbordan},
        {"S6 cola llena/FIFO", ColaLlenaConservaFIFOYRecupera},
        {"S6 anillo repetido", ColaCircularRepetida}, {"S4 orden repetido/regresivo", OrdenRepetidoYRegresivo},
        {"S4 terminador con orden invalido", OrdenInvalidoEnTerminador},
        {"S4 frontera UINT64_MAX", OrdenMaximoYRollover}, {"S7 NULL/vacio", ParametrosNulosYVacio},
        {"S7 reinicio", ReinicioPurgaColaYTrama}, {"S7 instancias", InstanciasIndependientes},
        {"S8 integracion B01", IntegracionAntiguaYNuevaConB01}
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
