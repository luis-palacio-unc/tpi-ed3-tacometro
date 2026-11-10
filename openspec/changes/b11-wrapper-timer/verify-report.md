# Verificación B11 — tres wrappers Timer

2026-11-10. Alcance autorizado por el usuario; resultado pendiente de aceptación.
No sincronizado/archivado, sin push. Propuestas temporales anteriores descartadas.
Cambios locales previos y restitución del usuario preservados; snapshot externo.

## Implementación

Dispositivos/Timer.c/.h: Init configura TIM_US/TIMER_BASE_MS*1000 con TIM_InitTimer,
Start llama TIM_Enable, Read retorna TIM_ReadTimer*TIMER_BASE_MS modular.
TIMER0 sólo en .c; header stdint; default1ms configurable por macro. Se eliminó
Configuracion/Temporizacion.h. Tres USW documentadas, sin estado mutable/helpers/
ISR/counter software/proveedores/servicio Tiempo. No modificar driver por limitaciones.

## Evidencia ejecutada

Logs externos: `/tmp/ed3-b11-wrapper-timer-bwqdkfu3`. C11 estricto -Wall -Wextra -Werror -Wconversion -Wshadow
-Wstrict-prototypes; comprobaciones de objeto/configuración además -pedantic-errors.

| Verificación | Resultado | Log |
|---|---|---|
| Host Debug build/regresiones | PASS17/17,179 casos C previos + oráculo Python; no ejecutar Timer en host | host-*.log |
| ARM Debug build/controles | PASS13/13; B00 conserva14 casos,18 objetos host exactos | arm-*.log |
| Wrapper ARM, llamadas del objeto | Sólo TIM_InitTimer, TIM_Enable y TIM_ReadTimer como símbolos externos | timer-undefined.log |
| Configuraciones compiladas ARM-O2 | Bases1/2/5 PASS;0 rechazado por static_assert | base-*.log |
| Header independiente | Compila ejemplo de consulta/resta con sólo Dispositivos, sin CMSIS/driver | header-isolation.log |
| Doxygen estricto HTML/XML | PASS35 funciones únicas propias,3 Timer, sin avisos | doxygen.log |
| Preservación | SHA25690 archivos Drivers antes/después iguales; contenido original a21cc1e | drivers-before.json y revisión final |

No se agregaron pruebas que sólo repitan llamadas del wrapper. Las suites host/ARM
anteriores son regresiones, no pruebas funcionales del nuevo timer. No instalar ni
desactivar controles, no flashear/ejecutar MCU. Servicios y main conservados.

## Escenarios y límites

- WT1/S1: fuente/objeto confirman configuración en microsegundos y delegación de
  arranque. Ejecución sobre TIMER0/PCLK real pendiente de integración/hardware.
- WT2/S2: inspección verifica normalización uint32 a ms y lectura sin escrituras;
  bases1/2/5 compilan, cero inválido rechazado. No se midieron intervalos físicos.
- WT3/S3: archivo eliminado, header aislado, driver íntegro, compilación y Doxygen.

Objeto ARM Debug del wrapper: .text86 bytes, .data0/.bss0; no incluye driver ni
imagen completa. Main mínimo no llama Timer; GC elimina el wrapper sin uso del ELF.
El bloque valida compilación del wrapper contra headers originales. No se acredita
enlace/ejecución del conjunto de fuentes de biblioteca, pendiente al integrar arranque.
Se prohíbe modificar Drivers; cualquier ajuste de integración debe quedar fuera de ellos.

Precondiciones: TIMER0 exclusivo desde reset sin match/captura previa, reloj informado
SystemCoreClock correcto, período exacto/representable según driver y acceso serializado.
Init antes de Start/Read; reinit abre época nueva. Marcas uint32 modulares, duración
real<2^31ms, no detectar épocas mezcladas/vueltas omitidas. Reloj/frecuencia física
no confirmados. Sin promesas de avance durante suspensión/debug.

Reproducción desde raíz R y salida O: cmake -S "$R" -B "$O/host" -DCMAKE_BUILD_TYPE=Debug;
cmake --build "$O/host" --parallel 4; ctest --test-dir "$O/host" --output-on-failure.
ARM usa directorio propio, -DTPI_BUILD_ARM=ON y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake"; misma secuencia build/CTest.
Doxygen usa docs/doxygen/Doxyfile con OUTPUT_DIRECTORY externo. Overrides se compilan
con arm-none-eabi-gcc -DTIMER_BASE_MS=valor; sólo compilación, no ejecución ARM.

Próxima acción: revisar/aceptar el resultado del wrapper antes de promover specs.

## Corrección del encabezado posterior a39ab465
+
+Usuario informó «Cannot resolve symbol _Static_assert» en Timer.h. Reproducción:
+header válido en C11, pero g++17 estricto rechaza _Static_assert en línea23.
+Esto es compatible con una interpretación del header fuera de C11; no se observó
+directamente el analizador de CLion. Se sustituyó por #if/#error equivalente:
+misma validación positiva/conversión uint32, sin cambios de API ni runtime.
+
+Ejecutado después: header C11/C++17/ARM-C11 con default1 y bases2/5 PASS;0 y4294968
+rechazados como corresponde en los tres modos. Timer.c ARM-C11 estricto compila;
+host17/17 y Doxygen HTML/XML sin avisos. Drivers90 hashes intactos y originales.
+No prueba física ni verificación visual de CLion. Logs: /tmp/ed3-timer-header-error-nleunr8u.
+