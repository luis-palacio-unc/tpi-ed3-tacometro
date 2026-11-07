# Verificación B02 — 2026-11-06

## Resultado

PASS para recepción lógica, cola, integración host B01, enlace ARM y documentación.
11/11 tareas al cerrar. Alcance autorizado; resultado pendiente de aceptación.
Sin sincronización/archivo del candidato B02. Commit local autorizado, sin push.
Base: 1535715fa67dc3ed2a7cb666a40ba803e1a8139b.

## Entorno y ejecución

GCC host 13.3.0, ARM GCC 13.2.1, CMake 3.28.3, Doxygen 1.9.8 y Python 3.12.3.
Logs/artefactos: `/tmp/ed3-b02-20261106-6p_kq52o`; temporales fuera del árbol versionado.
No se instalaron dependencias ni se modificó configuración de hardware.

Desde la raíz (sustituir `<logs>` por esa ruta):

```sh
cmake -S . -B <logs>/host -DTPI_BUILD_ARM=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build <logs>/host --parallel 2
ctest --test-dir <logs>/host --output-on-failure -V
cmake -S . -B <logs>/san -DTPI_BUILD_ARM=OFF -DTPI_HOST_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build <logs>/san --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir <logs>/san --output-on-failure -V
cmake -S . -B <logs>/arm -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake"
cmake --build <logs>/arm --parallel 2
ctest --test-dir <logs>/arm --output-on-failure -V
cmake -S . -B <logs>/release -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake"
cmake --build <logs>/release --parallel 2
ctest --test-dir <logs>/release -R b02.enlace_arm --output-on-failure -V
```

Host: 2/2 CTest, B01 17/17 y B02 17/17 casos C (34 total).
Sanitizadores: mismas dos suites, 34/34, ASan/LeakSanitizer/UBSan sin hallazgos.
Ejecutadas fuera del sandbox por incompatibilidad de LeakSanitizer con ptrace ya
registrada en B01; todos los detectores se mantuvieron habilitados.
ARM Debug: 4/4 CTest (imagen B00, 14/14 contratos B00, fixture B01 y fixture B02).
Release: build ELF correcto y fixture B02 a -O3 correcto (1/1 seleccionado).
La prueba ARM no ejecuta instrucciones en MCU: compila, enlaza e inspecciona ELF.

## Conformidad por escenario

| Escenario | Pruebas C/evidencia | Resultado |
|---|---|---|
| S1 gramática/rango | TramasValidas, FragmentadaYCRLF | PASS |
| S2 inválidos/enteros | ValoresFueraDeRango, FormatosInvalidos, TodosLosBytesEnCampoNumerico, EnterosExtremosNoDesbordan | PASS |
| S3 fragmentos/FIFO/marcas | FragmentadaYCRLF, ConsecutivasYMarcasOriginales | PASS |
| S4 orden/rollover | OrdenRepetidoYRegresivo, OrdenInvalidoEnTerminador, OrdenMaximoYRollover | PASS |
| S5 desborde/resincronización | ExcesoLongitudYRecuperacion | PASS |
| S6 cola llena/circular | ColaLlenaConservaFIFOYRecupera, ColaCircularRepetida | PASS |
| S7 NULL/reinicio/instancias | ParametrosNulosYVacio, ReinicioPurgaColaYTrama, InstanciasIndependientes | PASS |
| S8 antigüedad con B01 | IntegracionAntiguaYNuevaConB01 | PASS |
| S9 builds/documentación | Host/ARM/Release, B00/B01 y Doxygen estricto | PASS de su alcance |

La prueba de bytes recorre los 256 valores posibles en el campo decimal. Los tests
usan retornos y eventos públicos, sin inspeccionar metadatos internos o duplicar parser.
No se configuró umbral de cobertura. No se removieron/deshabilitaron pruebas anteriores.
No se observaron fallos funcionales ni advertencias durante estas ejecuciones.

## Documentación

Doxygen estricto HTML/XML: código 0, sin avisos (07-doxygen.log). XML inspeccionado:
6 USW ESW.2.1.7 (.1 Inicializar, .2 RecibirByte, .3 ExtraerConsigna, .4 ReiniciarTrama,
.5 Decodificar, .6 Encolar), más las 7 previas: 13 funciones documentadas.
Constantes/tipos/campos/retornos/unidades/errores/concurrencia documentados en headers.
Se amplió el Doxyfile parcial; no es todavía documentación final de todos los módulos.

## Recursos y revisión

Objeto Comunicacion ARM -O0: text 818 bytes, data/bss 0, sin undefined symbols.
Contexto estático en fixture: 88 bytes (4 eventos y buffer/metadatos). Contexto B01:
40 bytes. Cola sin heap; buffer de 8 bytes antes de LF, sin terminador C string.
Stack-usage -O0: Inicializar 24, RecibirByte 32, ExtraerConsigna 16, ReiniciarTrama 16,
Decodificar 32, Encolar 32 bytes. Camino del módulo hasta 64 bytes, sin llamador/ISR;
no es medición del máximo real de pila del sistema.

Fixture completo B01+B02: huella flash 2688 bytes Debug, 1872 Release; SRAM 2176
(128 de contextos + 2048 de pila reservada). Firmware principal mínimo: flash 844/840
Debug/Release, SRAM 2048. --gc-sections descarta módulos aún sin integrar a main;
el fixture evita que esto oculte dependencias de enlace, incluso con -O3.

R14 tiene fuente común Configuracion/Limites.h; Procesamiento conserva su alias y
comportamiento 0..185. Servicios no incluyen Aplicacion. Se agregaron includes al
build y fixture B01 para el dato común. main, Procesamiento.c, tests B01, dispositivos,
Drivers y archivos B00/B01 preservados contra SHA-256 inicial. Sin cambios de PLL,
relojes, startup, linker, UART física o protocolo de telemetría.

Captura de órdenes por byte y acceso al contexto deberán serializarse al integrar
ISR; todavía no existe esa integración. Pendientes pruebas físicas de UART, tiempos
reales, sensores/motor y depuración. Sin bloqueos para este alcance B02.
