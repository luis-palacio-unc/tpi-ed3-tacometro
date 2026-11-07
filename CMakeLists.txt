cmake_minimum_required(VERSION 3.20)

project(TPI_ED3_Tacometro LANGUAGES C)
option(TPI_BUILD_ARM "Generar ELF para LPC1769 (requiere toolchain ARM)" OFF)

set(TPI_SOURCES
    Aplicacion/main.c
    Aplicacion/Procesamiento.c
    Servicios/Medicion_Velocidad.c
    Servicios/Temperatura.c
    Servicios/Mandos.c
    Servicios/Driver_Alarmas.c
    Servicios/Comunicacion.c
    Servicios/Driver_Motor.c
    Dispositivos/Motor.c
    Dispositivos/Sensor_Optico.c
    Dispositivos/LM35.c
    Dispositivos/Potenciometro.c
    Dispositivos/Selector.c
    Dispositivos/LED.c
    Dispositivos/Buzzer.c
    Dispositivos/UART.c
    Dispositivos/soporte/Adquisicion_ADC_DMA.c
    Dispositivos/soporte/Base_Tiempos.c
)

if(TPI_BUILD_ARM)
    if(NOT CMAKE_CROSSCOMPILING OR NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
        message(FATAL_ERROR "TPI_BUILD_ARM requiere cmake/arm-none-eabi.cmake; no se permite usar el compilador host.")
    endif()
    execute_process(COMMAND "${CMAKE_C_COMPILER}" -dumpmachine
        OUTPUT_VARIABLE TPI_COMPILER_TARGET OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY)
    if(NOT TPI_COMPILER_TARGET STREQUAL "arm-none-eabi")
        message(FATAL_ERROR "El compilador seleccionado no es arm-none-eabi.")
    endif()
    enable_language(ASM)
    find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
    add_executable(firmware ${TPI_SOURCES}
        plataforma/startup_lpc1769.S plataforma/cmsis_compat.c)
    set_target_properties(firmware PROPERTIES SUFFIX ".elf")
    target_include_directories(firmware SYSTEM PRIVATE
        Drivers/CMSISv2p00_LPC17xx/inc)
    target_compile_options(firmware PRIVATE
        -ffreestanding -ffunction-sections -fdata-sections -Wa,--noexecstack
        -fno-unwind-tables -fno-asynchronous-unwind-tables
        $<$<COMPILE_LANGUAGE:C>:-fno-common>)
    target_link_options(firmware PRIVATE -nostdlib
        "-T${CMAKE_CURRENT_SOURCE_DIR}/plataforma/lpc1769.ld"
        "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/firmware.map"
        -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings)
    set_property(TARGET firmware APPEND PROPERTY LINK_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/plataforma/lpc1769.ld"
        "${CMAKE_CURRENT_SOURCE_DIR}/scripts/verificar_elf.py")
    get_filename_component(TPI_ARM_BIN "${CMAKE_C_COMPILER}" DIRECTORY)
    set(TPI_TOOL_PREFIX "${TPI_ARM_BIN}/arm-none-eabi-")
    add_custom_command(TARGET firmware POST_BUILD
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/verificar_elf.py"
            "$<TARGET_FILE:firmware>" --tool-prefix "${TPI_TOOL_PREFIX}"
            --report "${CMAKE_CURRENT_BINARY_DIR}/memoria.txt"
        VERBATIM)
    include(CTest)
    if(BUILD_TESTING)
        add_test(NAME b00.imagen COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/verificar_elf.py"
            "$<TARGET_FILE:firmware>" --tool-prefix "${TPI_TOOL_PREFIX}")
        add_test(NAME b00.contratos COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/pruebas_b00.py"
            --source "${CMAKE_CURRENT_SOURCE_DIR}" --elf "$<TARGET_FILE:firmware>"
            --tool-prefix "${TPI_TOOL_PREFIX}")
    endif()
else()
    if(CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "Use TPI_BUILD_ARM=ON con el toolchain ARM, o un build host separado.")
    endif()
    add_library(firmware OBJECT ${TPI_SOURCES})
endif()

set_target_properties(firmware PROPERTIES
    C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
target_include_directories(firmware PRIVATE
    Aplicacion Servicios Dispositivos Dispositivos/soporte)
target_compile_options(firmware PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Werror;-Wconversion;-Wshadow;-Wstrict-prototypes>)
