# Verificación B00 — 2026-11-06

## Resultado

PASS para el alcance acordado de compilación ARM y verificaciones estáticas.
Implementación completa. Resultado pendiente de aceptación del usuario; no archivado.
Sin commits/push. No acredita ejecución del startup en MCU, temporización ni electricidad.

## Entorno y ejecución real

Proyecto en ruta con espacios; GCC ARM 13.2.1, binutils 2.42, GCC host 13.3.0,
CMake 3.28.3 y Python 3.12.3. Sin instalar dependencias.
Logs y artefactos fuera del árbol versionado: `/tmp/ed3-b00-20261106-yyj_xnlc`.

Comandos ejecutados (desde raíz; reemplazar `<logs>` por la ruta anterior):

```sh
cmake -S . -B <logs>/arm-debug -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake"
cmake --build <logs>/arm-debug --parallel 2
ctest --test-dir <logs>/arm-debug --output-on-failure -V
cmake -S . -B <logs>/arm-release -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake"
cmake --build <logs>/arm-release --parallel 2
cmake --list-presets=all
```

Ambas configuraciones parten de directorios inicialmente vacíos. El build Debug se
repitió tras las correcciones descritas abajo; Release se ejecutó después de ellas.
La suite configura/compila host, fixtures ARM y configuraciones negativas en temporales.
Se inspeccionaron con readelf/objdump los atributos, segmentos, símbolos y desensamblado.

## Matriz de escenarios

| Requisito/escenario | Evidencia ejecutada | Resultado |
|---|---|---|
| R1/S1 ARM limpio | Configure/build Debug y Release, post-build verificar_elf.py | PASS |
| R1/S2 Toolchain inválido | test_toolchain_ausente, test_directorio_toolchain_invalido | PASS: rechazo esperado |
| R2/S3 Imagen válida | b00.imagen, test_imagen_valida; checksum, CRP, regiones y símbolos | PASS |
| R2/S4 Imagen corrupta | ELF truncado/no ARM, checksum, vector fuera de flash, pila desalineada, CRP, entry | PASS: rechazos esperados |
| R2/S5 Límites de enlace | test_desborde_sram, test_main_ausente | PASS: errores reales de linker |
| R3/S6 Ruta de arranque | test_fixture_data_bss_y_llamada_main; revisión manual de bucles y límites | PASS estático; runtime no probado |
| R4/S7 Host | test_objetos_host, 18 objetos y ningún startup ARM | PASS |

CTest Debug: **2/2** pruebas (b00.imagen y b00.contratos); la segunda ejecuta **14/14**
casos unittest. Sin omitidos. No son pruebas funcionales del tacómetro ni ejecución ARM.
No se calculó cobertura de aplicación: no corresponde a este bloque de infraestructura.

## Memoria y arranque

| Perfil | Huella flash | SRAM local asignada | Reserva de pila |
|---|---:|---:|---:|
| Debug | 844 bytes | 2048 bytes | 2048 bytes |
| Release | 840 bytes | 2048 bytes | 2048 bytes |

Flash tiene 512 kB, SRAM local 32 kB; AHB0/AHB1 separadas y sin asignación en B00.
La huella incluye el hueco de flash hasta .text; difiere de la suma text de GNU size.
La reserva de pila no es medición de máximo consumo; no hay heap.
Startup revisado: comparación previa al acceso en rangos vacíos, avance de palabra,
.data LMA→VMA, borrado .bss, llamada main y bucle si retorna. Handlers débiles por defecto.
No llama SystemInit ni escribe PLL, clocks, flash timing, NVIC o periféricos.

## Fallos encontrados y correcciones

1. Primer enlace falló con --fatal-warnings por ausencia de .note.GNU-stack en objetos
   C. Se declaró noexecstack en ensamblado; se mantuvieron los controles del linker.
2. La primera suite rechazó un fixture válido por usar nombre CPU descriptivo distinto.
   Se verifican atributos semánticos v7/Microcontroller/Thumb-2, no ese nombre.
3. La prueba añadida de inmutabilidad reprodujo que objcopy reescribía la fecha del ELF.
   La extracción usa salida temporal explícita; el test ahora comprueba bytes y fecha.

Logs de fallos: 02-build-debug.log, 04-ctest-debug.log y 05-readonly-reproduction.log.
Resultados corregidos: 06-build-debug.log, 07-ctest-debug.log y 09-build-release.log.
No se ocultaron advertencias ni se desactivaron pruebas para aprobar.

## Revisión y límites

Drivers/ y los módulos de Servicios/Dispositivos conservan los hashes iniciales.
Sólo main cambia en la aplicación; no implementa regulación ni servicios.
Git en main sin HEAD de commit ni índice preparado. Diff revisado contra snapshot
inicial, no sólo git diff (los archivos son aún untracked).
Ninguna ejecución de placa/simulador, flasheo o prueba heredada de periféricos.
PCB/silicio y entorno de depuración real pendientes. Sin bloqueos dentro del alcance
acordado; pendiente revisión del usuario antes de sincronizar specs/archivar.

SHA-256 ELF Debug: `76f34bede6c540f65e67c28a19a8fa6bd5ff8f1adc0aac4db9c7a5594f0d0401`.
SHA-256 ELF Release: `44dd183eae7bfba9ac2218eb5dec0338e9febf63fe8949112957f72c41991320`.

## Aceptación posterior

El usuario aceptó B00 el 2026-11-06 y autorizó sincronizar, archivar y publicar
la estructura inicial. Las observaciones de Git/sin commits y aceptación pendiente
del cuerpo del informe describen el instante de verificación anterior a este cierre.
La aceptación se limita al alcance verificado; no acredita funcionamiento físico.
