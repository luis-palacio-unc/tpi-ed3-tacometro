# Retener B08 y composicion B04/B01 con salida emulada; no ejecutar ARM.
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
#include "Driver_Motor.h"
#include "Mandos.h"
#include "Procesamiento.h"
typedef struct { volatile uint16_t ultimo; volatile unsigned llamadas; } SalidaEmulada;
static SalidaEmulada motor;
static Procesamiento_Contexto contexto;
static Mandos_MandoManual mando;
static volatile uint32_t lectura = 2048U;
static volatile uint32_t escala = 4095U;
static volatile Driver_Motor_Estado resultado;
static bool EscribirMotorEmulado(void *estado, uint16_t por_mil)
{
    SalidaEmulada *salida = estado;
    salida->ultimo = por_mil;
    ++salida->llamadas;
    return true;
}
static const Driver_Motor_Salida salida = {EscribirMotorEmulado, &motor};
int main(void)
{
    if (!Procesamiento_Inicializar(&contexto, PROCESAMIENTO_MODO_MANUAL, 0U) ||
        !Mandos_Inicializar(&mando) ||
        Mandos_NormalizarManual(&mando, lectura, escala) != MANDOS_VALIDO) {
        return 1;
    }
    Procesamiento_Salida seleccion;
    Procesamiento_Entradas entradas = {PROCESAMIENTO_MODO_MANUAL, mando.por_mil, 1U, 0};
    if (!Procesamiento_Actualizar(&contexto, &entradas, &seleccion) ||
        seleccion.fuente != PROCESAMIENTO_FUENTE_MANUAL) {
        return 1;
    }
    resultado = Driver_Motor_Aplicar(&salida, seleccion.mando_manual_por_mil);
    if (resultado != DRIVER_MOTOR_ACEPTADA || motor.ultimo != 500U || motor.llamadas != 1U) {
        return 1;
    }
    entradas.modo = PROCESAMIENTO_MODO_AUTOMATICO;
    entradas.orden_actual = 2U;
    if (!Procesamiento_Actualizar(&contexto, &entradas, &seleccion) ||
        seleccion.fuente != PROCESAMIENTO_FUENTE_CERO) {
        return 1;
    }
    resultado = Driver_Motor_SolicitarCero(&salida);
    return resultado == DRIVER_MOTOR_ACEPTADA && motor.ultimo == 0U && motor.llamadas == 2U ? 0 : 1;
}
]=])
execute_process(COMMAND "${TOOL_PREFIX}gcc"
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 ${TPI_FIXTURE_OPT}
    -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack
    -fno-unwind-tables -fno-asynchronous-unwind-tables
    -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors
    "-I${SOURCE_DIR}/Servicios" "-I${SOURCE_DIR}/Aplicacion" "-I${SOURCE_DIR}/Configuracion"
    "${OUTPUT_DIR}/main_fixture.c" "${SOURCE_DIR}/Servicios/Driver_Motor.c"
    "${SOURCE_DIR}/Aplicacion/Procesamiento.c" "${SOURCE_DIR}/Servicios/Mandos.c"
    "${SOURCE_DIR}/plataforma/startup_lpc1769.S"
    -nostdlib "-T${SOURCE_DIR}/plataforma/lpc1769.ld"
    -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings -Wl,-z,noexecstack
    "-Wl,-Map=${OUTPUT_DIR}/fixture.map" -lgcc -o "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fallo de enlace B08/B04/B01: ${output} ${errors}")
endif()
execute_process(COMMAND "${PYTHON}" "${SOURCE_DIR}/scripts/verificar_elf.py"
    "${OUTPUT_DIR}/fixture.elf" --tool-prefix "${TOOL_PREFIX}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fixture B08 invalido: ${output} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}nm" --defined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "No se pudieron inspeccionar simbolos B08: ${errors}")
endif()
foreach(required_symbol Driver_Motor_Aplicar Driver_Motor_SolicitarCero
        Mandos_Inicializar Mandos_NormalizarManual Procesamiento_Inicializar
        Procesamiento_Actualizar EscribirMotorEmulado)
    if(NOT symbols MATCHES "[ \t]${required_symbol}[\r\n]")
        message(FATAL_ERROR "API/proveedor B08 ausente: ${required_symbol}")
    endif()
endforeach()
execute_process(COMMAND "${TOOL_PREFIX}nm" --undefined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE undefined_symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT undefined_symbols STREQUAL "")
    message(FATAL_ERROR "B08 tiene simbolos indefinidos: ${undefined_symbols} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}readelf" -lW "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE headers ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT headers MATCHES "GNU_STACK[^\r\n]*[ \t]RW[ \t]+0x")
    message(FATAL_ERROR "B08 requiere GNU_STACK RW: ${headers} ${errors}")
endif()
message(STATUS "B08/B04/B01 y salida emulada enlazados sin libc/dispositivos; no ejecutados en MCU. ${output}")
