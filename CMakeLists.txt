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
    Servicios/Sistema.c
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
            "$<TARGET_FILE:firmware>" --tool-prefix "${TPI_TOOL_PREFIX}" --exigir-reloj
            --report "${CMAKE_CURRENT_BINARY_DIR}/memoria.txt"
        VERBATIM)
    if(BUILD_TESTING)
        add_test(NAME b00.imagen COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/verificar_elf.py"
            "$<TARGET_FILE:firmware>" --tool-prefix "${TPI_TOOL_PREFIX}" --exigir-reloj)
        add_test(NAME i2.firmware COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}" "-DELF=$<TARGET_FILE:firmware>"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_firmware_i2.cmake")
        add_test(NAME i2.capas COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/verificar_capas.py"
            --source "${CMAKE_CURRENT_SOURCE_DIR}")
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
        add_test(NAME b05.lm35_enlace_arm COMMAND "${CMAKE_COMMAND}"
            "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/b05-lm35-fixture" "-DBUILD_TYPE=$<CONFIG>"
            "-DTOOL_PREFIX=${TPI_TOOL_PREFIX}" "-DPYTHON=${Python3_EXECUTABLE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verificar_enlace_b05_lm35.cmake")
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
    set(TPI_CMSIS_INC Drivers/CMSISv2p00_LPC17xx/inc Drivers/CMSISv2p00_LPC17xx/Drivers/inc)

    # Regulador PI: logica pura. El oraculo compila un emisor por juego de ganancias.
    add_executable(pruebas_regulador tests/host/test_regulador.c Aplicacion/Procesamiento.c)
    set(TPI_REGULADOR_TARGETS pruebas_regulador)
    set(TPI_REGULADOR_ORACLE_CASES)
    foreach(ganancias IN ITEMS 4_1000_1_1000 1_1000_1_1000 1000_1_1000_1 1_1_1_1
            1000_1000_1000_1000 3_7_2_9 1_1000_1000_1 1_7_5_3)
        string(REPLACE "_" ";" g "${ganancias}")
        list(GET g 0 kp_num)
        list(GET g 1 kp_den)
        list(GET g 2 ki_num)
        list(GET g 3 ki_den)
        set(regulador_target "consulta_regulador_${ganancias}")
        add_executable(${regulador_target} tests/host/consulta_regulador.c
            Aplicacion/Procesamiento.c)
        target_compile_definitions(${regulador_target} PRIVATE
            "TPI_CONTROL_KP_NUM=${kp_num}" "TPI_CONTROL_KP_DEN=${kp_den}"
            "TPI_CONTROL_KI_NUM=${ki_num}" "TPI_CONTROL_KI_DEN=${ki_den}")
        list(APPEND TPI_REGULADOR_TARGETS ${regulador_target})
        list(APPEND TPI_REGULADOR_ORACLE_CASES --caso "$<TARGET_FILE:${regulador_target}>"
            ${kp_num} ${kp_den} ${ki_num} ${ki_den})
    endforeach()

    # I2: servicios, Procesamiento y aplicacion completa con dispositivos simulados.
    set(TPI_FAKES tests/host/fakes_dispositivos.c)
    set(TPI_SERVICIOS Servicios/Sistema.c Servicios/Mandos.c Servicios/Medicion_Velocidad.c
        Servicios/Temperatura.c Servicios/Driver_Motor.c Servicios/Driver_Alarmas.c
        Servicios/Comunicacion.c)
    add_executable(pruebas_sistema tests/host/test_sistema.c Servicios/Sistema.c ${TPI_FAKES})
    add_executable(pruebas_mandos tests/host/test_mandos.c Servicios/Mandos.c ${TPI_FAKES})
    add_executable(pruebas_medicion_velocidad tests/host/test_medicion_velocidad.c
        Servicios/Medicion_Velocidad.c ${TPI_FAKES})
    add_executable(pruebas_temperatura tests/host/test_temperatura.c
        Servicios/Temperatura.c ${TPI_FAKES})
    add_executable(pruebas_drivers tests/host/test_drivers.c
        Servicios/Driver_Motor.c Servicios/Driver_Alarmas.c ${TPI_FAKES})
    add_executable(pruebas_comunicacion tests/host/test_comunicacion.c
        Servicios/Comunicacion.c ${TPI_FAKES})
    add_executable(pruebas_procesamiento tests/host/test_procesamiento.c
        Aplicacion/Procesamiento.c)
    # main.c se compila aparte con main renombrado para enlazarlo con la prueba.
    add_library(tpi_main_bajo_prueba OBJECT Aplicacion/main.c)
    target_compile_definitions(tpi_main_bajo_prueba PRIVATE main=Aplicacion_MainFirmware)
    add_executable(pruebas_aplicacion tests/host/test_aplicacion.c
        $<TARGET_OBJECTS:tpi_main_bajo_prueba> Aplicacion/Procesamiento.c
        ${TPI_SERVICIOS} ${TPI_FAKES})
    set(TPI_PRUEBAS_I2 pruebas_sistema pruebas_mandos pruebas_medicion_velocidad
        pruebas_temperatura pruebas_drivers pruebas_comunicacion pruebas_procesamiento
        tpi_main_bajo_prueba pruebas_aplicacion)
    foreach(prueba IN LISTS TPI_PRUEBAS_I2)
        target_include_directories(${prueba} PRIVATE Dispositivos Dispositivos/soporte tests/host)
    endforeach()

    # Dispositivos: sus .c reales contra dobles de la biblioteca.
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
    add_executable(pruebas_integracion_temperatura tests/host/test_integracion_temperatura.c
        Servicios/Temperatura.c Dispositivos/LM35.c)
    target_compile_definitions(pruebas_integracion_temperatura PRIVATE ADC_VREF_MV=2000)
    add_executable(pruebas_potenciometro tests/host/test_potenciometro.c
        Dispositivos/Potenciometro.c)
    add_executable(pruebas_motor tests/host/test_motor.c Dispositivos/Motor.c)
    add_executable(pruebas_sensor_optico tests/host/test_sensor_optico.c
        Dispositivos/Sensor_Optico.c)
    add_executable(pruebas_uart tests/host/test_uart.c)
    add_executable(pruebas_timer tests/host/test_timer.c)
    add_executable(pruebas_reloj tests/host/test_reloj.c)
    add_executable(pruebas_adquisicion_adc_dma tests/host/test_adquisicion_adc_dma.c
        Dispositivos/Potenciometro.c Dispositivos/LM35.c)
    add_executable(pruebas_dispositivos_gpio tests/host/test_dispositivos_gpio.c
        Dispositivos/Selector.c Dispositivos/LED.c Dispositivos/Buzzer.c Servicios/Driver_Alarmas.c)
    set(TPI_PRUEBAS_DISPOSITIVOS pruebas_lm35 ${TPI_LM35_VARIANT_TARGETS}
        pruebas_integracion_temperatura pruebas_potenciometro pruebas_motor
        pruebas_sensor_optico pruebas_uart pruebas_timer pruebas_reloj
        pruebas_adquisicion_adc_dma pruebas_dispositivos_gpio)
    foreach(prueba IN LISTS TPI_PRUEBAS_DISPOSITIVOS)
        target_include_directories(${prueba} PRIVATE Dispositivos Dispositivos/soporte tests/host)
        target_include_directories(${prueba} SYSTEM PRIVATE ${TPI_CMSIS_INC} plataforma/compat)
    endforeach()

    foreach(prueba IN LISTS TPI_PRUEBAS_I2 TPI_PRUEBAS_DISPOSITIVOS
            TPI_REGULADOR_TARGETS)
        target_include_directories(${prueba} PRIVATE Aplicacion Servicios Configuracion)
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
    add_test(NAME b17.oraculo_planta COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/verificar_regulador.py"
        ${TPI_REGULADOR_ORACLE_CASES}
        --evidencia "${CMAKE_CURRENT_BINARY_DIR}/evidencia-b17/oraculo")
    add_test(NAME b17.configuracion_control COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/verificar_configuracion_control.py"
        --compilador "${CMAKE_C_COMPILER}" --include "${CMAKE_CURRENT_SOURCE_DIR}/Configuracion"
        --evidencia "${CMAKE_CURRENT_BINARY_DIR}/evidencia-b17/configuracion")
    add_test(NAME i2.capas COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/scripts/verificar_capas.py"
        --source "${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME i2.sistema COMMAND pruebas_sistema)
    add_test(NAME i2.mandos COMMAND pruebas_mandos)
    add_test(NAME i2.medicion_velocidad COMMAND pruebas_medicion_velocidad)
    add_test(NAME i2.temperatura COMMAND pruebas_temperatura)
    add_test(NAME i2.temperatura_lm35 COMMAND pruebas_integracion_temperatura)
    add_test(NAME i2.drivers COMMAND pruebas_drivers)
    add_test(NAME i2.comunicacion COMMAND pruebas_comunicacion)
    add_test(NAME i2.procesamiento COMMAND pruebas_procesamiento)
    add_test(NAME i2.aplicacion COMMAND pruebas_aplicacion)
    add_test(NAME b05.lm35 COMMAND pruebas_lm35)
    add_test(NAME b05r1.oraculo_lm35 COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/verificar_lm35.py" ${TPI_LM35_ORACLE_CASES})
    add_test(NAME b05r1.configuracion_lm35 COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/verificar_configuracion_lm35.py"
        --compilador "${CMAKE_C_COMPILER}" --include "${CMAKE_CURRENT_SOURCE_DIR}/Configuracion")
    add_test(NAME b12.dispositivos_gpio COMMAND pruebas_dispositivos_gpio)
    add_test(NAME b13.uart COMMAND pruebas_uart)
    add_test(NAME i1.timer COMMAND pruebas_timer)
    add_test(NAME i1.reloj COMMAND pruebas_reloj)
    add_test(NAME i1.adquisicion_adc_dma COMMAND pruebas_adquisicion_adc_dma)
    add_test(NAME b14.sensor_optico COMMAND pruebas_sensor_optico)
    add_test(NAME b15.motor COMMAND pruebas_motor)
    add_test(NAME b16.potenciometro COMMAND pruebas_potenciometro)
endif()
