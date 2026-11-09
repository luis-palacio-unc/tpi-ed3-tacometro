# Retener evaluacion B06, API B01 y servicio B05 en ELF ARM, sin ejecutar firmware.
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
#include "Procesamiento.h"
static Procesamiento_Contexto contexto;
static Temperatura_Lectura lectura;
static volatile int32_t temperatura = 2534;
static volatile int32_t umbral = 2500;
static volatile Procesamiento_EstadoAlarma decision;
int main(void)
{
    Procesamiento_Entradas entradas = {PROCESAMIENTO_MODO_MANUAL, 375U, 1U, 0};
    Procesamiento_Salida salida;
    if (!Procesamiento_Inicializar(&contexto, PROCESAMIENTO_MODO_MANUAL, 0U) ||
        !Procesamiento_Actualizar(&contexto, &entradas, &salida) ||
        !Temperatura_Inicializar(&lectura)) {
        return 1;
    }
    if (Temperatura_Actualizar(&lectura, temperatura, TEMPERATURA_FUENTE_VALIDA) != TEMPERATURA_VALIDA) {
        return 1;
    }
    decision = Procesamiento_EvaluarAlarmaTemperatura(&lectura, umbral);
    return decision == PROCESAMIENTO_ALARMA_ACTIVA &&
           salida.fuente == PROCESAMIENTO_FUENTE_MANUAL && salida.mando_manual_por_mil == 375U ? 0 : 1;
}
]=])
execute_process(COMMAND "${TOOL_PREFIX}gcc"
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 ${TPI_FIXTURE_OPT}
    -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack
    -fno-unwind-tables -fno-asynchronous-unwind-tables
    -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors
    "-I${SOURCE_DIR}/Aplicacion" "-I${SOURCE_DIR}/Servicios" "-I${SOURCE_DIR}/Configuracion"
    "${OUTPUT_DIR}/main_fixture.c" "${SOURCE_DIR}/Aplicacion/Procesamiento.c"
    "${SOURCE_DIR}/Servicios/Temperatura.c" "${SOURCE_DIR}/plataforma/startup_lpc1769.S"
    -nostdlib "-T${SOURCE_DIR}/plataforma/lpc1769.ld"
    -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings -Wl,-z,noexecstack
    "-Wl,-Map=${OUTPUT_DIR}/fixture.map" -o "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fallo de enlace API B06/B01/Temperatura: ${output} ${errors}")
endif()
execute_process(COMMAND "${PYTHON}" "${SOURCE_DIR}/scripts/verificar_elf.py"
    "${OUTPUT_DIR}/fixture.elf" --tool-prefix "${TOOL_PREFIX}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fixture B06 invalido: ${output} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}nm" --defined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "No se pudieron inspeccionar simbolos B06: ${errors}")
endif()
foreach(required_symbol Procesamiento_EvaluarAlarmaTemperatura Procesamiento_Inicializar
        Procesamiento_Actualizar Temperatura_Inicializar Temperatura_Actualizar)
    if(NOT symbols MATCHES "[ \t]${required_symbol}[\r\n]")
        message(FATAL_ERROR "API B06/B01/B05 ausente: ${required_symbol}")
    endif()
endforeach()
execute_process(COMMAND "${TOOL_PREFIX}nm" --undefined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE undefined_symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT undefined_symbols STREQUAL "")
    message(FATAL_ERROR "B06 tiene simbolos indefinidos: ${undefined_symbols} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}readelf" -lW "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE headers ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT headers MATCHES "GNU_STACK[^\r\n]*[ \t]RW[ \t]+0x")
    message(FATAL_ERROR "B06 requiere GNU_STACK RW: ${headers} ${errors}")
endif()
message(STATUS "B06/B01/Temperatura enlazados sin dispositivos/libc; no ejecutados en MCU. ${output}")
