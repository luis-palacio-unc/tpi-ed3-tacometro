#include "Procesamiento.h"
#include "Control.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return false; } } while (0)
static const Procesamiento_ConfigRegulador defaults = {
    TPI_CONTROL_KP_NUM, TPI_CONTROL_KP_DEN, TPI_CONTROL_KI_NUM,
    TPI_CONTROL_KI_DEN, TPI_CONTROL_MAX_PERIODOS_SIN_MEDICION
};

static bool PasoYEmpate(void)
{
    Procesamiento_Regulador r;
    uint16_t u = 999U;
    Medicion_Velocidad_Lectura m = {0U, MEDICION_VELOCIDAD_VALIDA};
    CHECK(Procesamiento_InicializarRegulador(&r, &defaults));
    CHECK(Procesamiento_Regular(&r, 100U, &m, &u) == PROCESAMIENTO_REGULADOR_VALIDO && u == 400U && r.integral_num == 100000);
    CHECK(Procesamiento_Regular(&r, 100U, &m, &u) == PROCESAMIENTO_REGULADOR_VALIDO && u == 500U && r.integral_num == 200000);
    const Procesamiento_ConfigRegulador mitad = {1U, 1000U, 1U, 1000U, 4U};
    CHECK(Procesamiento_InicializarRegulador(&r, &mitad));
    m.rpm_milesimas = 500U;
    CHECK(Procesamiento_Regular(&r, 1U, &m, &u) == PROCESAMIENTO_REGULADOR_VALIDO && u == 1U);
    CHECK(r.integral_num == 500 && m.rpm_milesimas == 500U && m.estado == MEDICION_VELOCIDAD_VALIDA);
    return true;
}

static bool AntiWindup(void)
{
    Procesamiento_Regulador r;
    uint16_t u;
    Medicion_Velocidad_Lectura m = {0U, MEDICION_VELOCIDAD_VALIDA};
    CHECK(Procesamiento_InicializarRegulador(&r, &defaults));
    (void)Procesamiento_Regular(&r, 185U, &m, &u);
    CHECK(u == 740U && r.integral_num == 185000);
    (void)Procesamiento_Regular(&r, 185U, &m, &u);
    CHECK(u == 925U && r.integral_num == 370000);
    for (unsigned k = 0U; k < 40U; ++k) {
        CHECK(Procesamiento_Regular(&r, 185U, &m, &u) == PROCESAMIENTO_REGULADOR_VALIDO);
        CHECK(u == 1000U && r.integral_num == 370000);
    }
    (void)Procesamiento_Regular(&r, 100U, &m, &u);
    CHECK(u == 770U && r.integral_num == 470000);
    m.rpm_milesimas = UINT32_MAX;
    (void)Procesamiento_Regular(&r, 100U, &m, &u);
    CHECK(u == 0U && r.integral_num == 470000 && r.ultima_valida == UINT32_MAX);
    m.rpm_milesimas = 0U;
    (void)Procesamiento_Regular(&r, 100U, &m, &u);
    CHECK(u == 870U);
    return true;
}

static bool LimitesIntegralKiMayor(void)
{
    const Procesamiento_ConfigRegulador cfg = {1U, 1000U, 1000U, 1U, 4U};
    Procesamiento_Regulador r;
    uint16_t u = 777U;
    Medicion_Velocidad_Lectura m = {0U, MEDICION_VELOCIDAD_VALIDA};
    CHECK(Procesamiento_InicializarRegulador(&r, &cfg));
    CHECK(Procesamiento_Regular(&r, 100U, &m, &u) == PROCESAMIENTO_REGULADOR_VALIDO);
    CHECK(u == 100U && r.integral_num == 1000);
    m.rpm_milesimas = 150000U;
    CHECK(Procesamiento_Regular(&r, 100U, &m, &u) == PROCESAMIENTO_REGULADOR_VALIDO);
    CHECK(u == 950U && r.integral_num == 0);
    return true;
}

static bool CongelamientoInferiorEnCero(void)
{
    Procesamiento_Regulador r;
    uint16_t u = 777U;
    Medicion_Velocidad_Lectura m = {0U, MEDICION_VELOCIDAD_VALIDA};
    CHECK(Procesamiento_InicializarRegulador(&r, &defaults));
    for (unsigned k = 0U; k < 4U; ++k) {
        CHECK(Procesamiento_Regular(&r, 1U, &m, &u) == PROCESAMIENTO_REGULADOR_VALIDO);
    }
    CHECK(r.integral_num == 4000);
    m.rpm_milesimas = 2000U;
    CHECK(Procesamiento_Regular(&r, 1U, &m, &u) == PROCESAMIENTO_REGULADOR_VALIDO);
    CHECK(u == 0U && r.integral_num == 4000);
    m.rpm_milesimas = 1000U;
    CHECK(Procesamiento_Regular(&r, 1U, &m, &u) == PROCESAMIENTO_REGULADOR_VALIDO);
    CHECK(u == 4U && r.integral_num == 4000);
    return true;
}

static bool RetencionYRecuperacion(void)
{
    const Medicion_Velocidad_Estado estados[] = {MEDICION_VELOCIDAD_SIN_DATOS,
        MEDICION_VELOCIDAD_ERROR_ARGUMENTO, MEDICION_VELOCIDAD_ERROR_DURACION,
        MEDICION_VELOCIDAD_ERROR_RANGO, MEDICION_VELOCIDAD_ERROR_PULSOS_POR_VUELTA,
        (Medicion_Velocidad_Estado)77};
    for (size_t i = 0U; i < sizeof(estados)/sizeof(estados[0]); ++i) {
        Procesamiento_Regulador r;
        Medicion_Velocidad_Lectura m = {UINT32_MAX, estados[i]};
        uint16_t u = 777U;
        CHECK(Procesamiento_InicializarRegulador(&r, &defaults));
        CHECK(Procesamiento_Regular(&r, 100U, &m, &u) == PROCESAMIENTO_REGULADOR_SIN_MEDICION && u == 0U);
        m = (Medicion_Velocidad_Lectura){0U, MEDICION_VELOCIDAD_VALIDA};
        (void)Procesamiento_Regular(&r, 100U, &m, &u);
        (void)Procesamiento_Regular(&r, 100U, &m, &u);
        m = (Medicion_Velocidad_Lectura){UINT32_MAX, estados[i]};
        for (uint32_t n = 1U; n <= 4U; ++n) {
            CHECK(Procesamiento_Regular(&r, 100U, &m, &u) == PROCESAMIENTO_REGULADOR_RETENIENDO_MEDICION);
            CHECK(u == 600U && r.integral_num == 200000 && r.periodos_invalidos == n && r.ultima_valida == 0U);
        }
        CHECK(Procesamiento_Regular(&r, 100U, &m, &u) == PROCESAMIENTO_REGULADOR_SIN_MEDICION);
        CHECK(u == 0U && r.integral_num == 0 && !r.tiene_medicion && r.periodos_invalidos == 0U);
        m = (Medicion_Velocidad_Lectura){50000U, MEDICION_VELOCIDAD_VALIDA};
        CHECK(Procesamiento_Regular(&r, 100U, &m, &u) == PROCESAMIENTO_REGULADOR_VALIDO && u == 200U);
        CHECK(m.rpm_milesimas == 50000U && r.periodos_invalidos == 0U);
    }
    return true;
}

static bool FronterasN(void)
{
    const uint32_t limites[] = {1U, 4U, 65535U};
    for (size_t i = 0U; i < sizeof(limites)/sizeof(limites[0]); ++i) {
        Procesamiento_ConfigRegulador cfg = defaults;
        cfg.max_periodos_sin_medicion = limites[i];
        Procesamiento_Regulador r;
        uint16_t u;
        Medicion_Velocidad_Lectura m = {0U, MEDICION_VELOCIDAD_VALIDA};
        CHECK(Procesamiento_InicializarRegulador(&r, &cfg));
        (void)Procesamiento_Regular(&r, 100U, &m, &u);
        m.estado = MEDICION_VELOCIDAD_SIN_DATOS;
        for (uint32_t k = 1U; k <= limites[i]; ++k) {
            CHECK(Procesamiento_Regular(&r, 100U, &m, &u) == PROCESAMIENTO_REGULADOR_RETENIENDO_MEDICION);
            CHECK(r.periodos_invalidos == k && r.integral_num == 100000);
        }
        CHECK(Procesamiento_Regular(&r, 100U, &m, &u) == PROCESAMIENTO_REGULADOR_SIN_MEDICION && u == 0U);
    }
    return true;
}

static bool RachaInterrumpidaYConsigna(void)
{
    Procesamiento_Regulador r;
    uint16_t u;
    Medicion_Velocidad_Lectura m = {0U, MEDICION_VELOCIDAD_VALIDA};
    CHECK(Procesamiento_InicializarRegulador(&r, &defaults));
    (void)Procesamiento_Regular(&r, 100U, &m, &u);
    m.estado = MEDICION_VELOCIDAD_SIN_DATOS;
    (void)Procesamiento_Regular(&r, 50U, &m, &u);
    CHECK(u == 300U && r.integral_num == 100000 && r.periodos_invalidos == 1U);
    m = (Medicion_Velocidad_Lectura){100000U, MEDICION_VELOCIDAD_VALIDA};
    (void)Procesamiento_Regular(&r, 100U, &m, &u);
    CHECK(u == 100U && r.integral_num == 100000 && r.periodos_invalidos == 0U);
    m.estado = MEDICION_VELOCIDAD_ERROR_RANGO;
    (void)Procesamiento_Regular(&r, 100U, &m, &u);
    CHECK(u == 100U && r.periodos_invalidos == 1U);
    CHECK(Procesamiento_ReiniciarRegulador(&r) && r.configurado && !r.tiene_medicion && r.integral_num == 0);
    CHECK(Procesamiento_Regular(&r, 100U, &m, &u) == PROCESAMIENTO_REGULADOR_SIN_MEDICION && u == 0U);
    return true;
}

static bool Configuracion(void)
{
    CHECK(!Procesamiento_InicializarRegulador(NULL, &defaults));
    CHECK(!Procesamiento_ReiniciarRegulador(NULL));
    for (unsigned campo = 0U; campo < 5U; ++campo) {
        const uint32_t malos[] = {0U, campo == 4U ? 65536U : 1001U, UINT32_MAX};
        for (size_t k = 0U; k < 3U; ++k) {
            Procesamiento_ConfigRegulador cfg = defaults;
            switch (campo) {
                case 0U: cfg.kp_num = malos[k]; break;
                case 1U: cfg.kp_den = malos[k]; break;
                case 2U: cfg.ki_num = malos[k]; break;
                case 3U: cfg.ki_den = malos[k]; break;
                default: cfg.max_periodos_sin_medicion = malos[k]; break;
            }
            Procesamiento_Regulador r;
            uint16_t u = 777U;
            Medicion_Velocidad_Lectura m = {0U, MEDICION_VELOCIDAD_VALIDA};
            CHECK(!Procesamiento_InicializarRegulador(&r, &cfg));
            CHECK(!r.configurado && !r.tiene_medicion && r.integral_num == 0);
            CHECK(Procesamiento_Regular(&r, 186U, &m, &u) == PROCESAMIENTO_REGULADOR_ERROR_CONFIGURACION && u == 0U);
        }
    }
    Procesamiento_ConfigRegulador cfg = defaults;
    Procesamiento_Regulador r;
    CHECK(Procesamiento_InicializarRegulador(&r, &cfg));
    cfg.kp_den = 0U;
    CHECK(r.config.kp_den == 1000U);
    CHECK(!Procesamiento_InicializarRegulador(&r, NULL) && !r.configurado);
    return true;
}

static bool PrioridadesYCero(void)
{
    Procesamiento_Regulador r;
    uint16_t u = 777U;
    Medicion_Velocidad_Lectura m = {0U, MEDICION_VELOCIDAD_VALIDA};
    CHECK(Procesamiento_Regular(NULL, 186U, NULL, &u) == PROCESAMIENTO_REGULADOR_ERROR_ARGUMENTO && u == 0U);
    CHECK(!Procesamiento_InicializarRegulador(&r, NULL));
    CHECK(Procesamiento_Regular(&r, 186U, NULL, &u) == PROCESAMIENTO_REGULADOR_ERROR_ARGUMENTO);
    CHECK(Procesamiento_Regular(&r, 0U, &m, &u) == PROCESAMIENTO_REGULADOR_ERROR_CONFIGURACION);
    CHECK(Procesamiento_InicializarRegulador(&r, &defaults));
    (void)Procesamiento_Regular(&r, 100U, &m, &u);
    m.estado = MEDICION_VELOCIDAD_ERROR_RANGO;
    CHECK(Procesamiento_Regular(&r, 186U, &m, &u) == PROCESAMIENTO_REGULADOR_ERROR_CONSIGNA && u == 0U && !r.tiene_medicion);
    CHECK(Procesamiento_Regular(&r, UINT32_MAX, &m, &u) == PROCESAMIENTO_REGULADOR_ERROR_CONSIGNA);
    CHECK(Procesamiento_Regular(&r, 0U, &m, &u) == PROCESAMIENTO_REGULADOR_PARADO && u == 0U);
    m.estado = MEDICION_VELOCIDAD_VALIDA;
    (void)Procesamiento_Regular(&r, 100U, &m, &u);
    CHECK(Procesamiento_Regular(&r, 0U, NULL, &u) == PROCESAMIENTO_REGULADOR_ERROR_ARGUMENTO && !r.tiene_medicion);
    (void)Procesamiento_Regular(&r, 100U, &m, &u);
    CHECK(Procesamiento_Regular(&r, 100U, &m, NULL) == PROCESAMIENTO_REGULADOR_ERROR_ARGUMENTO && r.integral_num == 0);
    CHECK(Procesamiento_Regular(&r, 0U, &m, &u) == PROCESAMIENTO_REGULADOR_PARADO && !r.tiene_medicion);
    return true;
}

static bool ExtremosEInstancias(void)
{
    const uint32_t velocidades[] = {0U,1U,184999U,185000U,185001U,UINT32_MAX};
    const Procesamiento_ConfigRegulador cfgs[] = {
        {1U,1U,1U,1U,1U}, {1000U,1U,1000U,1U,1U},
        {1U,1000U,1U,1000U,4U}, {1000U,1000U,1000U,1000U,65535U}
    };
    for (size_t a = 0U; a < sizeof(cfgs)/sizeof(cfgs[0]); ++a) {
        Procesamiento_Regulador r, otra;
        CHECK(Procesamiento_InicializarRegulador(&r, &cfgs[a]));
        CHECK(Procesamiento_InicializarRegulador(&otra, &defaults));
        uint16_t u;
        Medicion_Velocidad_Lectura m = {0U,MEDICION_VELOCIDAD_VALIDA};
        (void)Procesamiento_Regular(&otra,100U,&m,&u);
        for (size_t v = 0U; v < sizeof(velocidades)/sizeof(velocidades[0]); ++v) {
            m.rpm_milesimas = velocidades[v];
            CHECK(Procesamiento_Regular(&r,185U,&m,&u) == PROCESAMIENTO_REGULADOR_VALIDO && u <= 1000U);
            CHECK(r.ultima_valida == velocidades[v] && r.integral_num >= 0 && r.integral_num <= (int64_t)1000 * (int64_t)cfgs[a].ki_den);
            CHECK(otra.integral_num == 100000 && otra.ultima_valida == 0U);
        }
    }
    return true;
}

int main(void)
{
    static const struct { const char *nombre; bool (*test)(void); } casos[] = {
        {"S1 paso/empate",PasoYEmpate}, {"S2 anti-windup",AntiWindup},
        {"S2 limites integral Ki>Kp",LimitesIntegralKiMayor},
        {"S2 n0 e negativo",CongelamientoInferiorEnCero},
        {"S3 retencion/recuperacion",RetencionYRecuperacion}, {"S4 N/N+1",FronterasN},
        {"S3/S4 racha/consigna/reinicio",RachaInterrumpidaYConsigna},
        {"S5 configuracion",Configuracion}, {"S5 prioridades/cero",PrioridadesYCero},
        {"S6 extremos/instancias",ExtremosEInstancias}
    };
    for (size_t i=0U;i<sizeof(casos)/sizeof(casos[0]);++i) {
        if (!casos[i].test()) { fprintf(stderr,"FAIL %s\n",casos[i].nombre); return 1; }
        printf("PASS %s\n",casos[i].nombre);
    }
    puts("10/10 casos correctos");
    return 0;
}
