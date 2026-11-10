# Retener APIs B11/B10 y backend CMSIS real; inspeccion estatica, sin ejecutar MCU.
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
#include "Base_Tiempos.h"
#include "Mandos.h"
static Base_Tiempos_Lectura lectura;
static Mandos_Selector selector;
static volatile uint32_t reloj_fixture = 4000000U;
static volatile Base_Tiempos_Estado resultado;
static volatile Mandos_EstadoSelector resultado_selector;
int main(void)
{
    resultado = Base_Tiempos_Inicializar(reloj_fixture);
    resultado = Base_Tiempos_Leer(&lectura);
    if (Mandos_InicializarSelector(&selector, 20U) && resultado == BASE_TIEMPOS_ACTIVA) {
        resultado_selector = Mandos_ActualizarSelector(&selector, MANDOS_SELECTOR_MODO_MANUAL,
            MANDOS_SELECTOR_FUENTE_VALIDA, lectura.milisegundos);
    }
    resultado = Base_Tiempos_Detener();
    return 0;
}
]=])
execute_process(COMMAND "${TOOL_PREFIX}gcc"
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 ${TPI_FIXTURE_OPT}
    -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack -fstack-usage
    -fno-unwind-tables -fno-asynchronous-unwind-tables
    -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors
    "-I${SOURCE_DIR}/Dispositivos/soporte" "-I${SOURCE_DIR}/Servicios" "-I${SOURCE_DIR}/Configuracion"
    -isystem "${SOURCE_DIR}/Drivers/CMSISv2p00_LPC17xx/inc"
    "${OUTPUT_DIR}/main_fixture.c" "${SOURCE_DIR}/Dispositivos/soporte/Base_Tiempos.c"
    "${SOURCE_DIR}/Dispositivos/soporte/Base_Tiempos_LPC1769.c" "${SOURCE_DIR}/Servicios/Mandos.c"
    "${SOURCE_DIR}/plataforma/startup_lpc1769.S"
    -nostdlib "-T${SOURCE_DIR}/plataforma/lpc1769.ld"
    -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings -Wl,-z,noexecstack
    "-Wl,-Map=${OUTPUT_DIR}/fixture.map" -lgcc -o "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fallo enlace B11/B10: ${output} ${errors}")
endif()
foreach(verificador verificar_elf.py verificar_vector_b11.py)
    set(extra)
    if(verificador STREQUAL "verificar_vector_b11.py")
        set(extra --api)
    endif()
    execute_process(COMMAND "${PYTHON}" "${SOURCE_DIR}/scripts/${verificador}"
        "${OUTPUT_DIR}/fixture.elf" --tool-prefix "${TOOL_PREFIX}" ${extra}
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Fixture B11 invalido: ${output} ${errors}")
    endif()
endforeach()
execute_process(COMMAND "${TOOL_PREFIX}nm" --undefined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE undefined_symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT undefined_symbols STREQUAL "")
    message(FATAL_ERROR "B11 tiene simbolos indefinidos: ${undefined_symbols} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}readelf" -lW "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE headers ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT headers MATCHES "GNU_STACK[^\r\n]*[ \t]RW[ \t]+0x")
    message(FATAL_ERROR "B11 requiere GNU_STACK RW: ${headers} ${errors}")
endif()
message(STATUS "B11/B10 con backend CMSIS real enlazado; no ejecutado en MCU. ${output}")
