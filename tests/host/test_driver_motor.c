#include "Driver_Motor.h"
#include <stddef.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

typedef struct {
    unsigned llamadas;
    uint16_t ultimo_solicitado;
    bool acepta;
    unsigned ruta;
} SalidaEmulada;

static bool Escribir(void *contexto, uint16_t mando_por_mil)
{
    SalidaEmulada *salida = contexto;
    if (salida == NULL) {
        return false;
    }
    ++salida->llamadas;
    salida->ultimo_solicitado = mando_por_mil;
    salida->ruta = 1U;
    return salida->acepta;
}

static bool EscribirSegunda(void *contexto, uint16_t mando_por_mil)
{
    SalidaEmulada *salida = contexto;
    const bool aceptada = Escribir(contexto, mando_por_mil);
    if (salida != NULL) {
        salida->ruta = 2U;
    }
    return aceptada;
}

static Driver_Motor_Salida Crear(SalidaEmulada *emulada)
{
    const Driver_Motor_Salida salida = {Escribir, emulada};
    return salida;
}

static bool RangoCompleto(void)
{
    SalidaEmulada emulada = {0U, 777U, true, 0U};
    const Driver_Motor_Salida salida = Crear(&emulada);
    CHECK(DRIVER_MOTOR_MANDO_MAX == 1000U && TPI_MANDO_MANUAL_MAX == 1000U);
    for (uint32_t valor = 0U; valor <= UINT16_MAX; ++valor) {
        const unsigned anteriores = emulada.llamadas;
        const uint16_t previo = emulada.ultimo_solicitado;
        const Driver_Motor_Estado estado = Driver_Motor_Aplicar(&salida, (uint16_t)valor);
        if (valor <= 1000U) {
            CHECK(estado == DRIVER_MOTOR_ACEPTADA);
            CHECK(emulada.llamadas == anteriores + 1U);
            CHECK(emulada.ultimo_solicitado == valor && emulada.ruta == 1U);
        } else {
            CHECK(estado == DRIVER_MOTOR_ERROR_RANGO);
            CHECK(emulada.llamadas == anteriores && emulada.ultimo_solicitado == previo);
        }
    }
    CHECK(emulada.llamadas == 1001U && emulada.ultimo_solicitado == 1000U);
    return true;
}

static bool SolicitudDeCero(void)
{
    SalidaEmulada emulada = {0U, 0U, true, 0U};
    const Driver_Motor_Salida salida = Crear(&emulada);
    CHECK(Driver_Motor_Aplicar(&salida, 750U) == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 1U && emulada.ultimo_solicitado == 750U);
    CHECK(Driver_Motor_SolicitarCero(&salida) == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 2U && emulada.ultimo_solicitado == 0U);
    CHECK(Driver_Motor_SolicitarCero(&salida) == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 3U && emulada.ultimo_solicitado == 0U);
    return true;
}

static bool RepeticionesSinCache(void)
{
    SalidaEmulada emulada = {0U, 0U, true, 0U};
    const Driver_Motor_Salida salida = Crear(&emulada);
    const uint16_t pedidos[] = {500U, 500U, 1000U, 1000U, 0U, 0U};
    for (size_t i = 0U; i < sizeof(pedidos) / sizeof(pedidos[0]); ++i) {
        CHECK(Driver_Motor_Aplicar(&salida, pedidos[i]) == DRIVER_MOTOR_ACEPTADA);
        CHECK(emulada.llamadas == i + 1U && emulada.ultimo_solicitado == pedidos[i]);
    }
    return true;
}

static bool InterfazInvalidaYPrioridad(void)
{
    SalidaEmulada emulada = {0U, 777U, true, 0U};
    const Driver_Motor_Salida sin_funcion = {NULL, &emulada};
    const Driver_Motor_Salida vacia = {NULL, NULL};
    const uint16_t valores[] = {0U, 1000U, 1001U, UINT16_MAX};
    for (size_t i = 0U; i < sizeof(valores) / sizeof(valores[0]); ++i) {
        CHECK(Driver_Motor_Aplicar(NULL, valores[i]) == DRIVER_MOTOR_ERROR_ARGUMENTO);
        CHECK(Driver_Motor_Aplicar(&sin_funcion, valores[i]) == DRIVER_MOTOR_ERROR_ARGUMENTO);
        CHECK(Driver_Motor_Aplicar(&vacia, valores[i]) == DRIVER_MOTOR_ERROR_ARGUMENTO);
    }
    CHECK(Driver_Motor_SolicitarCero(NULL) == DRIVER_MOTOR_ERROR_ARGUMENTO);
    CHECK(Driver_Motor_SolicitarCero(&sin_funcion) == DRIVER_MOTOR_ERROR_ARGUMENTO);
    CHECK(Driver_Motor_SolicitarCero(&vacia) == DRIVER_MOTOR_ERROR_ARGUMENTO);
    CHECK(emulada.llamadas == 0U && emulada.ultimo_solicitado == 777U);
    return true;
}

static bool ErrorSalidaYRecuperacion(void)
{
    SalidaEmulada emulada = {0U, 0U, true, 0U};
    const Driver_Motor_Salida salida = Crear(&emulada);
    CHECK(Driver_Motor_Aplicar(&salida, 500U) == DRIVER_MOTOR_ACEPTADA);
    emulada.acepta = false;
    CHECK(Driver_Motor_Aplicar(&salida, 700U) == DRIVER_MOTOR_ERROR_SALIDA);
    CHECK(emulada.llamadas == 2U && emulada.ultimo_solicitado == 700U);
    emulada.acepta = true;
    CHECK(Driver_Motor_Aplicar(&salida, 700U) == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 3U && emulada.ultimo_solicitado == 700U);
    return true;
}

static bool ErrorCeroYRecuperacion(void)
{
    SalidaEmulada emulada = {0U, 700U, false, 0U};
    const Driver_Motor_Salida salida = Crear(&emulada);
    CHECK(Driver_Motor_SolicitarCero(&salida) == DRIVER_MOTOR_ERROR_SALIDA);
    CHECK(emulada.llamadas == 1U && emulada.ultimo_solicitado == 0U);
    emulada.acepta = true;
    CHECK(Driver_Motor_SolicitarCero(&salida) == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 2U && emulada.ultimo_solicitado == 0U);
    CHECK(Driver_Motor_Aplicar(&salida, 200U) == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 3U && emulada.ultimo_solicitado == 200U);
    return true;
}

static bool SinContexto(void *contexto, uint16_t mando_por_mil)
{
    return contexto == NULL && mando_por_mil <= 1000U;
}

static bool ContextoNuloPermitido(void)
{
    const Driver_Motor_Salida salida = {SinContexto, NULL};
    CHECK(Driver_Motor_Aplicar(&salida, 500U) == DRIVER_MOTOR_ACEPTADA);
    CHECK(Driver_Motor_Aplicar(&salida, 1000U) == DRIVER_MOTOR_ACEPTADA);
    CHECK(Driver_Motor_SolicitarCero(&salida) == DRIVER_MOTOR_ACEPTADA);
    return true;
}

static bool InstanciasIndependientes(void)
{
    SalidaEmulada ea = {0U, 0U, false, 0U}, eb = {0U, 0U, true, 0U};
    const Driver_Motor_Salida a = Crear(&ea), b = {EscribirSegunda, &eb};
    CHECK(Driver_Motor_Aplicar(&a, 250U) == DRIVER_MOTOR_ERROR_SALIDA);
    CHECK(eb.llamadas == 0U && eb.ultimo_solicitado == 0U && eb.ruta == 0U);
    CHECK(Driver_Motor_Aplicar(&b, 750U) == DRIVER_MOTOR_ACEPTADA);
    CHECK(ea.llamadas == 1U && ea.ultimo_solicitado == 250U && ea.ruta == 1U);
    CHECK(eb.llamadas == 1U && eb.ultimo_solicitado == 750U && eb.ruta == 2U);
    CHECK(Driver_Motor_SolicitarCero(&b) == DRIVER_MOTOR_ACEPTADA);
    ea.acepta = true;
    CHECK(Driver_Motor_Aplicar(&a, 333U) == DRIVER_MOTOR_ACEPTADA);
    CHECK(eb.llamadas == 2U && eb.ultimo_solicitado == 0U && eb.ruta == 2U);
    CHECK(ea.llamadas == 2U && ea.ultimo_solicitado == 333U && ea.ruta == 1U);
    return true;
}

static bool InterfazInmutable(void)
{
    SalidaEmulada emulada = {0U, 0U, true, 0U};
    const Driver_Motor_Salida salida = Crear(&emulada);
    const Driver_Motor_Salida copia = salida;
    CHECK(Driver_Motor_Aplicar(&salida, 631U) == DRIVER_MOTOR_ACEPTADA);
    CHECK(Driver_Motor_SolicitarCero(&salida) == DRIVER_MOTOR_ACEPTADA);
    CHECK(salida.escribir == copia.escribir && salida.contexto == copia.contexto);
    CHECK(emulada.llamadas == 2U && emulada.ultimo_solicitado == 0U);
    return true;
}

static bool RechazoNoEsParada(void)
{
    SalidaEmulada emulada = {0U, 0U, true, 0U};
    const Driver_Motor_Salida salida = Crear(&emulada);
    CHECK(Driver_Motor_Aplicar(&salida, 700U) == DRIVER_MOTOR_ACEPTADA);
    CHECK(Driver_Motor_Aplicar(&salida, 1001U) == DRIVER_MOTOR_ERROR_RANGO);
    CHECK(Driver_Motor_Aplicar(&salida, UINT16_MAX) == DRIVER_MOTOR_ERROR_RANGO);
    CHECK(emulada.llamadas == 1U && emulada.ultimo_solicitado == 700U);
    CHECK(Driver_Motor_Aplicar(&salida, 250U) == DRIVER_MOTOR_ACEPTADA);
    CHECK(emulada.llamadas == 2U && emulada.ultimo_solicitado == 250U);
    return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S1/S3 dominio uint16 completo", RangoCompleto},
        {"S2 cero explicito", SolicitudDeCero}, {"S1 repeticion", RepeticionesSinCache},
        {"S3 interfaz/prioridad", InterfazInvalidaYPrioridad},
        {"S4 fallo/recuperacion mando", ErrorSalidaYRecuperacion},
        {"S2/S4 fallo/recuperacion cero", ErrorCeroYRecuperacion},
        {"S5 contexto NULL", ContextoNuloPermitido}, {"S5 instancias", InstanciasIndependientes},
        {"S5 interfaz const", InterfazInmutable}, {"S3 rechazo no es parada", RechazoNoEsParada}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre); return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));
    return 0;
}
