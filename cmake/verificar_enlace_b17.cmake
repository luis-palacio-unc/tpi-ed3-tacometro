# Retener B17 y composicion B01/B03/B08 con PI y salida emulados; no ejecutar ARM.
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
#include "Driver_Motor.h"
#include "Control.h"
static Procesamiento_Contexto app;
static Procesamiento_Regulador pi;
static Medicion_Velocidad_Lectura medicion;
static volatile uint32_t pulsos;
static volatile uint16_t ultimo;
static bool EscribirMotor(void *contexto, uint16_t mando)
{
    (void)contexto;
    ultimo=mando;
    return true;
}
int main(void)
{
    const Procesamiento_ConfigRegulador cfg={TPI_CONTROL_KP_NUM,TPI_CONTROL_KP_DEN,
        TPI_CONTROL_KI_NUM,TPI_CONTROL_KI_DEN,TPI_CONTROL_MAX_PERIODOS_SIN_MEDICION};
    const Driver_Motor_Salida salida={EscribirMotor,0};
    if (!Procesamiento_Inicializar(&app,PROCESAMIENTO_MODO_MANUAL,0U) ||
        !Procesamiento_InicializarRegulador(&pi,&cfg) ||
        !Medicion_Velocidad_Inicializar(&medicion)) { return 1; }
    Procesamiento_Entradas in={PROCESAMIENTO_MODO_AUTOMATICO,0U,1U,0};
    Procesamiento_Salida seleccion;
    if (!Procesamiento_Actualizar(&app,&in,&seleccion) ||
        seleccion.fuente!=PROCESAMIENTO_FUENTE_CERO ||
        !Procesamiento_ReiniciarRegulador(&pi) ||
        Driver_Motor_SolicitarCero(&salida)!=DRIVER_MOTOR_ACEPTADA) { return 1; }
    const Procesamiento_Consigna c={100U,2U,true};
    in.consigna=&c;in.orden_actual=2U;
    if (!Procesamiento_Actualizar(&app,&in,&seleccion) ||
        seleccion.fuente!=PROCESAMIENTO_FUENTE_AUTOMATICA ||
        Medicion_Velocidad_Actualizar(&medicion,pulsos,500000U,60U)!=MEDICION_VELOCIDAD_VALIDA) { return 1; }
    uint16_t mando;
    if (Procesamiento_Regular(&pi,seleccion.consigna_rpm,&medicion,&mando)!=PROCESAMIENTO_REGULADOR_VALIDO) { return 1; }
    return Driver_Motor_Aplicar(&salida,mando)==DRIVER_MOTOR_ACEPTADA ? 0 : 1;
}
]=])
execute_process(COMMAND "${TOOL_PREFIX}gcc"
    -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 ${TPI_FIXTURE_OPT}
    -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack
    -fno-unwind-tables -fno-asynchronous-unwind-tables
    -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors
    "-I${SOURCE_DIR}/Servicios" "-I${SOURCE_DIR}/Aplicacion" "-I${SOURCE_DIR}/Configuracion"
    "${OUTPUT_DIR}/main_fixture.c" "${SOURCE_DIR}/Servicios/Driver_Motor.c"
    "${SOURCE_DIR}/Aplicacion/Procesamiento.c" "${SOURCE_DIR}/Servicios/Medicion_Velocidad.c"
    "${SOURCE_DIR}/plataforma/startup_lpc1769.S"
    -nostdlib "-T${SOURCE_DIR}/plataforma/lpc1769.ld"
    -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings -Wl,-z,noexecstack
    "-Wl,-Map=${OUTPUT_DIR}/fixture.map" -lgcc -o "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fallo de enlace B17/B01/B03/B08: ${output} ${errors}")
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
foreach(required_symbol Procesamiento_Inicializar Procesamiento_Actualizar
        Procesamiento_InicializarRegulador Procesamiento_Regular Procesamiento_ReiniciarRegulador
        Medicion_Velocidad_Inicializar Medicion_Velocidad_Actualizar
        Driver_Motor_Aplicar Driver_Motor_SolicitarCero EscribirMotor)
    if(NOT symbols MATCHES "[ \t]${required_symbol}[\r\n]")
        message(FATAL_ERROR "API/proveedor B17 ausente: ${required_symbol}")
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
message(STATUS "B17/B01/B03/B08 y PI y salida emulados enlazados sin libc/dispositivos; no ejecutados en MCU. ${output}")

if(symbols MATCHES "__aeabi_[fd]")
    message(FATAL_ERROR "B17 no admite auxiliares flotantes: ${symbols}")
endif()
