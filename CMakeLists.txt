cmake_minimum_required(VERSION 3.20)

project(TPI_ED3_Tacometro LANGUAGES C)
option(TPI_BUILD_ARM "Generar ELF para LPC1769 (requiere toolchain ARM)" OFF)
option(TPI_HOST_SANITIZERS "Instrumentar pruebas host con ASan/UBSan" OFF)
include(CTest)

set(TPI_SOURCES
    Aplicacion/main.c
    Aplicacion/Procesamiento.c
    Servicios/Medicion_Velocidad.c
    Servicios/Temperatura.c
    Servicios/Mandos.c
    Servicios/Driver_Alarmas.c
    Servicios/Comunicacion.c
    Servicios/Driver_Motor.c
    Dispositivos/Timer.c
    Dispositivos/Motor.c
    Dispositivos/Sensor_Optico.c
    Dispositivos/LM35.c
    Dispositivos/Potenciometro.c
    Dispositivos/Selector.c
    Dispositivos/LED.c
    Dispositivos/Buzzer.c
    Dispositivos/UART.c
    Dispositivos/soporte/Adquisicion_ADC_DMA.c
    Dispositivos/soporte/Reloj.c
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
    set(TPI_DRIVERS_DIR Drivers/CMSISv2p00_LPC17xx)
    add_library(tpi_drivers OBJECT
        ${TPI_DRIVERS_DIR}/src/system_LPC17xx.c
        ${TPI_DRIVERS_DIR}/Drivers/src/lpc17xx_adc.c
        ${TPI_DRIVERS_DIR}/Drivers/src/lpc17xx_clkpwr.c
        ${TPI_DRIVERS_DIR}/Drivers/src/lpc17xx_gpdma.c
        ${TPI_DRIVERS_DIR}/Drivers/src/lpc17xx_gpio.c
        ${TPI_DRIVERS_DIR}/Drivers/src/lpc17xx_libcfg_default.c
        ${TPI_DRIVERS_DIR}/Drivers/src/lpc17xx_pinsel.c
        ${TPI_DRIVERS_DIR}/Drivers/src/lpc17xx_pwm.c
        ${TPI_DRIVERS_DIR}/Drivers/src/lpc17xx_timer.c
        ${TPI_DRIVERS_DIR}/Drivers/src/lpc17xx_uart.c)
    target_include_directories(tpi_drivers SYSTEM PRIVATE
        ${TPI_DRIVERS_DIR}/inc ${TPI_DRIVERS_DIR}/Drivers/inc plataforma/compat)
    set_target_properties(tpi_drivers PROPERTIES
        C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    target_compile_options(tpi_drivers PRIVATE
        -ffreestanding -ffunction-sections -fdata-sections
        -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-common)
    add_executable(firmware ${TPI_SOURCES} $<TARGET_OBJECTS:tpi_drivers>
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
        -Wl,--gc-sections -Wl,--build-id=none -Wl,--fatal-warnings -Wl,-z,noexecstack)
    target_link_libraries(firmware PRIVATE gcc)
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
    if(BUILD_TESTING)
        add_test(NAME b00.imagen COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/verificar_elf.py"
            "$<TARGET_FILE:firmware>" --tool-prefix "${TPI_TOOL_PREFIX}")
        add_test(NAME b00.contratos COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/pruebas_b00.py"
            --source "${CMAKE_CURRENT_SOURCE_DIR}" --elf "$<TARGET_FILE:firmware>"
            --tool-prefix "${TPI_TOOL_PREFIX}")
        add_test(NAME b17.enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b17-fixture"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            "-DBUILD_TYPE=$<CONFIG>" -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b17.cmake")
        add_test(NAME b17.enlace_arm_release COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b17-fixture-release"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            "-DBUILD_TYPE=Release" -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b17.cmake")
        add_test(NAME i1.enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/i1-fixture"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            "-DBUILD_TYPE=$<CONFIG>" -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_i1.cmake")
        add_test(NAME i1.enlace_arm_release COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/i1-fixture-release"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            "-DBUILD_TYPE=Release" -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_i1.cmake")
        add_test(NAME b01.enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b01-fixture"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b01.cmake")
        add_test(NAME b02.enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b02-fixture" "-DBUILD_TYPE=$<CONFIG>"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b02.cmake")
        add_test(NAME b03.enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b03-fixture" "-DBUILD_TYPE=$<CONFIG>"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b03.cmake")
        add_test(NAME b04.enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b04-fixture" "-DBUILD_TYPE=$<CONFIG>"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b04.cmake")
        add_test(NAME b05.enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b05-fixture" "-DBUILD_TYPE=$<CONFIG>"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b05.cmake")
        add_test(NAME b05.lm35_enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b05-lm35-fixture" "-DBUILD_TYPE=$<CONFIG>"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b05_lm35.cmake")
        add_test(NAME b06.enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b06-fixture" "-DBUILD_TYPE=$<CONFIG>"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b06.cmake")
        add_test(NAME b07.enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b07-fixture" "-DBUILD_TYPE=$<CONFIG>"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b07.cmake")
        add_test(NAME b08.enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b08-fixture" "-DBUILD_TYPE=$<CONFIG>"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b08.cmake")
        add_test(NAME b09.enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b09-fixture" "-DBUILD_TYPE=$<CONFIG>"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b09.cmake")
        add_test(NAME b10.enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b10-fixture" "-DBUILD_TYPE=$<CONFIG>"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b10.cmake")
    endif()
else()
    if(CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "Use TPI_BUILD_ARM=ON con el toolchain ARM, o un build host separado.")
    endif()
    add_library(firmware OBJECT ${TPI_SOURCES})
endif()

target_include_directories(firmware SYSTEM PRIVATE
    Drivers/CMSISv2p00_LPC17xx/inc Drivers/CMSISv2p00_LPC17xx/Drivers/inc plataforma/compat)

set_target_properties(firmware PROPERTIES
    C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
target_include_directories(firmware PRIVATE
    Aplicacion Servicios Dispositivos Dispositivos/soporte Configuracion)
target_compile_options(firmware PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Werror;-Wconversion;-Wshadow;-Wstrict-prototypes>)

if(NOT TPI_BUILD_ARM AND BUILD_TESTING)
    find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
    add_executable(consulta_medicion_velocidad tests/host/consulta_medicion_velocidad.c
        Servicios/Medicion_Velocidad.c)
    add_executable(pruebas_regulador tests/host/test_regulador.c Aplicacion/Procesamiento.c)
    add_executable(consulta_regulador tests/host/consulta_regulador.c Aplicacion/Procesamiento.c)
    add_executable(pruebas_integracion_regulador tests/host/test_integracion_regulador.c
        Aplicacion/Procesamiento.c Servicios/Driver_Motor.c)
    add_executable(pruebas_procesamiento
        tests/host/test_procesamiento.c Aplicacion/Procesamiento.c)
    add_executable(pruebas_comunicacion tests/host/test_comunicacion.c
        Servicios/Comunicacion.c Aplicacion/Procesamiento.c)
    add_executable(pruebas_medicion_velocidad tests/host/test_medicion_velocidad.c
        Servicios/Medicion_Velocidad.c)
    add_executable(pruebas_mandos tests/host/test_mandos.c
        Servicios/Mandos.c Aplicacion/Procesamiento.c)
    add_executable(pruebas_temperatura tests/host/test_temperatura.c
        Servicios/Temperatura.c)
    add_executable(pruebas_lm35 tests/host/test_lm35.c Dispositivos/LM35.c)
    set(TPI_LM35_VARIANT_TARGETS)
    set(TPI_LM35_ORACLE_CASES)
    foreach(vref IN ITEMS 1000 2000 2540 3300 5000)
        set(lm35_target "pruebas_lm35_vref_${vref}")
        add_executable(${lm35_target} tests/host/test_lm35.c Dispositivos/LM35.c)
        target_compile_definitions(${lm35_target} PRIVATE "ADC_VREF_MV=${vref}")
        list(APPEND TPI_LM35_VARIANT_TARGETS ${lm35_target})
        list(APPEND TPI_LM35_ORACLE_CASES --caso "$<TARGET_FILE:${lm35_target}>" "${vref}")
        add_test(NAME b05r1.lm35_vref_${vref} COMMAND ${lm35_target})
    endforeach()
    foreach(lm35_target IN ITEMS pruebas_lm35 ${TPI_LM35_VARIANT_TARGETS})
        target_include_directories(${lm35_target} PRIVATE Dispositivos Dispositivos/soporte)
        target_include_directories(${lm35_target} SYSTEM PRIVATE
            Drivers/CMSISv2p00_LPC17xx/inc Drivers/CMSISv2p00_LPC17xx/Drivers/inc)
    endforeach()
    add_executable(pruebas_integracion_temperatura tests/host/test_integracion_temperatura.c
        Servicios/Temperatura.c Dispositivos/LM35.c)
    add_executable(pruebas_alarma_temperatura tests/host/test_alarma_temperatura.c
        Aplicacion/Procesamiento.c Servicios/Temperatura.c)
    add_executable(pruebas_driver_alarmas tests/host/test_driver_alarmas.c
        Servicios/Driver_Alarmas.c)
    add_executable(pruebas_integracion_alarmas tests/host/test_integracion_alarmas.c
        Servicios/Driver_Alarmas.c Aplicacion/Procesamiento.c Servicios/Temperatura.c)
    add_executable(pruebas_driver_motor tests/host/test_driver_motor.c
        Servicios/Driver_Motor.c)
    add_executable(pruebas_integracion_motor tests/host/test_integracion_motor.c
        Servicios/Driver_Motor.c Servicios/Mandos.c Aplicacion/Procesamiento.c)
    add_executable(pruebas_telemetria tests/host/test_telemetria.c Servicios/Comunicacion.c)
    add_executable(pruebas_integracion_telemetria tests/host/test_integracion_telemetria.c
        Servicios/Comunicacion.c Servicios/Medicion_Velocidad.c Servicios/Temperatura.c
        Aplicacion/Procesamiento.c)
    add_executable(pruebas_selector tests/host/test_selector.c Servicios/Mandos.c)
    add_executable(pruebas_integracion_selector tests/host/test_integracion_selector.c
        Servicios/Mandos.c Aplicacion/Procesamiento.c Servicios/Driver_Motor.c)
    add_executable(pruebas_potenciometro tests/host/test_potenciometro.c
        Dispositivos/Potenciometro.c)
    target_include_directories(pruebas_potenciometro PRIVATE Dispositivos Dispositivos/soporte)
    target_include_directories(pruebas_potenciometro SYSTEM PRIVATE
        Drivers/CMSISv2p00_LPC17xx/inc Drivers/CMSISv2p00_LPC17xx/Drivers/inc)
    add_executable(pruebas_motor tests/host/test_motor.c Dispositivos/Motor.c)
    target_include_directories(pruebas_motor PRIVATE Dispositivos)
    target_include_directories(pruebas_motor SYSTEM PRIVATE
        Drivers/CMSISv2p00_LPC17xx/inc Drivers/CMSISv2p00_LPC17xx/Drivers/inc)
    add_executable(pruebas_sensor_optico tests/host/test_sensor_optico.c
        Dispositivos/Sensor_Optico.c Servicios/Medicion_Velocidad.c)
    target_include_directories(pruebas_sensor_optico PRIVATE Dispositivos)
    target_include_directories(pruebas_sensor_optico SYSTEM PRIVATE
        Drivers/CMSISv2p00_LPC17xx/inc Drivers/CMSISv2p00_LPC17xx/Drivers/inc plataforma/compat)
    add_executable(pruebas_uart tests/host/test_uart.c)
    add_executable(pruebas_timer tests/host/test_timer.c)
    add_executable(pruebas_reloj tests/host/test_reloj.c)
    add_executable(pruebas_adquisicion_adc_dma tests/host/test_adquisicion_adc_dma.c
        Dispositivos/Potenciometro.c Dispositivos/LM35.c)
    foreach(prueba_i1 pruebas_uart pruebas_timer pruebas_reloj pruebas_adquisicion_adc_dma)
        target_include_directories(${prueba_i1} PRIVATE Dispositivos Dispositivos/soporte tests/host)
        target_include_directories(${prueba_i1} SYSTEM PRIVATE
            Drivers/CMSISv2p00_LPC17xx/inc Drivers/CMSISv2p00_LPC17xx/Drivers/inc plataforma/compat)
    endforeach()
    add_executable(pruebas_dispositivos_gpio tests/host/test_dispositivos_gpio.c
        Dispositivos/Selector.c Dispositivos/LED.c Dispositivos/Buzzer.c Servicios/Driver_Alarmas.c)
    target_include_directories(pruebas_dispositivos_gpio PRIVATE Dispositivos)
    target_include_directories(pruebas_dispositivos_gpio SYSTEM PRIVATE
        Drivers/CMSISv2p00_LPC17xx/inc Drivers/CMSISv2p00_LPC17xx/Drivers/inc plataforma/compat)
    target_include_directories(pruebas_lm35 PRIVATE Dispositivos)
    target_include_directories(pruebas_integracion_temperatura PRIVATE Dispositivos Dispositivos/soporte)
    target_include_directories(pruebas_integracion_temperatura SYSTEM PRIVATE
        Drivers/CMSISv2p00_LPC17xx/inc Drivers/CMSISv2p00_LPC17xx/Drivers/inc)
    target_compile_definitions(pruebas_integracion_temperatura PRIVATE ADC_VREF_MV=2000)
    foreach(prueba pruebas_regulador consulta_regulador pruebas_integracion_regulador pruebas_procesamiento pruebas_comunicacion pruebas_medicion_velocidad pruebas_mandos pruebas_temperatura pruebas_lm35 pruebas_integracion_temperatura consulta_medicion_velocidad pruebas_alarma_temperatura pruebas_driver_alarmas pruebas_integracion_alarmas pruebas_driver_motor pruebas_integracion_motor pruebas_telemetria pruebas_integracion_telemetria pruebas_selector pruebas_integracion_selector pruebas_dispositivos_gpio pruebas_uart pruebas_timer pruebas_reloj pruebas_adquisicion_adc_dma pruebas_sensor_optico pruebas_motor pruebas_potenciometro ${TPI_LM35_VARIANT_TARGETS})
        if(prueba STREQUAL "consulta_medicion_velocidad" OR prueba STREQUAL "pruebas_driver_alarmas")
            target_include_directories(${prueba} PRIVATE Servicios)
        elseif(prueba STREQUAL "pruebas_driver_motor" OR prueba STREQUAL "pruebas_telemetria" OR prueba STREQUAL "pruebas_selector")
            target_include_directories(${prueba} PRIVATE Servicios Configuracion)
        else()
            target_include_directories(${prueba} PRIVATE Aplicacion Servicios Configuracion)
        endif()
        set_target_properties(${prueba} PROPERTIES
            C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
        target_compile_options(${prueba} PRIVATE
            -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes -pedantic-errors)
        if(TPI_HOST_SANITIZERS)
            target_compile_options(${prueba} PRIVATE
                -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(${prueba} PRIVATE -fsanitize=address,undefined)
        endif()
    endforeach()
    add_test(NAME b17.regulador COMMAND pruebas_regulador)
    add_test(NAME b17.integracion_regulador COMMAND pruebas_integracion_regulador)
    add_test(NAME b17.oraculo_planta COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/verificar_regulador.py"
        --programa "$<TARGET_FILE:consulta_regulador>"
        --evidencia "${CMAKE_CURRENT_BINARY_DIR}/evidencia-b17/oraculo")
    add_test(NAME b17.configuracion_control COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/verificar_configuracion_control.py"
        --compilador "${CMAKE_C_COMPILER}" --include "${CMAKE_CURRENT_SOURCE_DIR}/Configuracion"
        --evidencia "${CMAKE_CURRENT_BINARY_DIR}/evidencia-b17/configuracion")
    add_test(NAME b01.modos COMMAND pruebas_procesamiento)
    add_test(NAME b02.comunicacion COMMAND pruebas_comunicacion)
    add_test(NAME b03.medicion_velocidad COMMAND pruebas_medicion_velocidad)
    add_test(NAME b03.oraculo_velocidad COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/verificar_velocidad.py"
        --programa "$<TARGET_FILE:consulta_medicion_velocidad>")
    add_test(NAME b04.mandos COMMAND pruebas_mandos)
    add_test(NAME b05.temperatura COMMAND pruebas_temperatura)
    add_test(NAME b05.lm35 COMMAND pruebas_lm35)
    add_test(NAME b05r1.oraculo_lm35 COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/verificar_lm35.py" ${TPI_LM35_ORACLE_CASES})
    add_test(NAME b05r1.configuracion_lm35 COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/verificar_configuracion_lm35.py"
        --compilador "${CMAKE_C_COMPILER}" --include "${CMAKE_CURRENT_SOURCE_DIR}/Configuracion")
    add_test(NAME b05.integracion_temperatura COMMAND pruebas_integracion_temperatura)
    add_test(NAME b06.alarma_temperatura COMMAND pruebas_alarma_temperatura)
    add_test(NAME b07.driver_alarmas COMMAND pruebas_driver_alarmas)
    add_test(NAME b07.integracion_alarmas COMMAND pruebas_integracion_alarmas)
    add_test(NAME b08.driver_motor COMMAND pruebas_driver_motor)
    add_test(NAME b08.integracion_motor COMMAND pruebas_integracion_motor)
    add_test(NAME b09.telemetria COMMAND pruebas_telemetria)
    add_test(NAME b09.integracion_telemetria COMMAND pruebas_integracion_telemetria)
    add_test(NAME b10.selector COMMAND pruebas_selector)
    add_test(NAME b10.integracion_selector COMMAND pruebas_integracion_selector)
    add_test(NAME b12.dispositivos_gpio COMMAND pruebas_dispositivos_gpio)
    add_test(NAME b13.uart COMMAND pruebas_uart)
    add_test(NAME i1.timer COMMAND pruebas_timer)
    add_test(NAME i1.reloj COMMAND pruebas_reloj)
    add_test(NAME i1.adquisicion_adc_dma COMMAND pruebas_adquisicion_adc_dma)
    add_test(NAME b14.sensor_optico COMMAND pruebas_sensor_optico)
    add_test(NAME b15.motor COMMAND pruebas_motor)
    add_test(NAME b16.potenciometro COMMAND pruebas_potenciometro)
endif()
