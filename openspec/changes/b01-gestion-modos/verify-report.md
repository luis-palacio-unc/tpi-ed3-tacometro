# Verificación B01 — 2026-11-06

## Resultado y alcance

PASS para lógica aislada, compilación/enlace y documentación. 11/11 tareas al cierre;
resultado pendiente de aceptación. No sincronizar specs ni archivar aún.
Base: a21cc1e3b001c9cdfdcf0379710e1694c924932e en main. Commit B01 autorizado;
push no autorizado. Sin integración de servicios o main, control PI/PID ni hardware.

## Ejecución real

GCC host 13.3.0, ARM 13.2.1, CMake 3.28.3, Doxygen 1.9.8, Python 3.12.3.
Logs/artefactos: `/tmp/ed3-b01-20261106-02a8ln3f`; fuera del árbol versionado.

Comandos desde raíz (sustituir `<logs>` por esa ruta):

```sh
cmake -S . -B <logs>/host -DTPI_BUILD_ARM=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build <logs>/host --parallel 2
ctest --test-dir <logs>/host --output-on-failure -V
cmake -S . -B <logs>/san -DTPI_BUILD_ARM=OFF -DBUILD_TESTING=ON -DTPI_HOST_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build <logs>/san --target pruebas_procesamiento --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir <logs>/san --output-on-failure -V
cmake -S . -B <logs>/arm -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake"
cmake --build <logs>/arm --parallel 2
ctest --test-dir <logs>/arm --output-on-failure -V
cmake -S . -B <logs>/release -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake"
cmake --build <logs>/release --parallel 2
```

Host: CTest 1/1 con 17/17 casos C. Misma suite con AddressSanitizer, LeakSanitizer y
UBSan: 17/17, sin hallazgos (ejecución fuera del sandbox, ver limitación abajo).
ARM Debug: CTest 3/3 = imagen B00, contratos B00 (14/14) y enlace de API B01.
ARM Release compila/verifica ELF. No se ejecutaron instrucciones ARM en MCU o simulador.

## Matriz de conformidad

| Requisito/escenario | Pruebas/ejecución | Resultado |
|---|---|---|
| R1/S1 manual | ManualLimites, ManualIgnoraConsigna | PASS |
| R1/S2 automático/potenciómetro | ConsignaNuevaEIgnorarPotenciometro | PASS |
| R2/S3 espera al entrar | ArranqueAutomatico, CambioYConsignaSimultaneos, ReiniciarPurgaConsigna | PASS |
| R2/S4 nueva consigna | ConsignaNuevaEIgnorarPotenciometro, CambioYConsignaSimultaneos | PASS |
| R2/S5 reentrada/cola | ManualIgnoraConsigna, ReentradaDescartaColaAnterior | PASS |
| R3/S6 descarte sin sustitución | DescartesMientrasEspera, DescartesNoSustituyenActiva | PASS |
| R4/S7 errores | ManualFueraDeRango, ErrorModoYRecuperacion, RegresionDeOrden, InicializacionInvalida, ParametrosNulos | PASS |
| R4/S8 fronteras/instancias | LimitesRPM, OrdenMaximoSinRollover, ContextosIndependientes | PASS |
| R5/S9 build/docs | Builds, b01.enlace_arm, B00, Doxygen y XML | PASS, alcance estático ARM |

No se configuró umbral de cobertura. Los tests observan retornos/salidas públicas;
no duplican la implementación de las transiciones ni inspeccionan campos privados.
La prueba B00 de objetos host construye target firmware explícito para conservar su
propósito original de 18 objetos, sin contar los objetos de nuevos tests. No se quita
ninguna prueba B00; las pruebas funcionales se ejecutan en CTest host independiente.

## Documentación y recursos

Doxygen estricto HTML/XML: código 0 y sin advertencias. XML inspeccionado: main
USW.2.1.1.1 y seis USW de Procesamiento (2 públicas, 4 internas) con sus IDs.
Una sonda sin @return fue rechazada con controles habilitados.
Doxyfile parcial en docs/doxygen/; ampliar INPUT por módulo al documentar el resto.

No hay heap, estado global mutable, periféricos ni libc en el módulo ARM.
Objeto ARM de Procesamiento a -O0: text 740 bytes, data/bss 0; sin undefined symbols.
Contexto ARM: 40 bytes en fixture. Stack-usage estático -O0: Inicializar 24, Actualizar
48, helpers 16/16/24/16 bytes; camino Actualizar→Invalidar hasta 72 bytes, sin ISR.
Es una estimación estática de esas llamadas, no medición del máximo de pila del sistema.
Fixture con API retenida: huella flash 1676 bytes, SRAM 2088 (40 contexto + 2048 pila).
Firmware mínimo sigue 844/840 bytes flash Debug/Release y 2048 de pila reservada:
--gc-sections descarta Procesamiento al no usarse todavía desde main. El fixture
se agregó para que esa eliminación no oculte problemas de enlace de la API.

## Fallos/limitaciones de herramientas registrados

- LeakSanitizer falló inicialmente bajo ptrace del sandbox (06-san-test.log). La
  misma suite pasó fuera del sandbox con todos los detectores habilitados
  (12-san-unsandboxed.log); no se desactivó detección de fugas.
- Doxygen 1.9.8 en modo sólo XML avisaba falsamente que faltaba @return, incluso
  apareciendo en XML. Se reprodujo con fuente mínima (13/15-doxygen*). HTML+XML
  reconocen la documentación; controles estrictos permanecen habilitados. Se eligieron
  etiquetas automáticas inglesas por el aviso del traductor español instalado.
  Evidencia final: 17-doxygen-final.log, 18-doxygen-xml.log y sonda negativa 19-*.

## Revisión y límites de aceptación

Diff y dependencias revisados; Drivers, servicios, dispositivos y archivo B00 intactos
contra SHA-256 inicial. main sólo agrega documentación de retorno, sin cambiar código.
La secuencia se asignará por el integrador al recibir eventos, nunca por la PC ni al
sacarlos de cola. Su captura de 64 bits y llamadas deben serializarse al integrar ISR.
No se acreditan tiempos, boot físico, consumo real de pila, UART, PWM ni regulación.
Ningún bloqueo pendiente dentro del alcance B01; esperar aceptación antes de archivar.
