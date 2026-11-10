# Retener servicio B07 y conexion B05/B06 con salidas emuladas; no ejecutar ARM.
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
#include "Driver_Alarmas.h"
#include "Procesamiento.h"
typedef struct { volatile bool ultima_solicitud; volatile unsigned llamadas; } SalidaEmulada;
static SalidaEmulada led, buzzer;
static Temperatura_Lectura lectura;
static volatile int32_t temperatura = 2534;
static volatile int32_t umbral = 2500;
static volatile Driver_Alarmas_Estado resultado;
static void EscribirLedEmulado(bool activar)
{
    led.ultima_solicitud = activar;
    ++led.llamadas;
}
static void EscribirBuzzerEmulado(bool activar)
{
    buzzer.ultima_solicitud = activar;
    ++buzzer.llamadas;
}
static const Driver_Alarmas_Salidas salidas = {
    EscribirLedEmulado, EscribirBuzzerEmulado
};
int main(void)
{
    if (!Temperatura_Inicializar(&lectura) ||
        Temperatura_Actualizar(&lectura, temperatura, TEMPERATURA_FUENTE_VALIDA) != TEMPERATURA_VALIDA) {
        return 1;
    }
    const Procesamiento_EstadoAlarma decision = Procesamiento_EvaluarAlarmaTemperatura(&lectura, umbral);
    if (decision != PROCESAMIENTO_ALARMA_ACTIVA && decision != PROCESAMIENTO_ALARMA_INACTIVA) {
        return 1;
    }
    resultado = Driver_Alarmas_Aplicar(&salidas, decision == PROCESAMIENTO_ALARMA_ACTIVA);
    return resultado == DRIVER_ALARMAS_ACEPTADA && led.ultima_solicitud &&
           buzzer.ultima_solicitud && led.llamadas == 1U && buzzer.llamadas == 1U ? 0 : 1;
}
]=])
execute_process(COMMAND "${TOOL_PREFIX}gcc"
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 ${TPI_FIXTURE_OPT}
    -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack
    -fno-unwind-tables -fno-asynchronous-unwind-tables
    -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors
    "-I${SOURCE_DIR}/Servicios" "-I${SOURCE_DIR}/Aplicacion" "-I${SOURCE_DIR}/Configuracion"
    "${OUTPUT_DIR}/main_fixture.c" "${SOURCE_DIR}/Servicios/Driver_Alarmas.c"
    "${SOURCE_DIR}/Aplicacion/Procesamiento.c" "${SOURCE_DIR}/Servicios/Temperatura.c"
    "${SOURCE_DIR}/plataforma/startup_lpc1769.S"
    -nostdlib "-T${SOURCE_DIR}/plataforma/lpc1769.ld"
    -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings -Wl,-z,noexecstack
    "-Wl,-Map=${OUTPUT_DIR}/fixture.map" -o "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fallo de enlace B07/B06/B05: ${output} ${errors}")
endif()
execute_process(COMMAND "${PYTHON}" "${SOURCE_DIR}/scripts/verificar_elf.py"
    "${OUTPUT_DIR}/fixture.elf" --tool-prefix "${TOOL_PREFIX}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fixture B07 invalido: ${output} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}nm" --defined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "No se pudieron inspeccionar simbolos B07: ${errors}")
endif()
foreach(required_symbol Driver_Alarmas_Aplicar Procesamiento_EvaluarAlarmaTemperatura
        Temperatura_Inicializar Temperatura_Actualizar EscribirLedEmulado EscribirBuzzerEmulado)
    if(NOT symbols MATCHES "[ \t]${required_symbol}[\r\n]")
        message(FATAL_ERROR "API/proveedor B07 ausente: ${required_symbol}")
    endif()
endforeach()
execute_process(COMMAND "${TOOL_PREFIX}nm" --undefined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE undefined_symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT undefined_symbols STREQUAL "")
    message(FATAL_ERROR "B07 tiene simbolos indefinidos: ${undefined_symbols} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}readelf" -lW "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE headers ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT headers MATCHES "GNU_STACK[^\r\n]*[ \t]RW[ \t]+0x")
    message(FATAL_ERROR "B07 requiere GNU_STACK RW: ${headers} ${errors}")
endif()
message(STATUS "B07/B06/B05 y salidas emuladas enlazados sin libc/dispositivos; no ejecutados en MCU. ${output}")
