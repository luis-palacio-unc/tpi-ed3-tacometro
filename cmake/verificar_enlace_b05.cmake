# Retener API Temperatura y auxiliares en ARM; no ejecutar codigo ARM.
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
#include "Temperatura.h"
static Temperatura_Lectura lectura;
static volatile int32_t centesimas = -725;
static volatile Temperatura_EstadoFuente fuente = TEMPERATURA_FUENTE_VALIDA;
int main(void)
{
    if (!Temperatura_Inicializar(&lectura)) {
        return 1;
    }
    return Temperatura_Actualizar(&lectura, centesimas, fuente) == TEMPERATURA_VALIDA &&
           lectura.centesimas_c == -725 ? 0 : 1;
}
]=])
execute_process(COMMAND "${TOOL_PREFIX}gcc"
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 ${TPI_FIXTURE_OPT}
    -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack
    -fno-unwind-tables -fno-asynchronous-unwind-tables
    -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors
    "-I${SOURCE_DIR}/Servicios" "${OUTPUT_DIR}/main_fixture.c"
    "${SOURCE_DIR}/Servicios/Temperatura.c" "${SOURCE_DIR}/plataforma/startup_lpc1769.S"
    -nostdlib "-T${SOURCE_DIR}/plataforma/lpc1769.ld"
    -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings -Wl,-z,noexecstack
    "-Wl,-Map=${OUTPUT_DIR}/fixture.map" -o "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fallo de enlace API Temperatura: ${output} ${errors}")
endif()
execute_process(COMMAND "${PYTHON}" "${SOURCE_DIR}/scripts/verificar_elf.py"
    "${OUTPUT_DIR}/fixture.elf" --tool-prefix "${TOOL_PREFIX}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fixture ARM B05 invalido: ${output} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}nm" --defined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "No se pudieron inspeccionar simbolos B05: ${errors}")
endif()
foreach(required_symbol Temperatura_Inicializar Temperatura_Actualizar)
    if(NOT symbols MATCHES "[ \t]${required_symbol}[\r\n]")
        message(FATAL_ERROR "API/auxiliar B05 eliminado o ausente: ${required_symbol}")
    endif()
endforeach()
execute_process(COMMAND "${TOOL_PREFIX}readelf" -lW "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE headers ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT headers MATCHES "GNU_STACK[^\r\n]*[ \t]RW[ \t]+0x")
    message(FATAL_ERROR "B05 requiere GNU_STACK RW, sin ejecucion: ${headers} ${errors}")
endif()
message(STATUS "Servicio Temperatura enlazado sin dispositivo concreto, libgcc ni libc; no ejecutada en MCU. ${output}")
