# Enlace de la API B01 fuera del main de produccion, sin ejecutar codigo ARM.
foreach(required SOURCE_DIR OUTPUT_DIR TOOL_PREFIX PYTHON)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Falta parametro ${required}")
    endif()
endforeach()
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
file(WRITE "${OUTPUT_DIR}/main_fixture.c" [=[
#include "Procesamiento.h"
static Procesamiento_Contexto contexto;
int main(void)
{
    Procesamiento_Entradas entradas;
    Procesamiento_Salida salida;
    if (!Procesamiento_Inicializar(&contexto, PROCESAMIENTO_MODO_AUTOMATICO, 0U)) {
        return 1;
    }
    entradas.modo = PROCESAMIENTO_MODO_AUTOMATICO;
    entradas.mando_manual_por_mil = 0U;
    entradas.orden_actual = 1U;
    entradas.consigna = 0;
    return Procesamiento_Actualizar(&contexto, &entradas, &salida) ? (int)salida.fuente : 1;
}
]=])
execute_process(COMMAND "${TOOL_PREFIX}gcc"
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 -O0
    -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack
    -fno-unwind-tables -fno-asynchronous-unwind-tables
    -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors
    "-I${SOURCE_DIR}/Aplicacion" "${OUTPUT_DIR}/main_fixture.c"
    "${SOURCE_DIR}/Aplicacion/Procesamiento.c" "${SOURCE_DIR}/plataforma/startup_lpc1769.S"
    -nostdlib "-T${SOURCE_DIR}/plataforma/lpc1769.ld"
    -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings
    "-Wl,-Map=${OUTPUT_DIR}/fixture.map" -o "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fallo de enlace API B01: ${output} ${errors}")
endif()
execute_process(COMMAND "${PYTHON}" "${SOURCE_DIR}/scripts/verificar_elf.py"
    "${OUTPUT_DIR}/fixture.elf" --tool-prefix "${TOOL_PREFIX}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fixture ARM B01 invalido: ${output} ${errors}")
endif()
message(STATUS "API B01 enlazada para ARM sin libc; no ejecutada en MCU. ${output}")
