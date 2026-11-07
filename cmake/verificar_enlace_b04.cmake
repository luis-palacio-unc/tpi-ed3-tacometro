# Retener API Mandos/B01 y auxiliares en ARM; no ejecutar codigo ARM.
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
#include "Mandos.h"
#include "Procesamiento.h"
#include <stddef.h>
static Mandos_MandoManual mando;
static Procesamiento_Contexto contexto;
static volatile uint32_t lectura = 2048U;
static volatile uint32_t escala = 4095U;
int main(void)
{
    if (!Mandos_Inicializar(&mando) ||
        !Procesamiento_Inicializar(&contexto, PROCESAMIENTO_MODO_MANUAL, 0U)) {
        return 1;
    }
    if (Mandos_NormalizarManual(&mando, lectura, escala) != MANDOS_VALIDO) {
        return 1;
    }
    const Procesamiento_Entradas entradas = {PROCESAMIENTO_MODO_MANUAL, mando.por_mil, 1U, NULL};
    Procesamiento_Salida salida;
    if (!Procesamiento_Actualizar(&contexto, &entradas, &salida)) {
        return 1;
    }
    return salida.fuente == PROCESAMIENTO_FUENTE_MANUAL && salida.mando_manual_por_mil == 500U ? 0 : 1;
}
]=])
execute_process(COMMAND "${TOOL_PREFIX}gcc"
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 ${TPI_FIXTURE_OPT}
    -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack
    -fno-unwind-tables -fno-asynchronous-unwind-tables
    -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors
    "-I${SOURCE_DIR}/Servicios" "-I${SOURCE_DIR}/Aplicacion" "-I${SOURCE_DIR}/Configuracion"
    "${OUTPUT_DIR}/main_fixture.c" "${SOURCE_DIR}/Servicios/Mandos.c"
    "${SOURCE_DIR}/Aplicacion/Procesamiento.c" "${SOURCE_DIR}/plataforma/startup_lpc1769.S"
    -nostdlib "-T${SOURCE_DIR}/plataforma/lpc1769.ld"
    -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings -Wl,-z,noexecstack
    "-Wl,-Map=${OUTPUT_DIR}/fixture.map" -lgcc -o "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fallo de enlace API Mandos/B01: ${output} ${errors}")
endif()
execute_process(COMMAND "${PYTHON}" "${SOURCE_DIR}/scripts/verificar_elf.py"
    "${OUTPUT_DIR}/fixture.elf" --tool-prefix "${TOOL_PREFIX}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fixture ARM B04 invalido: ${output} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}nm" --defined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "No se pudieron inspeccionar simbolos B04: ${errors}")
endif()
foreach(required_symbol Mandos_Inicializar Mandos_NormalizarManual Procesamiento_Inicializar Procesamiento_Actualizar __aeabi_uldivmod)
    if(NOT symbols MATCHES "[ \t]${required_symbol}[\r\n]")
        message(FATAL_ERROR "API/auxiliar B04 eliminado o ausente: ${required_symbol}")
    endif()
endforeach()
execute_process(COMMAND "${TOOL_PREFIX}readelf" -lW "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE headers ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT headers MATCHES "GNU_STACK[^\r\n]*[ \t]RW[ \t]+0x")
    message(FATAL_ERROR "B04 requiere GNU_STACK RW, sin ejecucion: ${headers} ${errors}")
endif()
message(STATUS "API Mandos/B01 enlazada con libgcc, sin libc; no ejecutada en MCU. ${output}")
