#include "Comunicacion.h"
#include "Medicion_Velocidad.h"
#include "Temperatura.h"
#include "Procesamiento.h"
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); return false; \
} } while (0)

/* Composicion de prueba; no acoplar Comunicacion a tipos de Aplicacion. */
static Comunicacion_Telemetria Adaptar(const Medicion_Velocidad_Lectura *velocidad,
    const Temperatura_Lectura *temperatura, Procesamiento_Estado operacion)
{
    Comunicacion_Telemetria datos = {0U, 0, COMUNICACION_DATO_ERROR, COMUNICACION_DATO_ERROR,
        (Comunicacion_EstadoOperacion)99};
    if (velocidad->estado == MEDICION_VELOCIDAD_VALIDA) {
        datos.rpm_milesimas = velocidad->rpm_milesimas;datos.validez_rpm = COMUNICACION_DATO_VALIDO;
    } else if (velocidad->estado == MEDICION_VELOCIDAD_SIN_DATOS) {
        datos.validez_rpm = COMUNICACION_DATO_SIN_DATOS;
    }
    if (temperatura->estado == TEMPERATURA_VALIDA) {
        datos.temperatura_centesimas_c = temperatura->centesimas_c;datos.validez_temperatura = COMUNICACION_DATO_VALIDO;
    } else if (temperatura->estado == TEMPERATURA_SIN_DATOS) {
        datos.validez_temperatura = COMUNICACION_DATO_SIN_DATOS;
    }
    switch (operacion) {
        case PROCESAMIENTO_MANUAL: datos.estado_operacion = COMUNICACION_OPERACION_MANUAL;break;
        case PROCESAMIENTO_AUTO_ESPERA_CONSIGNA: datos.estado_operacion = COMUNICACION_OPERACION_AUTO_ESPERA;break;
        case PROCESAMIENTO_AUTO_ACTIVO: datos.estado_operacion = COMUNICACION_OPERACION_AUTO_ACTIVO;break;
        default: break;
    }
    return datos;
}

static bool Esperar(const Comunicacion_Telemetria *datos, const char *texto)
{
    char buffer[COMUNICACION_TELEMETRIA_CAPACIDAD];size_t longitud = 0U;
    CHECK(Comunicacion_CodificarTelemetria(datos, buffer, sizeof(buffer), &longitud) == COMUNICACION_TELEMETRIA_OK);
    CHECK(strcmp(buffer, texto) == 0 && longitud == strlen(texto));return true;
}

static Comunicacion_Resultado Recibir(Comunicacion_Contexto *rx, const char *texto, uint64_t *orden)
{
    Comunicacion_Resultado resultado = COMUNICACION_OK;
    while (*texto != '\0') { resultado = Comunicacion_RecibirByte(rx, (uint8_t)*texto, ++*orden);++texto; }
    return resultado;
}

static bool ManualConServicios(void)
{
    Medicion_Velocidad_Lectura velocidad;Temperatura_Lectura temperatura;Procesamiento_Contexto contexto;
    CHECK(Medicion_Velocidad_Actualizar(&velocidad, 123U, 1000000U, 60U) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(Temperatura_Actualizar(&temperatura, 2534, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Procesamiento_Inicializar(&contexto, PROCESAMIENTO_MODO_MANUAL, 0U));
    const Procesamiento_Entradas entradas = {PROCESAMIENTO_MODO_MANUAL, 500U, 1U, NULL};Procesamiento_Salida salida;
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &salida));
    const Comunicacion_Telemetria datos = Adaptar(&velocidad, &temperatura, salida.estado);
    CHECK(Esperar(&datos, "RPM=123.000;TEMP=25.34;ESTADO=MANUAL\n"));
    CHECK(salida.fuente == PROCESAMIENTO_FUENTE_MANUAL && salida.mando_manual_por_mil == 500U);
    CHECK(velocidad.rpm_milesimas == 123000U && temperatura.centesimas_c == 2534);return true;
}

static bool TransicionesAutomaticas(void)
{
    Medicion_Velocidad_Lectura velocidad;Temperatura_Lectura temperatura;Procesamiento_Contexto contexto;Comunicacion_Contexto rx;
    CHECK(Medicion_Velocidad_Actualizar(&velocidad, 300U, 1000000U, 60U) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(Temperatura_Actualizar(&temperatura, 0, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    CHECK(Procesamiento_Inicializar(&contexto, PROCESAMIENTO_MODO_MANUAL, 0U));CHECK(Comunicacion_Inicializar(&rx, 0U));
    Procesamiento_Entradas entradas = {PROCESAMIENTO_MODO_AUTOMATICO, 0U, 1U, NULL};Procesamiento_Salida salida;
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &salida));
    Comunicacion_Telemetria datos = Adaptar(&velocidad, &temperatura, salida.estado);
    CHECK(Esperar(&datos, "RPM=300.000;TEMP=0.00;ESTADO=AUTO_ESPERA\n"));
    uint64_t orden = 1U;CHECK(Recibir(&rx, "RPM 185\n", &orden) == COMUNICACION_ENCOLADA);
    Comunicacion_Consigna evento;CHECK(Comunicacion_ExtraerConsigna(&rx, &evento) == COMUNICACION_EXTRAIDA);
    const Procesamiento_Consigna consigna = {evento.rpm, evento.orden_recepcion, true};entradas.consigna = &consigna;entradas.orden_actual = ++orden;
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &salida));datos = Adaptar(&velocidad, &temperatura, salida.estado);
    CHECK(Esperar(&datos, "RPM=300.000;TEMP=0.00;ESTADO=AUTO_ACTIVO\n"));
    CHECK(salida.consigna_rpm == 185U && velocidad.rpm_milesimas == 300000U);
    entradas.modo = PROCESAMIENTO_MODO_MANUAL;entradas.mando_manual_por_mil = 1000U;entradas.orden_actual = ++orden;
    CHECK(Procesamiento_Actualizar(&contexto, &entradas, &salida));datos = Adaptar(&velocidad, &temperatura, salida.estado);
    CHECK(Esperar(&datos, "RPM=300.000;TEMP=0.00;ESTADO=MANUAL\n"));return true;
}

static bool AusenciaErrorYCero(void)
{
    Medicion_Velocidad_Lectura velocidad;Temperatura_Lectura temperatura;
    CHECK(Medicion_Velocidad_Inicializar(&velocidad));CHECK(Temperatura_Inicializar(&temperatura));
    Comunicacion_Telemetria datos = Adaptar(&velocidad, &temperatura, PROCESAMIENTO_MANUAL);
    CHECK(Esperar(&datos, "RPM=ND;TEMP=ND;ESTADO=MANUAL\n"));
    CHECK(Medicion_Velocidad_Actualizar(&velocidad, 1U, 0U, 60U) == MEDICION_VELOCIDAD_ERROR_DURACION);
    CHECK(Temperatura_Actualizar(&temperatura, 2534, TEMPERATURA_FUENTE_ERROR) == TEMPERATURA_ERROR_FUENTE);
    datos = Adaptar(&velocidad, &temperatura, PROCESAMIENTO_MANUAL);CHECK(Esperar(&datos, "RPM=ERR;TEMP=ERR;ESTADO=MANUAL\n"));
    CHECK(Medicion_Velocidad_Actualizar(&velocidad, 0U, 1000000U, 60U) == MEDICION_VELOCIDAD_VALIDA);
    CHECK(Temperatura_Actualizar(&temperatura, 0, TEMPERATURA_FUENTE_VALIDA) == TEMPERATURA_VALIDA);
    datos = Adaptar(&velocidad, &temperatura, PROCESAMIENTO_MANUAL);CHECK(Esperar(&datos, "RPM=0.000;TEMP=0.00;ESTADO=MANUAL\n"));return true;
}

static bool RecepcionIntercalada(void)
{
    Comunicacion_Contexto rx;uint64_t orden = 0U;CHECK(Comunicacion_Inicializar(&rx, 0U));
    CHECK(Recibir(&rx, "RPM 1\n", &orden) == COMUNICACION_ENCOLADA);
    CHECK(Recibir(&rx, "RPM 1", &orden) == COMUNICACION_OK);
    const Comunicacion_Telemetria datos = {0U, 0, COMUNICACION_DATO_SIN_DATOS, COMUNICACION_DATO_ERROR, COMUNICACION_OPERACION_MANUAL};
    char buffer[54];size_t longitud = 999U;
    CHECK(Comunicacion_CodificarTelemetria(&datos, buffer, 1U, &longitud) == COMUNICACION_TELEMETRIA_ERROR_CAPACIDAD);
    CHECK(longitud == 0U && buffer[0] == '\0');CHECK(Esperar(&datos, "RPM=ND;TEMP=ERR;ESTADO=MANUAL\n"));
    CHECK(rx.ultimo_orden == 11U && rx.longitud == 5U && rx.cantidad == 1U);
    CHECK(Recibir(&rx, "85\n", &orden) == COMUNICACION_ENCOLADA);
    Comunicacion_Consigna evento;CHECK(Comunicacion_ExtraerConsigna(&rx, &evento) == COMUNICACION_EXTRAIDA);
    CHECK(evento.rpm == 1U && evento.orden_recepcion == 6U);
    CHECK(Comunicacion_ExtraerConsigna(&rx, &evento) == COMUNICACION_EXTRAIDA);
    CHECK(evento.rpm == 185U && evento.orden_recepcion == 14U);
    CHECK(Comunicacion_ExtraerConsigna(&rx, &evento) == COMUNICACION_VACIA);return true;
}

int main(void)
{
    const struct { const char *nombre; bool (*ejecutar)(void); } casos[] = {
        {"S7 manual B01/B03/B05", ManualConServicios}, {"S7 transiciones automaticas", TransicionesAutomaticas},
        {"S3/S7 ND/ERR/cero", AusenciaErrorYCero}, {"S7 recepcion intercalada", RecepcionIntercalada}
    };
    for (size_t i = 0U; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        if (!casos[i].ejecutar()) { fprintf(stderr, "FAIL %s\n", casos[i].nombre);return 1; }
        printf("PASS %s\n", casos[i].nombre);
    }
    printf("%zu/%zu casos correctos\n", sizeof(casos) / sizeof(casos[0]), sizeof(casos) / sizeof(casos[0]));return 0;
}
