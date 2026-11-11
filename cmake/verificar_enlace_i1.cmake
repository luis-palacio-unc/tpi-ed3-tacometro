# Enlazar I1: inicializaciones, ISR y APIs de dispositivos con la biblioteca original
# y Reloj_Init como primera llamada de main. Verificacion estatica; no ejecuta codigo ARM.
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
set(TPI_DRIVERS "${SOURCE_DIR}/Drivers/CMSISv2p00_LPC17xx")
set(TPI_ARM_FLAGS -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 ${TPI_FIXTURE_OPT}
    -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack
    -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-common)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
file(WRITE "${OUTPUT_DIR}/main_fixture.c" [=[
#include "Adquisicion_ADC_DMA.h"
#include "Buzzer.h"
#include "LED.h"
#include "LM35.h"
#include "Motor.h"
#include "Potenciometro.h"
#include "Reloj.h"
#include "Selector.h"
#include "Sensor_Optico.h"
#include "Timer.h"
#include "UART.h"
static const uint8_t trama[53] = {0x52U};
static volatile uint32_t observado;
int main(void)
{
    uint8_t recibidos[8];
    uint32_t pulsos = 0U;
    Reloj_Init();
    Selector_Init();
    LED_Init();
    Buzzer_Init();
    SensorOptico_Init();
    Motor_Init();
    UART0_Init();
    Timer_Init();
    if (!Adquisicion_ADC_DMA_Init()) {
        return 1;
    }
    Timer_Start();
    for (;;) {
        if (Timer_ConsumirVentana(&pulsos)) {
            (void)Motor_Write((uint16_t)(pulsos > 1000U ? 1000U : pulsos));
            (void)UART_Write(trama, (uint32_t)sizeof(trama));
            LED_Write(Selector_Read());
            Buzzer_Write(Adquisicion_ADC_DMA_HayDatos());
        }
        if (UART_Read(recibidos, (uint32_t)sizeof(recibidos)) != 0U) {
            observado = (uint32_t)Pot_Read() + (uint32_t)LM35_Read() + Timer_Read() +
                        SensorOptico_Read() + Adquisicion_ADC_DMA_ContarPares();
        }
    }
}
]=])
set(TPI_DRIVER_OBJECTS)
foreach(driver system_LPC17xx lpc17xx_adc lpc17xx_clkpwr lpc17xx_gpdma lpc17xx_gpio
        lpc17xx_libcfg_default lpc17xx_pinsel lpc17xx_pwm lpc17xx_timer lpc17xx_uart)
    if(driver STREQUAL "system_LPC17xx")
        set(driver_source "${TPI_DRIVERS}/src/${driver}.c")
    else()
        set(driver_source "${TPI_DRIVERS}/Drivers/src/${driver}.c")
    endif()
    execute_process(COMMAND "${TOOL_PREFIX}gcc" ${TPI_ARM_FLAGS}
        -isystem "${TPI_DRIVERS}/inc" -isystem "${TPI_DRIVERS}/Drivers/inc"
        -isystem "${SOURCE_DIR}/plataforma/compat"
        -c "${driver_source}" -o "${OUTPUT_DIR}/${driver}.o"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "No compila la biblioteca original ${driver}: ${output} ${errors}")
    endif()
    list(APPEND TPI_DRIVER_OBJECTS "${OUTPUT_DIR}/${driver}.o")
endforeach()
execute_process(COMMAND "${TOOL_PREFIX}gcc" ${TPI_ARM_FLAGS}
    -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors
    "-I${SOURCE_DIR}/Dispositivos" "-I${SOURCE_DIR}/Dispositivos/soporte"
    "-I${SOURCE_DIR}/Configuracion"
    -isystem "${TPI_DRIVERS}/inc" -isystem "${TPI_DRIVERS}/Drivers/inc"
    -isystem "${SOURCE_DIR}/plataforma/compat"
    "${OUTPUT_DIR}/main_fixture.c"
    "${SOURCE_DIR}/Dispositivos/Timer.c" "${SOURCE_DIR}/Dispositivos/Motor.c"
    "${SOURCE_DIR}/Dispositivos/Sensor_Optico.c" "${SOURCE_DIR}/Dispositivos/LM35.c"
    "${SOURCE_DIR}/Dispositivos/Potenciometro.c" "${SOURCE_DIR}/Dispositivos/Selector.c"
    "${SOURCE_DIR}/Dispositivos/LED.c" "${SOURCE_DIR}/Dispositivos/Buzzer.c"
    "${SOURCE_DIR}/Dispositivos/UART.c"
    "${SOURCE_DIR}/Dispositivos/soporte/Adquisicion_ADC_DMA.c"
    "${SOURCE_DIR}/Dispositivos/soporte/Reloj.c"
    ${TPI_DRIVER_OBJECTS} "${SOURCE_DIR}/plataforma/startup_lpc1769.S"
    -nostdlib "-T${SOURCE_DIR}/plataforma/lpc1769.ld"
    -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings -Wl,-z,noexecstack
    "-Wl,-Map=${OUTPUT_DIR}/fixture.map" -lgcc -o "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fallo de compilacion/enlace I1: ${output} ${errors}")
endif()
execute_process(COMMAND "${PYTHON}" "${SOURCE_DIR}/scripts/verificar_elf.py"
    "${OUTPUT_DIR}/fixture.elf" --tool-prefix "${TOOL_PREFIX}" --exigir-reloj
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Fixture I1 invalido: ${output} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}nm" "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "No se pudieron inspeccionar simbolos I1: ${errors}")
endif()
foreach(required_symbol Selector_Init Selector_Read LED_Init LED_Write Buzzer_Init
        Buzzer_Write SensorOptico_Init SensorOptico_Read Motor_Init Motor_Write
        UART0_Init UART_Read UART_Write Timer_Init Timer_Start Timer_Read
        Timer_ConsumirVentana Adquisicion_ADC_DMA_Init Adquisicion_ADC_DMA_HayDatos
        Adquisicion_ADC_DMA_ContarPares Pot_Read LM35_Read
        TIMER0_IRQHandler UART0_IRQHandler DMA_IRQHandler Reloj_Init SystemInit
        SystemCoreClockUpdate)
    if(NOT symbols MATCHES "[0-9a-f]+ T ${required_symbol}[\r\n]")
        message(FATAL_ERROR "API/ISR/arranque I1 ausente o no global fuerte: ${required_symbol}")
    endif()
endforeach()
foreach(required_data ADC_DMA_Pot ADC_DMA_LM35 SystemCoreClock)
    if(NOT symbols MATCHES "[0-9a-f]+ [BD] ${required_data}[\r\n]")
        message(FATAL_ERROR "Dato I1 ausente: ${required_data}")
    endif()
endforeach()
foreach(default_handler ADC_IRQHandler TIMER1_IRQHandler PWM1_IRQHandler UART1_IRQHandler)
    if(NOT symbols MATCHES "[0-9a-f]+ W ${default_handler}[\r\n]")
        message(FATAL_ERROR "ISR no prevista en I1 con definicion fuerte: ${default_handler}")
    endif()
endforeach()
if(symbols MATCHES "__aeabi_[fd]")
    message(FATAL_ERROR "I1 no admite auxiliares flotantes: ${symbols}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}nm" --undefined-only "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE undefined_symbols ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT undefined_symbols STREQUAL "")
    message(FATAL_ERROR "I1 tiene simbolos indefinidos: ${undefined_symbols} ${errors}")
endif()
execute_process(COMMAND "${TOOL_PREFIX}objcopy" -O binary -j .isr_vector
    "${OUTPUT_DIR}/fixture.elf" "${OUTPUT_DIR}/vectores.bin"
    RESULT_VARIABLE result OUTPUT_VARIABLE output_vectors ERROR_VARIABLE errors)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "No se pudo extraer la tabla de vectores I1: ${errors}")
endif()
file(READ "${OUTPUT_DIR}/vectores.bin" vectors HEX)
foreach(entry "TIMER0_IRQHandler;17" "UART0_IRQHandler;21" "DMA_IRQHandler;42")
    list(GET entry 0 handler)
    list(GET entry 1 index)
    string(REGEX MATCH "([0-9a-f]+) T ${handler}[\r\n]" unused "${symbols}")
    math(EXPR expected "0x${CMAKE_MATCH_1} | 1")
    math(EXPR offset "${index} * 8")
    string(SUBSTRING "${vectors}" ${offset} 8 word)
    string(REGEX REPLACE "(..)(..)(..)(..)" "\\4\\3\\2\\1" word "${word}")
    math(EXPR actual "0x${word}")
    if(NOT actual EQUAL expected)
        message(FATAL_ERROR "Vector ${index} no apunta a ${handler}: 0x${word}")
    endif()
endforeach()
execute_process(COMMAND "${TOOL_PREFIX}readelf" -lW "${OUTPUT_DIR}/fixture.elf"
    RESULT_VARIABLE result OUTPUT_VARIABLE headers ERROR_VARIABLE errors)
if(NOT result EQUAL 0 OR NOT headers MATCHES "GNU_STACK[^\r\n]*[ \t]RW[ \t]+0x")
    message(FATAL_ERROR "I1 requiere GNU_STACK RW: ${headers} ${errors}")
endif()
message(STATUS "I1: inicializaciones, 3 ISR en sus vectores y biblioteca original enlazadas; Reloj_Init primero en main; sin flotantes ni libc; no ejecutado en MCU. ${output}")
