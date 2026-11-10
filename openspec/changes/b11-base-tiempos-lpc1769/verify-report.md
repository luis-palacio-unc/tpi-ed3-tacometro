# Verificación B11 — 2026-11-10

**PASS para alcance host/compilación**; resultado pendiente de aceptación.
Alcance acordado el 2026-11-09; base main b4981c92a300243c8b7d960d09a99aa81c1ce300.
11/11 tareas completas, revisión/índice preparados para commit local autorizado,
asunto `feat: agregar base temporal SysTick con reloj explicito`. Sin push autorizado.
Candidato BT1–BT4/S1–S8; contratos vigentes/archivos aceptados sin cambios.

## Ejecución y reproducción

Logs, comandos exactos y artefactos externos: /tmp/ed3-b11-20261109-br0hi5vv.
El directorio conserva fecha de inicio del bloque. GCC host 13.3.0, GCC ARM 13.2.1,
CMake 3.28.3, Python 3.12.3 y Doxygen 1.9.8; versiones en versiones.log.
C11 con -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes;
pruebas/fixture además -pedantic-errors. Sin instalar ni relajar controles.

| Control ejecutado | Resultado | Evidencia externa |
|---|---|---|
| Consumidor antes de implementar | RED esperado por API/puerto ausentes | red-build.log, red-command.json |
| Tres suites nuevas independientes | GREEN: 18 base + cuatro integración + cuatro puerto | *-green-build/run.log |
| Host Debug completo | 20/20; 205 C; selector 4096 muestras/400 rollovers, codec 5096 pares y velocidad 11199 ternas | host-configure/build/ctest.log |
| ASan/LeakSanitizer/UBSan completo, sin ptrace | 20/20, mismas cantidades, sin hallazgos y fugas habilitadas | san-ctest-sin-ptrace.log, san-options.txt |
| ARM Debug build/CTest | 15/15, con controles previos (B00 conserva sus 14 casos), vector y fixture nuevos | arm-configure/build/ctest.log |
| Fixture B11 Release -O3 | ELF/API/vector válidos, sin indefinidos ni pila ejecutable | arm-b11-release.log |
| Doxygen estricto HTML/XML | 40 funciones/USW, ocho nuevas .2.1.17.1–.8; sin avisos | doxygen.log, doxygen-warnings.log, doxygen-audit.json |
| Mutante sin cancelar pendiente | Compiló; PendienteAntiguoReinicio lo rechazó | mutante-sin-limpieza-pendiente-build/run.log |
| Mutante con recarga fuera de uno | Compiló; FrecuenciasValidas lo rechazó | mutante-recarga-fuera-uno-build/run.log |
| Vector por defecto débil | Verificador B11 lo rechazó correctamente | vector-debil-negativo.log |
| Recursos/preservación | Estado privado/puerto acotados, módulos previos intactos | recursos.json, size-*.log, inspeccion-*.txt, preservacion.json |

Primer Doxygen rechazó @return en funciones void; se trasladó la explicación a
@details, conservando todos los controles. Evidencia en primer-doxygen*.log.
Primer intento instrumentado terminó con error ambiental LeakSanitizer/ptrace;
se conservan san-ctest.log y san.json. Se repitió fuera de ptrace con
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 y
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1, sin desactivar controles.
La inspección auxiliar se adaptó a PRIMASK en mayúsculas de objdump; no cambió
firmware. No quedan errores funcionales/documentales/ambientales pendientes.

Reproducción desde raíz R y carpeta externa O: configurar host con
`cmake -S "$R" -B "$O/host" -DTPI_BUILD_ARM=OFF -DCMAKE_BUILD_TYPE=Debug`,
compilar `cmake --build "$O/host" --parallel 2` y
`ctest --test-dir "$O/host" --verbose`. San usa carpeta independiente y
-DTPI_HOST_SANITIZERS=ON, opciones anteriores y proceso sin ptrace.
ARM usa -DTPI_BUILD_ARM=ON y -DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake";
build/CTest idénticos. Release: cmake -P cmake/verificar_enlace_b11.cmake con
SOURCE_DIR, OUTPUT_DIR, TOOL_PREFIX, PYTHON y BUILD_TYPE=Release.
Doxygen usa configuración vigente con OUTPUT_DIRECTORY/WARN_LOGFILE externos.
Comandos exactos de cada invocación en *.json/log; recursos con size/nm/objdump y
verificar_elf.py. Controles negativos modifican únicamente copias en el directorio externo.

## Matriz de cumplimiento

| Requisito/escenario | Prueba realmente ejecutada | Resultado |
|---|---|---|
| BT1/S1 | ConfiguracionYOrden, FrecuenciasValidas/Invalidas | PASS: 2000/120000000, LOAD mínimo/máximo, cero/no múltiplos/excesos, sin habilitación inválida |
| BT1/S2 | ErrorInvalidaEpoca, Recuperacion, ReinicioDesdeActiva/Detenida | PASS: borra época anterior, conserva error hasta reinicio válido |
| BT2/S3 | SinInicializar, NullSinEfectos | PASS: ausencia distinta de cero activo; NULL sin puerto/efectos propios |
| BT2/S4 | ParadaCongela, ParadaIdempotente, PendienteAntiguoParada | PASS: contador congelado, cancelación e idempotencia |
| BT3/S5 | TicksAtendidos, Rollover, SinInicializar/ParadaCongela, TicksColapsadosNoSeRecuperan | PASS: incremento activo modular; no avanza detenido/fallido, limita semántica a ticks atendidos |
| BT3/S6 | PreservaMascara, InstantaneaConTickAlSalir, PendienteAntiguoReinicio/Parada, Criticas del puerto | PASS: máscaras 0/1, snapshot antes de tick al restaurar, sin pendientes antiguos |
| BT4/S7 | FronteraManual, FronteraAutomaticaYRebote, DisponibilidadDeBase, ReinicioDeEpocaYConsumidor | PASS: frontera exacta y consumo sólo ACTIVA; reinicia consumidor entre épocas |
| BT4/S8 | MapaRegistros, IcsrYPrioridad, RegistroDesconocido; host/san/ARM/Doxygen/diff | PASS por modalidad; máscaras/orden y registros ajenos, vector fuerte y API retenida |

179 C anteriores + 18 base + cuatro integración + cuatro puerto = 205 casos C.
20 CTest host son 19 ejecutables C y oráculo Python B03-R1. Regresiones mantienen
referencias independientes B03-R1/B09/B10 y sus cantidades anteriores.
La siembra de contador cerca de UINT32_MAX se limita al test que incluye el .c;
no agrega API/condicional de prueba a producción. Puerto real probado en host
contra dobles CMSIS y compilado aparte con encabezados CMSIS reales en ARM.
El modelo host no simula CPU, bus, latencias, oscilador ni condiciones eléctricas.

## Recursos y revisión

Objetos ARM -O0: .text propia base 358 bytes, puerto 194 más NVIC_SetPriority
existente 84 (total 636); .data cero, .bss privada 5 bytes antes de alineación.
Máxima cadena propia de pila 64 bytes (inicializar 32 + puerto escribir 16 +
NVIC_SetPriority 16), excluye llamador/IRQ/otros. ISR usa 4 bytes propios Debug y
cero Release; falta sumar apilado de excepción ARM (al menos ocho palabras),
alineación y posibles interrupciones anidadas. No acredita profundidad real total.
Release: cadena propia máxima 24 bytes, antes de llamador/IRQ/otros.
Lectura ocupa 8 bytes con ABI actual; enum de estado 1, sin garantía portable.
Fixture B11/B10 Debug: Flash 2000 y SRAM local 2088 bytes; Release: 1588/2088,
ambos incluyen reserva de pila 2048. Imagen main mínima Debug: 884/2056.

Disassembly Debug/Release confirma MRS PRIMASK→CPSID→DSB/ISB y DSB→MSR PRIMASK→ISB;
handler sin llamadas/bucles, sólo consulta estado e incremento. Escritura ICSR
directa, no read-modify-write; prioridad sólo SysTick. Barreras de compilador
suplen clobber memory ausente en intrínsecos CMSIS antiguos, sin editar Drivers.
Base sólo depende de su puerto; objeto de puerto sin indefinidos, sin heap/libc
nueva ni reloj asumido. Fixture conserva APIs B11/B10 y GNU_STACK RW.
main/startup/linker, servicios/aplicación, bibliotecas, tests previos, contratos
vigentes y archivos aceptados intactos según snapshot/hash. CMake/Doxyfile sólo
agregan soporte/pruebas/documentación; código probado igual tras corregir comentarios.

## Límites y siguiente acción

Resultado listo para revisión, no aceptado/sincronizado/archivado todavía. Sin push.
No hubo ejecución ARM, simulación MCU, placa, flasheo, instalación o cambio sensible
del MCU. B11 no configura reloj de CPU ni inicializa el periférico desde main.
Frecuencia física, latencia de IRQ, precisión, suspensión/debug y prueba en placa
pendientes. Múltiples ticks colapsados no se detectan/recuperan; servicios y aplicación
siguen independientes de SysTick. No implementa ventana RPM de 1 s ni adquisición.
ESW.2.1.17 es extensión local de soporte, no requisito nuevo del anexo B.
Diagramas finales diferidos según CONTEXTO. Próxima acción: revisar/aceptar B11;
sin promover contratos ni publicar hasta orden explícita.
