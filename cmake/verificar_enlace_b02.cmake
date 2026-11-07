# Enlace de la API Comunicacion/B01 fuera del main de produccion, sin ejecutar codigo ARM.
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
#include "Procesamiento.h"
static Comunicacion_Contexto recepcion;
static Procesamiento_Contexto procesamiento;
int main(void)
{
    uint64_t orden = 0U;
    if (!Comunicacion_Inicializar(&recepcion, orden) ||
        !Procesamiento_Inicializar(&procesamiento, PROCESAMIENTO_MODO_AUTOMATICO, orden)) {
        return 1;
    }
    const char *texto = "RPM 185\n";
    while (*texto != '\0') {
        Comunicacion_Resultado r = Comunicacion_RecibirByte(&recepcion, (uint8_t)*texto, ++orden);
        if (r != COMUNICACION_OK && r != COMUNICACION_ENCOLADA) {
            return 1;
        }
        ++texto;
    }
    Comunicacion_Consigna evento;
    if (Comunicacion_ExtraerConsigna(&recepcion, &evento) != COMUNICACION_EXTRAIDA) {
        return 1;
    }
    Procesamiento_Consigna consigna;
    consigna.rpm = evento.rpm;
    consigna.orden_recepcion = evento.orden_recepcion;
    consigna.valida = true;
    Procesamiento_Entradas entradas;
    entradas.modo = PROCESAMIENTO_MODO_AUTOMATICO;
    entradas.mando_manual_por_mil = 0U;
    entradas.orden_actual = orden;
    entradas.consigna = &consigna;
    Procesamiento_Salida salida;
    if (!Procesamiento_Actualizar(&procesamiento, &entradas, &salida)) {
        return 1;
    }
    return salida.fuente == PROCESAMIENTO_FUENTE_AUTOMATICA && salida.consigna_rpm == 185U ? 0 : 1;
}
]=])
execute_process(COMMAND "${TOOL_PREFIX}gcc"
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 ${TPI_FIXTURE_OPT}
    -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack
    -fno-unwind-tables -fno-asynchronous-unwind-tables
    -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors
    "-I${SOURCE_DIR}/Aplicacion" "-I${SOURCE_DIR}/Configuracion" "-I${SOURCE_DIR}/Servicios" "${OUTPUT_DIR}/main_fixture.c"
    "${SOURCE_DIR}/Aplicacion/Procesamiento.c" "${SOURCE_DIR}/Servicios/Comunicacion.c"
    "${SOURCE_DIR}/plataforma/startup_lpc1769.S"
    -nostdlib "-T${SOURCE_DIR}/plataforma/lpc1769.ld"
    -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings
    "-Wl,-Map=${OUTPUT_DIR}/fixture.map" -o "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fallo de enlace API Comunicacion/B01: ${output} ${errors}")
endif()
execute_process(COMMAND "${PYTHON}" "${SOURCE_DIR}/scripts/verificar_elf.py"
    "${OUTPUT_DIR}/fixture.elf" --tool-prefix "${TOOL_PREFIX}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fixture ARM B02 invalido: ${output} ${errors}")
endif()
message(STATUS "API Comunicacion/B01 enlazada para ARM sin libc; no ejecutada en MCU. ${output}")
