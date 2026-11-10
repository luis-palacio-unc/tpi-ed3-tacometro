#include "Mandos.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)
#define MAN MANDOS_SELECTOR_MODO_MANUAL
#define AUT MANDOS_SELECTOR_MODO_AUTOMATICO
#define NIN MANDOS_SELECTOR_MODO_NINGUNO
#define VAL MANDOS_SELECTOR_FUENTE_VALIDA

static bool Paso(Mandos_Selector *s, Mandos_ModoSelector modo, Mandos_FuenteSelector fuente,
                 uint32_t tiempo, Mandos_EstadoSelector estado, Mandos_ModoSelector esperado)
{
    CHECK(Mandos_ActualizarSelector(s, modo, fuente, tiempo) == estado);
    CHECK(s->estado == estado && s->modo == esperado);return true;
}

static bool ArranqueManual(void)
{
    Mandos_Selector s;CHECK(Mandos_InicializarSelector(&s, 20U));
    CHECK(s.estado == MANDOS_SELECTOR_SIN_DATOS && s.modo == NIN);
    CHECK(Paso(&s, MAN, VAL, 100U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 119U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 120U, MANDOS_SELECTOR_VALIDO, MAN));return true;
}

static bool ArranqueAutomatico(void)
{
    Mandos_Selector s;CHECK(Mandos_InicializarSelector(&s, 15U));
    CHECK(Paso(&s, AUT, VAL, 1000U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, 1014U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, 1015U, MANDOS_SELECTOR_VALIDO, AUT));return true;
}

static bool RebotesAmbosSentidos(void)
{
    Mandos_Selector s;CHECK(Mandos_InicializarSelector(&s, 10U));
    CHECK(Paso(&s, MAN, VAL, 0U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 10U, MANDOS_SELECTOR_VALIDO, MAN));
    const struct { uint32_t tiempo; Mandos_ModoSelector muestra, esperado; } casos[] = {
        {11U, AUT, MAN}, {14U, MAN, MAN}, {18U, AUT, MAN}, {27U, AUT, MAN}, {28U, AUT, AUT},
        {30U, MAN, AUT}, {35U, AUT, AUT}, {40U, MAN, AUT}, {49U, MAN, AUT}, {50U, MAN, MAN}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        CHECK(Paso(&s, casos[i].muestra, VAL, casos[i].tiempo, MANDOS_SELECTOR_VALIDO, casos[i].esperado));
    }
    return true;
}

static bool CeroYMarcasRepetidas(void)
{
    Mandos_Selector s;CHECK(Mandos_InicializarSelector(&s, 0U));
    CHECK(Paso(&s, MAN, VAL, 55U, MANDOS_SELECTOR_VALIDO, MAN));
    CHECK(Paso(&s, AUT, VAL, 55U, MANDOS_SELECTOR_VALIDO, AUT));
    CHECK(Paso(&s, MAN, VAL, 55U, MANDOS_SELECTOR_VALIDO, MAN));
    CHECK(Mandos_InicializarSelector(&s, 1U));
    CHECK(Paso(&s, AUT, VAL, 55U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, 55U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, 56U, MANDOS_SELECTOR_VALIDO, AUT));return true;
}

static bool AusenciaExigeEstabilidadNueva(void)
{
    Mandos_Selector s;CHECK(Mandos_InicializarSelector(&s, 5U));
    CHECK(Paso(&s, MAN, VAL, 0U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 5U, MANDOS_SELECTOR_VALIDO, MAN));
    CHECK(Paso(&s, NIN, MANDOS_SELECTOR_FUENTE_SIN_DATOS, 6U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, 10U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, 14U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, 15U, MANDOS_SELECTOR_VALIDO, AUT));return true;
}

static bool FalloFuenteExigeEstabilidadNueva(void)
{
    Mandos_Selector s;CHECK(Mandos_InicializarSelector(&s, 10U));
    CHECK(Paso(&s, AUT, VAL, 0U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, 10U, MANDOS_SELECTOR_VALIDO, AUT));
    CHECK(Paso(&s, (Mandos_ModoSelector)99, MANDOS_SELECTOR_FUENTE_ERROR, 12U, MANDOS_SELECTOR_ERROR_FUENTE, NIN));
    CHECK(Paso(&s, MAN, VAL, 15U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 24U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 25U, MANDOS_SELECTOR_VALIDO, MAN));return true;
}

static bool ModoInvalidoYRecuperacion(void)
{
    Mandos_Selector s;CHECK(Mandos_InicializarSelector(&s, 2U));
    CHECK(Paso(&s, MAN, VAL, 0U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 2U, MANDOS_SELECTOR_VALIDO, MAN));
    CHECK(Paso(&s, NIN, VAL, 3U, MANDOS_SELECTOR_ERROR_MODO, NIN));
    CHECK(Paso(&s, (Mandos_ModoSelector)99, VAL, 4U, MANDOS_SELECTOR_ERROR_MODO, NIN));
    CHECK(Paso(&s, MAN, VAL, 5U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 6U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 7U, MANDOS_SELECTOR_VALIDO, MAN));return true;
}

static bool ArgumentosYPrioridades(void)
{
    CHECK(!Mandos_InicializarSelector(NULL, UINT32_MAX));
    CHECK(Mandos_ActualizarSelector(NULL, (Mandos_ModoSelector)99, (Mandos_FuenteSelector)99, 0U) == MANDOS_SELECTOR_ERROR_ARGUMENTO);
    Mandos_Selector s;CHECK(!Mandos_InicializarSelector(&s, UINT32_MAX));
    CHECK(Paso(&s, (Mandos_ModoSelector)99, (Mandos_FuenteSelector)99, 0U, MANDOS_SELECTOR_ERROR_CONFIGURACION, NIN));
    CHECK(Mandos_InicializarSelector(&s, 0U));CHECK(Paso(&s, MAN, VAL, 100U, MANDOS_SELECTOR_VALIDO, MAN));
    CHECK(Paso(&s, NIN, (Mandos_FuenteSelector)99, 1U, MANDOS_SELECTOR_ERROR_ARGUMENTO, NIN));
    CHECK(Paso(&s, MAN, VAL, 99U, MANDOS_SELECTOR_ERROR_TIEMPO, NIN));
    CHECK(Paso(&s, MAN, VAL, 101U, MANDOS_SELECTOR_VALIDO, MAN));
    CHECK(Paso(&s, (Mandos_ModoSelector)99, MANDOS_SELECTOR_FUENTE_ERROR, 100U, MANDOS_SELECTOR_ERROR_TIEMPO, NIN));
    CHECK(Paso(&s, MAN, VAL, 2000U, MANDOS_SELECTOR_VALIDO, MAN));
    CHECK(Paso(&s, (Mandos_ModoSelector)99, MANDOS_SELECTOR_FUENTE_ERROR, 2001U, MANDOS_SELECTOR_ERROR_FUENTE, NIN));
    CHECK(Paso(&s, MAN, VAL, 2000U, MANDOS_SELECTOR_ERROR_TIEMPO, NIN));
    CHECK(Paso(&s, MAN, VAL, 3000U, MANDOS_SELECTOR_VALIDO, MAN));
    CHECK(Paso(&s, NIN, VAL, 3001U, MANDOS_SELECTOR_ERROR_MODO, NIN));
    CHECK(Paso(&s, MAN, VAL, 3000U, MANDOS_SELECTOR_ERROR_TIEMPO, NIN));
    CHECK(Paso(&s, NIN, MANDOS_SELECTOR_FUENTE_SIN_DATOS, 4000U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 3999U, MANDOS_SELECTOR_ERROR_TIEMPO, NIN));
    return true;
}

static bool RetrocesoYNuevaEpoca(void)
{
    Mandos_Selector s;CHECK(Mandos_InicializarSelector(&s, 5U));
    CHECK(Paso(&s, MAN, VAL, 100U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 105U, MANDOS_SELECTOR_VALIDO, MAN));
    CHECK(Paso(&s, MAN, VAL, 104U, MANDOS_SELECTOR_ERROR_TIEMPO, NIN));
    CHECK(Paso(&s, MAN, VAL, 80U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 84U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, 85U, MANDOS_SELECTOR_VALIDO, MAN));return true;
}

static bool RolloverTemporal(void)
{
    Mandos_Selector s;CHECK(Mandos_InicializarSelector(&s, 10U));
    CHECK(Paso(&s, AUT, VAL, UINT32_MAX - 5U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, 3U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, 4U, MANDOS_SELECTOR_VALIDO, AUT));
    CHECK(Paso(&s, AUT, VAL, 4U, MANDOS_SELECTOR_VALIDO, AUT));return true;
}

static bool IntervaloMaximoYSemirrango(void)
{
    Mandos_Selector s;CHECK(MANDOS_SELECTOR_INTERVALO_MAX_MS == (uint32_t)INT32_MAX);
    CHECK(Mandos_InicializarSelector(&s, MANDOS_SELECTOR_INTERVALO_MAX_MS));
    CHECK(Paso(&s, MAN, VAL, 0U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, (uint32_t)INT32_MAX - 1U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, MAN, VAL, (uint32_t)INT32_MAX, MANDOS_SELECTOR_VALIDO, MAN));
    CHECK(Paso(&s, MAN, VAL, UINT32_MAX, MANDOS_SELECTOR_ERROR_TIEMPO, NIN));
    CHECK(Mandos_InicializarSelector(&s, MANDOS_SELECTOR_INTERVALO_MAX_MS));
    const uint32_t inicio = UINT32_MAX - 15U;
    CHECK(Paso(&s, AUT, VAL, inicio, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, inicio + (uint32_t)INT32_MAX, MANDOS_SELECTOR_VALIDO, AUT));return true;
}

static bool ConfiguracionYReinicio(void)
{
    Mandos_Selector s;const uint32_t malas[] = {(uint32_t)INT32_MAX + 1U, UINT32_MAX};
    for (size_t i = 0U; i < sizeof(malas) / sizeof(malas[0]); ++i) {
        CHECK(!Mandos_InicializarSelector(&s, malas[i]));
        CHECK(s.estado == MANDOS_SELECTOR_ERROR_CONFIGURACION && s.modo == NIN);
        CHECK(Paso(&s, MAN, VAL, 10U, MANDOS_SELECTOR_ERROR_CONFIGURACION, NIN));
    }
    CHECK(Mandos_InicializarSelector(&s, 0U));CHECK(Paso(&s, AUT, VAL, 77U, MANDOS_SELECTOR_VALIDO, AUT));
    CHECK(Mandos_InicializarSelector(&s, 8U));CHECK(s.estado == MANDOS_SELECTOR_SIN_DATOS && s.modo == NIN);
    CHECK(Paso(&s, AUT, VAL, 1U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, 8U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&s, AUT, VAL, 9U, MANDOS_SELECTOR_VALIDO, AUT));return true;
}

static bool InstanciasIndependientes(void)
{
    Mandos_Selector a, b;CHECK(Mandos_InicializarSelector(&a, 5U));CHECK(Mandos_InicializarSelector(&b, 20U));
    CHECK(Paso(&a, MAN, VAL, 0U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&b, AUT, VAL, 0U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&a, MAN, VAL, 5U, MANDOS_SELECTOR_VALIDO, MAN));
    CHECK(Paso(&b, AUT, VAL, 19U, MANDOS_SELECTOR_SIN_DATOS, NIN));
    CHECK(Paso(&a, NIN, MANDOS_SELECTOR_FUENTE_ERROR, 6U, MANDOS_SELECTOR_ERROR_FUENTE, NIN));
    CHECK(Paso(&b, AUT, VAL, 20U, MANDOS_SELECTOR_VALIDO, AUT));
    CHECK(Mandos_InicializarSelector(&a, 1U));
    CHECK(b.estado == MANDOS_SELECTOR_VALIDO && b.modo == AUT);return true;
}

/* Referencia por historial completo y tiempo absoluto amplio; no copia los metadatos incrementales. */
static bool OraculoHistorial(void)
{
    const uint32_t intervalos[] = {0U, 1U, 20U, (uint32_t)INT32_MAX};
    const uint32_t saltos[] = {0U, 1U, 4U, 19U, (uint32_t)INT32_MAX};
    struct { uint64_t tiempo; Mandos_ModoSelector modo; Mandos_FuenteSelector fuente; } historia[1024];
    unsigned cruces = 0U;uint32_t semilla = 0xB10U;
    for (size_t c = 0U; c < sizeof(intervalos) / sizeof(intervalos[0]); ++c) {
        Mandos_Selector s;CHECK(Mandos_InicializarSelector(&s, intervalos[c]));
        Mandos_ModoSelector confirmado = NIN;uint64_t absoluto = UINT32_MAX - 7U;
        uint32_t anterior = (uint32_t)absoluto;
        for (size_t i = 0U; i < 1024U; ++i) {
            semilla = semilla * 1664525U + 1013904223U;
            absoluto += saltos[(semilla >> 24U) % 5U];
            const uint32_t marca = (uint32_t)absoluto;if (marca < anterior) { ++cruces; }anterior = marca;
            const unsigned categoria = (semilla >> 16U) % 10U;
            const Mandos_FuenteSelector fuente = categoria < 8U ? VAL :
                (categoria == 8U ? MANDOS_SELECTOR_FUENTE_SIN_DATOS : MANDOS_SELECTOR_FUENTE_ERROR);
            const Mandos_ModoSelector modo = semilla & 0x1000U ? MAN : AUT;
            historia[i].tiempo = absoluto;historia[i].modo = modo;historia[i].fuente = fuente;
            Mandos_EstadoSelector esperado;
            if (fuente != VAL) {
                confirmado = NIN;esperado = fuente == MANDOS_SELECTOR_FUENTE_SIN_DATOS ? MANDOS_SELECTOR_SIN_DATOS : MANDOS_SELECTOR_ERROR_FUENTE;
            } else {
                size_t primero = i;
                while (primero > 0U && historia[primero - 1U].fuente == VAL && historia[primero - 1U].modo == modo) { --primero; }
                if (absoluto - historia[primero].tiempo >= intervalos[c]) { confirmado = modo; }
                esperado = confirmado == NIN ? MANDOS_SELECTOR_SIN_DATOS : MANDOS_SELECTOR_VALIDO;
            }
            CHECK(Paso(&s, modo, fuente, marca, esperado, confirmado));
        }
    }
    CHECK(cruces > 0U);printf("PASS referencia 4096 muestras, %u rollovers\n", cruces);return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S1 inicio manual", ArranqueManual}, {"S1 inicio auto", ArranqueAutomatico},
        {"S2 rebotes ambos sentidos", RebotesAmbosSentidos}, {"S2/S3 cero/repetidas", CeroYMarcasRepetidas},
        {"S4 ausencia/recuperacion", AusenciaExigeEstabilidadNueva}, {"S4 fallo/recuperacion", FalloFuenteExigeEstabilidadNueva},
        {"S4 modo invalido", ModoInvalidoYRecuperacion}, {"S4 prioridades/NULL", ArgumentosYPrioridades},
        {"S3 retroceso/nueva epoca", RetrocesoYNuevaEpoca}, {"S3 rollover", RolloverTemporal},
        {"S3/S5 maximo/semirrango", IntervaloMaximoYSemirrango}, {"S5 config/reinicio", ConfiguracionYReinicio},
        {"S5 instancias", InstanciasIndependientes}, {"S1-S5 oraculo historial", OraculoHistorial}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre);return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));return 0;
}
