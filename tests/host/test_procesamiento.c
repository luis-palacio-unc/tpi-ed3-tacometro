#include "Procesamiento.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

typedef bool (*Prueba)(void);

static bool Paso(Procesamiento_Contexto *ctx, Procesamiento_Modo modo,
                 uint16_t mando, uint64_t orden, const Procesamiento_Consigna *cmd,
                 Procesamiento_Salida *salida)
{
    const Procesamiento_Entradas entradas = {modo, mando, orden, cmd};
    return Procesamiento_Actualizar(ctx, &entradas, salida);
}

static bool EsCero(const Procesamiento_Salida *s)
{
    return s->estado == PROCESAMIENTO_AUTO_ESPERA_CONSIGNA &&
           s->fuente == PROCESAMIENTO_FUENTE_CERO &&
           s->mando_manual_por_mil == 0U && s->consigna_rpm == 0U;
}

static bool ManualLimites(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_MANUAL, 0U));
    const uint16_t valores[] = {0U, 1U, 500U, 999U, 1000U};
    for (size_t i = 0U; i < sizeof(valores) / sizeof(valores[0]); ++i) {
        CHECK(Paso(&ctx, PROCESAMIENTO_MODO_MANUAL, valores[i], 0U, NULL, &s));
        CHECK(s.estado == PROCESAMIENTO_MANUAL && s.fuente == PROCESAMIENTO_FUENTE_MANUAL);
        CHECK(s.mando_manual_por_mil == valores[i] && s.consigna_rpm == 0U);
    }
    return true;
}

static bool ManualIgnoraConsigna(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    const Procesamiento_Consigna cmd = {90U, 1U, true};
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_MANUAL, 0U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_MANUAL, 200U, 1U, &cmd, &s));
    CHECK(s.fuente == PROCESAMIENTO_FUENTE_MANUAL && s.mando_manual_por_mil == 200U);
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 2U, &cmd, &s));
    CHECK(EsCero(&s));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 3U, &cmd, &s));
    CHECK(EsCero(&s));
    return true;
}

static bool ArranqueAutomatico(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 10U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, UINT16_MAX, 10U, NULL, &s));
    CHECK(EsCero(&s));
    const Procesamiento_Consigna vieja = {80U, 10U, true};
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 1000U, 11U, &vieja, &s));
    CHECK(EsCero(&s));
    return true;
}

static bool ConsignaNuevaEIgnorarPotenciometro(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    const Procesamiento_Consigna cmd = {125U, 11U, true};
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 10U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 1000U, 11U, &cmd, &s));
    CHECK(s.estado == PROCESAMIENTO_AUTO_ACTIVO && s.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA);
    CHECK(s.consigna_rpm == 125U && s.mando_manual_por_mil == 0U);
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, UINT16_MAX, 12U, NULL, &s));
    CHECK(s.consigna_rpm == 125U && s.mando_manual_por_mil == 0U);
    CHECK(s.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA);
    return true;
}

static bool CambioYConsignaSimultaneos(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    Procesamiento_Consigna cmd = {50U, 5U, true};
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_MANUAL, 0U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 5U, &cmd, &s));
    CHECK(EsCero(&s));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 6U, &cmd, &s));
    CHECK(EsCero(&s));
    cmd.orden_recepcion = 7U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 7U, &cmd, &s));
    CHECK(s.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA && s.consigna_rpm == 50U);
    return true;
}

static bool ReentradaDescartaColaAnterior(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    Procesamiento_Consigna cmd = {40U, 1U, true};
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 1U, &cmd, &s));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_MANUAL, 600U, 2U, NULL, &s));
    CHECK(s.fuente == PROCESAMIENTO_FUENTE_MANUAL && s.consigna_rpm == 0U);
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 3U, NULL, &s));
    CHECK(EsCero(&s));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 4U, &cmd, &s));
    CHECK(EsCero(&s));
    cmd.orden_recepcion = 2U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 5U, &cmd, &s));
    CHECK(EsCero(&s));
    cmd.orden_recepcion = 6U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 6U, &cmd, &s));
    CHECK(s.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA);
    return true;
}

static bool DescartesMientrasEspera(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    const Procesamiento_Consigna eventos[] = {
        {70U, 9U, true}, {70U, 10U, true}, {70U, 11U, false}, {70U, 21U, true}
    };
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 10U));
    for (size_t i = 0U; i < sizeof(eventos) / sizeof(eventos[0]); ++i) {
        CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 20U, &eventos[i], &s));
        CHECK(EsCero(&s));
    }
    return true;
}

static bool DescartesNoSustituyenActiva(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    const Procesamiento_Consigna primera = {120U, 5U, true};
    const Procesamiento_Consigna descartes[] = {
        {90U, 6U, false}, {90U, 0U, true}, {90U, 4U, true},
        {90U, 5U, true}, {90U, 11U, true}
    };
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 5U, &primera, &s));
    for (size_t i = 0U; i < sizeof(descartes) / sizeof(descartes[0]); ++i) {
        CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 10U, &descartes[i], &s));
        CHECK(s.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA && s.consigna_rpm == 120U);
    }
    const Procesamiento_Consigna nueva = {150U, 12U, true};
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 12U, &nueva, &s));
    CHECK(s.consigna_rpm == 150U);
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 13U, &primera, &s));
    CHECK(s.consigna_rpm == 150U);
    return true;
}

static bool LimitesRPM(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    Procesamiento_Consigna cmd = {186U, 1U, true};
    CHECK(PROCESAMIENTO_RPM_MAX == 185U);
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 1U, &cmd, &s));
    CHECK(EsCero(&s));
    cmd.rpm = UINT32_MAX; cmd.orden_recepcion = 2U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 2U, &cmd, &s));
    CHECK(EsCero(&s));
    cmd.rpm = 0U; cmd.orden_recepcion = 3U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 3U, &cmd, &s));
    CHECK(s.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA && s.consigna_rpm == 0U);
    cmd.rpm = 185U; cmd.orden_recepcion = 4U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 4U, &cmd, &s));
    CHECK(s.consigna_rpm == 185U);
    cmd.rpm = 186U; cmd.orden_recepcion = 5U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 5U, &cmd, &s));
    CHECK(s.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA && s.consigna_rpm == 185U);
    cmd.rpm = UINT32_MAX; cmd.orden_recepcion = 6U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 6U, &cmd, &s));
    CHECK(s.consigna_rpm == 185U);
    cmd.rpm = 184U; cmd.orden_recepcion = 7U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 7U, &cmd, &s));
    CHECK(s.consigna_rpm == 184U);
    return true;
}

static bool ManualFueraDeRango(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_MANUAL, 0U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_MANUAL, 500U, 1U, NULL, &s));
    CHECK(!Paso(&ctx, PROCESAMIENTO_MODO_MANUAL, 1001U, 2U, NULL, &s));
    CHECK(EsCero(&s));
    CHECK(!Paso(&ctx, PROCESAMIENTO_MODO_MANUAL, UINT16_MAX, 3U, NULL, &s));
    CHECK(EsCero(&s));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_MANUAL, 1000U, 4U, NULL, &s));
    CHECK(s.fuente == PROCESAMIENTO_FUENTE_MANUAL && s.mando_manual_por_mil == 1000U);
    return true;
}

static bool ErrorModoYRecuperacion(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    Procesamiento_Consigna cmd = {50U, 1U, true};
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 1U, &cmd, &s));
    CHECK(!Paso(&ctx, (Procesamiento_Modo)-1, 0U, 2U, NULL, &s));
    CHECK(EsCero(&s));
    cmd.orden_recepcion = 3U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 3U, &cmd, &s));
    CHECK(EsCero(&s));
    cmd.orden_recepcion = 4U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 4U, &cmd, &s));
    CHECK(s.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA);
    return true;
}

static bool RegresionDeOrden(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    Procesamiento_Consigna cmd = {50U, 11U, true};
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 10U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 11U, &cmd, &s));
    CHECK(!Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 9U, &cmd, &s));
    CHECK(EsCero(&s));
    CHECK(!Paso(&ctx, PROCESAMIENTO_MODO_MANUAL, 100U, 10U, NULL, &s));
    CHECK(EsCero(&s));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 12U, &cmd, &s));
    CHECK(EsCero(&s));
    cmd.orden_recepcion = 13U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 13U, &cmd, &s));
    CHECK(s.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA);
    return true;
}

static bool InicializacionInvalida(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    Procesamiento_Consigna cmd = {50U, 6U, true};
    CHECK(!Procesamiento_Inicializar(NULL, PROCESAMIENTO_MODO_MANUAL, 0U));
    CHECK(!Procesamiento_Inicializar(&ctx, (Procesamiento_Modo)22, 5U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 6U, &cmd, &s));
    CHECK(EsCero(&s));
    cmd.orden_recepcion = 7U;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 7U, &cmd, &s));
    CHECK(s.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA);
    return true;
}

static bool ParametrosNulos(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    Procesamiento_Entradas e = {PROCESAMIENTO_MODO_AUTOMATICO, 0U, 0U, NULL};
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U));
    CHECK(!Procesamiento_Actualizar(NULL, &e, &s)); CHECK(EsCero(&s));
    CHECK(!Procesamiento_Actualizar(&ctx, NULL, &s)); CHECK(EsCero(&s));
    CHECK(!Procesamiento_Actualizar(&ctx, &e, NULL));
    CHECK(!Procesamiento_Actualizar(NULL, NULL, NULL));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 1U, NULL, &s));
    CHECK(EsCero(&s));
    return true;
}

static bool OrdenMaximoSinRollover(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    Procesamiento_Consigna cmd = {10U, UINT64_MAX - 1U, true};
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, UINT64_MAX - 2U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, UINT64_MAX - 1U, &cmd, &s));
    CHECK(s.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA);
    cmd.rpm = 20U; cmd.orden_recepcion = UINT64_MAX;
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, UINT64_MAX, &cmd, &s));
    CHECK(s.consigna_rpm == 20U);
    CHECK(!Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 0U, &cmd, &s));
    CHECK(EsCero(&s));
    return true;
}

static bool ContextosIndependientes(void)
{
    Procesamiento_Contexto a, b;
    Procesamiento_Salida sa, sb;
    const Procesamiento_Consigna cmd = {100U, 1U, true};
    CHECK(Procesamiento_Inicializar(&a, PROCESAMIENTO_MODO_MANUAL, 0U));
    CHECK(Procesamiento_Inicializar(&b, PROCESAMIENTO_MODO_AUTOMATICO, 0U));
    CHECK(Paso(&b, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 1U, &cmd, &sb));
    CHECK(Paso(&a, PROCESAMIENTO_MODO_MANUAL, 300U, 1U, NULL, &sa));
    CHECK(Paso(&b, PROCESAMIENTO_MODO_AUTOMATICO, 500U, 2U, NULL, &sb));
    CHECK(sa.fuente == PROCESAMIENTO_FUENTE_MANUAL && sa.mando_manual_por_mil == 300U);
    CHECK(sb.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA && sb.consigna_rpm == 100U);
    return true;
}

static bool ReiniciarPurgaConsigna(void)
{
    Procesamiento_Contexto ctx;
    Procesamiento_Salida s;
    const Procesamiento_Consigna cmd = {50U, 1U, true};
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 1U, &cmd, &s));
    CHECK(Procesamiento_Inicializar(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 10U));
    CHECK(Paso(&ctx, PROCESAMIENTO_MODO_AUTOMATICO, 0U, 10U, NULL, &s));
    CHECK(EsCero(&s));
    return true;
}

int main(void)
{
    const struct { const char *nombre; Prueba ejecutar; } pruebas[] = {
        {"S1 manual limites", ManualLimites}, {"S1/S5 manual ignora consigna", ManualIgnoraConsigna},
        {"S3 arranque automatico", ArranqueAutomatico},
        {"S2/S4 nueva y potenciometro ignorado", ConsignaNuevaEIgnorarPotenciometro},
        {"S3 cambio y consigna simultaneos", CambioYConsignaSimultaneos},
        {"S5 reentrada y cola anterior", ReentradaDescartaColaAnterior},
        {"S6 descartes en espera", DescartesMientrasEspera},
        {"S6 descartes con activa", DescartesNoSustituyenActiva}, {"S8 limites RPM", LimitesRPM},
        {"S7 manual fuera de rango", ManualFueraDeRango}, {"S7 modo y recuperacion", ErrorModoYRecuperacion},
        {"S7 orden regresivo", RegresionDeOrden}, {"S7 inicializacion invalida", InicializacionInvalida},
        {"S7 parametros nulos", ParametrosNulos}, {"S8 orden maximo", OrdenMaximoSinRollover},
        {"S8 contextos independientes", ContextosIndependientes},
        {"S3 reinicio", ReiniciarPurgaConsigna}
    };
    size_t correctas = 0U;
    for (size_t i = 0U; i < sizeof(pruebas) / sizeof(pruebas[0]); ++i) {
        if (!pruebas[i].ejecutar()) {
            fprintf(stderr, "FAIL %s\n", pruebas[i].nombre);
            return 1;
        }
        ++correctas;
        printf("PASS %s\n", pruebas[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", correctas, sizeof(pruebas) / sizeof(pruebas[0]));
    return 0;
}
