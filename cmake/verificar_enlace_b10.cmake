# Retener B10 y composicion B04/B01/B08 con selector y salida emulados; no ejecutar ARM.
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
#include "Driver_Motor.h"
typedef struct { volatile unsigned llamadas; volatile uint16_t ultimo; } MotorEmulado;
static MotorEmulado motor;
static Mandos_Selector selector;
static Mandos_MandoManual manual;
static Procesamiento_Contexto app;
static volatile uint32_t ahora;
static volatile Driver_Motor_Estado resultado;
static bool EscribirMotor(void *contexto, uint16_t mando)
{
    MotorEmulado *estado = contexto;
    ++estado->llamadas;
    estado->ultimo = mando;
    return true;
}
static const Driver_Motor_Salida salida_motor = {EscribirMotor, &motor};
int main(void)
{
    const uint32_t inicio = ahora;
    if (!Mandos_InicializarSelector(&selector, 5U) || !Mandos_Inicializar(&manual) ||
        Mandos_NormalizarManual(&manual, 1U, 2U) != MANDOS_VALIDO ||
        Mandos_ActualizarSelector(&selector, MANDOS_SELECTOR_MODO_MANUAL, MANDOS_SELECTOR_FUENTE_VALIDA, inicio) != MANDOS_SELECTOR_SIN_DATOS ||
        Mandos_ActualizarSelector(&selector, MANDOS_SELECTOR_MODO_MANUAL, MANDOS_SELECTOR_FUENTE_VALIDA, inicio + 5U) != MANDOS_SELECTOR_VALIDO ||
        selector.modo != MANDOS_SELECTOR_MODO_MANUAL ||
        !Procesamiento_Inicializar(&app, PROCESAMIENTO_MODO_MANUAL, 0U)) {
        return 1;
    }
    Procesamiento_Entradas entradas = {PROCESAMIENTO_MODO_MANUAL, manual.por_mil, 1U, 0};
    Procesamiento_Salida seleccion;
    if (!Procesamiento_Actualizar(&app, &entradas, &seleccion) || seleccion.fuente != PROCESAMIENTO_FUENTE_MANUAL) {
        return 1;
    }
    resultado = Driver_Motor_Aplicar(&salida_motor, seleccion.mando_manual_por_mil);
    if (resultado != DRIVER_MOTOR_ACEPTADA || motor.ultimo != 500U) {
        return 1;
    }
    if (Mandos_ActualizarSelector(&selector, MANDOS_SELECTOR_MODO_AUTOMATICO, MANDOS_SELECTOR_FUENTE_VALIDA, inicio + 6U) != MANDOS_SELECTOR_VALIDO ||
        selector.modo != MANDOS_SELECTOR_MODO_MANUAL ||
        Mandos_ActualizarSelector(&selector, MANDOS_SELECTOR_MODO_AUTOMATICO, MANDOS_SELECTOR_FUENTE_VALIDA, inicio + 11U) != MANDOS_SELECTOR_VALIDO ||
        selector.modo != MANDOS_SELECTOR_MODO_AUTOMATICO) {
        return 1;
    }
    entradas.modo = PROCESAMIENTO_MODO_AUTOMATICO;
    entradas.orden_actual = 2U;
    if (!Procesamiento_Actualizar(&app, &entradas, &seleccion) || seleccion.fuente != PROCESAMIENTO_FUENTE_CERO) {
        return 1;
    }
    resultado = Driver_Motor_SolicitarCero(&salida_motor);
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
    message(FATAL_ERROR "Fallo de enlace B10/B04/B01/B08: ${output} ${errors}")
endif()
execute_process(COMMAND "${PYTHON}" "${SOURCE_DIR}/scripts/verificar_elf.py"
    "${OUTPUT_DIR}/fixture.elf" --tool-prefix "${TOOL_PREFIX}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fixture B10 invalido: ${output} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}nm" --defined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "No se pudieron inspeccionar simbolos B10: ${errors}")
endif()
foreach(required_symbol Mandos_InicializarSelector Mandos_ActualizarSelector
        Mandos_Inicializar Mandos_NormalizarManual Procesamiento_Inicializar
        Procesamiento_Actualizar Driver_Motor_Aplicar Driver_Motor_SolicitarCero EscribirMotor)
    if(NOT symbols MATCHES "[ \t]${required_symbol}[\r\n]")
        message(FATAL_ERROR "API/proveedor B10 ausente: ${required_symbol}")
    endif()
endforeach()
execute_process(COMMAND "${TOOL_PREFIX}nm" --undefined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE undefined_symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT undefined_symbols STREQUAL "")
    message(FATAL_ERROR "B10 tiene simbolos indefinidos: ${undefined_symbols} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}readelf" -lW "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE headers ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT headers MATCHES "GNU_STACK[^\r\n]*[ \t]RW[ \t]+0x")
    message(FATAL_ERROR "B10 requiere GNU_STACK RW: ${headers} ${errors}")
endif()
message(STATUS "B10/B04/B01/B08 y selector y salida emulados enlazados sin libc/dispositivos; no ejecutados en MCU. ${output}")
