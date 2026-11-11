# Retener el regulador PI de B17 en ARM, sin servicios ni dispositivos; no ejecutar ARM.
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
#include "Control.h"
static Procesamiento_Regulador pi;
static volatile uint32_t rpm_milesimas;
static volatile uint16_t ultimo;
int main(void)
{
    const Procesamiento_ConfigRegulador cfg={TPI_CONTROL_KP_NUM,TPI_CONTROL_KP_DEN,
        TPI_CONTROL_KI_NUM,TPI_CONTROL_KI_DEN,TPI_CONTROL_MAX_PERIODOS_SIN_MEDICION};
    if (!Procesamiento_InicializarRegulador(&pi,&cfg) ||
        !Procesamiento_ReiniciarRegulador(&pi)) { return 1; }
    const Medicion_Velocidad_Lectura medicion={rpm_milesimas,MEDICION_VELOCIDAD_VALIDA};
    uint16_t mando;
    if (Procesamiento_Regular(&pi,100U,&medicion,&mando)!=PROCESAMIENTO_REGULADOR_VALIDO) { return 1; }
    ultimo=mando;
    return 0;
}
]=])
execute_process(COMMAND "${TOOL_PREFIX}gcc"
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 ${TPI_FIXTURE_OPT}
    -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack
    -fno-unwind-tables -fno-asynchronous-unwind-tables
    -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors
    "-I${SOURCE_DIR}/Servicios" "-I${SOURCE_DIR}/Aplicacion" "-I${SOURCE_DIR}/Configuracion"
    "${OUTPUT_DIR}/main_fixture.c" "${SOURCE_DIR}/Aplicacion/Procesamiento.c"
    "${SOURCE_DIR}/plataforma/startup_lpc1769.S"
    -nostdlib "-T${SOURCE_DIR}/plataforma/lpc1769.ld"
    -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings -Wl,-z,noexecstack
    "-Wl,-Map=${OUTPUT_DIR}/fixture.map" -lgcc -o "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fallo de enlace B17: ${output} ${errors}")
endif()
execute_process(COMMAND "${PYTHON}" "${SOURCE_DIR}/scripts/verificar_elf.py"
    "${OUTPUT_DIR}/fixture.elf" --tool-prefix "${TOOL_PREFIX}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fixture B17 invalido: ${output} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}nm" --defined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "No se pudieron inspeccionar simbolos B17: ${errors}")
endif()
foreach(required_symbol Procesamiento_InicializarRegulador Procesamiento_Regular
        Procesamiento_ReiniciarRegulador)
    if(NOT symbols MATCHES "[ \t]${required_symbol}[\r\n]")
        message(FATAL_ERROR "API B17 ausente: ${required_symbol}")
    endif()
endforeach()
execute_process(COMMAND "${TOOL_PREFIX}nm" --undefined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE undefined_symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT undefined_symbols STREQUAL "")
    message(FATAL_ERROR "B17 tiene simbolos indefinidos: ${undefined_symbols} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}readelf" -lW "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE headers ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT headers MATCHES "GNU_STACK[^\r\n]*[ \t]RW[ \t]+0x")
    message(FATAL_ERROR "B17 requiere GNU_STACK RW: ${headers} ${errors}")
endif()
message(STATUS "Regulador PI de B17 enlazado sin libc/servicios/dispositivos; no ejecutado en MCU. ${output}")

if(symbols MATCHES "__aeabi_[fd]")
    message(FATAL_ERROR "B17 no admite auxiliares flotantes: ${symbols}")
endif()
