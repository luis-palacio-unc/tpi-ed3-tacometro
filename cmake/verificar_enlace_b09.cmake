# Retener B09 y composicion B01/B03/B05/RX sin UART; no ejecutar ARM.
foreach(required SOURCE_DIR OUTPUT_DIR TOOL_PREFIX PYTHON)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Falta parametro ${required}")
    endif()
endforeach()
if(BUILD_TYPE STREQUAL "Release")
    set(TPI_FIXTURE_OPT -O3)
else()
    set(TPI_FIXTURE_OPT -O0)
endif()
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
file(WRITE "${OUTPUT_DIR}/main_fixture.c" [=[
#include "Comunicacion.h"
#include "Medicion_Velocidad.h"
#include "Temperatura.h"
#include "Procesamiento.h"
static Comunicacion_Contexto rx;
static Procesamiento_Contexto app;
static Medicion_Velocidad_Lectura velocidad;
static Temperatura_Lectura temperatura;
static volatile uint32_t pulsos = 123U;
static volatile int32_t centesimas = 2534;
static char buffer[COMUNICACION_TELEMETRIA_CAPACIDAD];
static size_t longitud;
static volatile Comunicacion_ResultadoTelemetria resultado;
int main(void)
{
    if (!Comunicacion_Inicializar(&rx, 0U) ||
        !Procesamiento_Inicializar(&app, PROCESAMIENTO_MODO_MANUAL, 0U) ||
        !Medicion_Velocidad_Inicializar(&velocidad) || !Temperatura_Inicializar(&temperatura)) {
        return 1;
    }
    const char comando[] = "RPM 0\n";
    uint64_t orden = 0U;
    for (size_t i = 0U; i < sizeof(comando) - 1U; ++i) {
        (void)Comunicacion_RecibirByte(&rx, (uint8_t)comando[i], ++orden);
    }
    Comunicacion_Consigna evento;
    if (Comunicacion_ExtraerConsigna(&rx, &evento) != COMUNICACION_EXTRAIDA ||
        Medicion_Velocidad_Actualizar(&velocidad, pulsos, 1000000U, 60U) != MEDICION_VELOCIDAD_VALIDA ||
        Temperatura_Actualizar(&temperatura, centesimas, TEMPERATURA_FUENTE_VALIDA) != TEMPERATURA_VALIDA) {
        return 1;
    }
    const Procesamiento_Entradas entradas = {PROCESAMIENTO_MODO_MANUAL, 500U, orden, 0};
    Procesamiento_Salida seleccion;
    if (!Procesamiento_Actualizar(&app, &entradas, &seleccion) || seleccion.estado != PROCESAMIENTO_MANUAL) {
        return 1;
    }
    const Comunicacion_Telemetria datos = {velocidad.rpm_milesimas, temperatura.centesimas_c,
        COMUNICACION_DATO_VALIDO, COMUNICACION_DATO_VALIDO, COMUNICACION_OPERACION_MANUAL};
    resultado = Comunicacion_CodificarTelemetria(&datos, buffer, sizeof(buffer), &longitud);
    return resultado == COMUNICACION_TELEMETRIA_OK && longitud > 0U && buffer[0] == 'R' &&
           buffer[longitud - 1U] == '\n' && buffer[longitud] == '\0' && evento.rpm == 0U ? 0 : 1;
}
]=])
execute_process(COMMAND "${TOOL_PREFIX}gcc"
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 ${TPI_FIXTURE_OPT}
    -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack
    -fno-unwind-tables -fno-asynchronous-unwind-tables
    -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors
    "-I${SOURCE_DIR}/Servicios" "-I${SOURCE_DIR}/Aplicacion" "-I${SOURCE_DIR}/Configuracion"
    "${OUTPUT_DIR}/main_fixture.c" "${SOURCE_DIR}/Servicios/Comunicacion.c"
    "${SOURCE_DIR}/Aplicacion/Procesamiento.c" "${SOURCE_DIR}/Servicios/Medicion_Velocidad.c"
    "${SOURCE_DIR}/Servicios/Temperatura.c"
    "${SOURCE_DIR}/plataforma/startup_lpc1769.S"
    -nostdlib "-T${SOURCE_DIR}/plataforma/lpc1769.ld"
    -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings -Wl,-z,noexecstack
    "-Wl,-Map=${OUTPUT_DIR}/fixture.map" -lgcc -o "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fallo de enlace B09/B01/B03/B05: ${output} ${errors}")
endif()
execute_process(COMMAND "${PYTHON}" "${SOURCE_DIR}/scripts/verificar_elf.py"
    "${OUTPUT_DIR}/fixture.elf" --tool-prefix "${TOOL_PREFIX}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fixture B09 invalido: ${output} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}nm" --defined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "No se pudieron inspeccionar simbolos B09: ${errors}")
endif()
foreach(required_symbol Comunicacion_CodificarTelemetria Comunicacion_Inicializar
        Comunicacion_RecibirByte Comunicacion_ExtraerConsigna Procesamiento_Inicializar
        Procesamiento_Actualizar Medicion_Velocidad_Inicializar Medicion_Velocidad_Actualizar
        Temperatura_Inicializar Temperatura_Actualizar)
    if(NOT symbols MATCHES "[ \t]${required_symbol}[\r\n]")
        message(FATAL_ERROR "API/proveedor B09 ausente: ${required_symbol}")
    endif()
endforeach()
execute_process(COMMAND "${TOOL_PREFIX}nm" --undefined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE undefined_symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT undefined_symbols STREQUAL "")
    message(FATAL_ERROR "B09 tiene simbolos indefinidos: ${undefined_symbols} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}readelf" -lW "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE headers ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT headers MATCHES "GNU_STACK[^\r\n]*[ \t]RW[ \t]+0x")
    message(FATAL_ERROR "B09 requiere GNU_STACK RW: ${headers} ${errors}")
endif()
message(STATUS "B09/B01/B03/B05 y telemetria logica enlazados sin libc/dispositivos; no ejecutados en MCU. ${output}")
