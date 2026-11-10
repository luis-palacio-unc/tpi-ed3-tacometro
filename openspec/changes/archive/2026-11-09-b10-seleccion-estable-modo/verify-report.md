# Verificación B10 — 2026-11-09

**PASS para alcance lógico**; resultado pendiente de aceptación. Base main
9f53e73e0841123cfba29ec3dbbbb2bab567cf03. Commit local autorizado sin push;
asunto previsto `feat: filtrar seleccion de modo con tiempo externo`.
Tareas 11/11 completas; revisión/índice preparados.
Delta candidato SE1–SE4 y MA3; contratos vigentes/archivos aceptados sin cambios.

## Ejecución

Logs, comandos exactos y artefactos fuera de Git: /tmp/ed3-b10-20261109-1jdibt8h.
GCC host 13.3.0, GCC ARM 13.2.1, CMake 3.28.3, Python 3.12.3 y Doxygen 1.9.8.
C11 con -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes;
consumidores/fixture también -pedantic-errors. Sin instalar ni relajar controles.

| Control ejecutado | Resultado | Evidencia externa |
|---|---|---|
| Consumidores base antes de API | RED esperado, tipos/API ausentes | test_*.c.red.log |
| Consumidores base después de implementar | GREEN: 14 selector + cuatro integración | test_*.c.green-build/run.log |
| Host Debug: build/CTest completo | 17/17; 179 C, historial 4096/400 rollovers, codec 5096 y velocidad 11199 | host-configure/build/ctest.log |
| ASan/LeakSanitizer/UBSan completo | 17/17, mismas cantidades sin hallazgos | san-build-*.log, san-ctest.log, san-options.txt |
| Refuerzo de pruebas temporales posterior | Suite afectada recompilada: host/san 1/1, 14 casos y mismo historial | frontera-host/san-build/ctest.log |
| ARM Debug: build/CTest | 13/13, con 14 casos B00 y fixtures previos | arm-configure/build/ctest.log |
| Fixture B10 Release -O3 | API retenida, ELF válido, sin indefinidos, GNU_STACK RW | arm-b10-release.log |
| Doxygen HTML/XML estricto | 32 funciones/USW; nuevas .2.1.5.3/.4/.5, sin avisos | doxygen.log, doxygen-warnings.log, doxygen-audit.json |
| Mutante sin reinicio de candidato | Compiló; suite detectó confirmación espuria durante rebote | mutante-sin-reinicio-candidato-build/run.log |
| Mutante con confirmación tardía | Compiló; suite detectó error en frontera exacta | mutante-frontera-tardia-build/run.log |
| Recursos/aislamiento | Servicio sólo necesita Servicios/Configuracion/std; B04 preservado | servicio-dependencias.log, arm-*-size.log, extra.json |

Sanitizadores se ejecutaron sin ptrace, con ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
y UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. Todos habilitados; sin errores
funcionales/ambientales pendientes. Tras el pase completo se ampliaron sólo las
aserciones de ArgumentosYPrioridades: fuente desconocida no acepta timestamp;
fuente ausente/fallida y modo inválido sí conservan tiempo aceptado. Se recompiló
y repitió únicamente b10.selector en host/san; código productivo sin cambios.

Reproducción: desde raíz R y carpeta externa O, host/san configura
`cmake -S "$R" -B "$O/host" -DTPI_BUILD_ARM=OFF -DCMAKE_BUILD_TYPE=Debug`,
build `cmake --build "$O/host" --parallel 2` y `ctest --test-dir "$O/host" --verbose`.
San usa directorio independiente y -DTPI_HOST_SANITIZERS=ON, con opciones anteriores.
ARM configura -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Debug y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake", luego build/CTest.
Release usa cmake -P cmake/verificar_enlace_b10.cmake con SOURCE_DIR, OUTPUT_DIR,
TOOL_PREFIX, PYTHON y BUILD_TYPE=Release. Doxygen usa configuración vigente con
OUTPUT_DIRECTORY/WARN_LOGFILE externos; INPUT/controles ya incluían Mandos.

## Cumplimiento por escenario

| Requisito / escenario | Prueba realmente ejecutada | Resultado |
|---|---|---|
| MA3/S6 | Regresiones B04/B01/B08, InstanciasIndependientes, TransicionConfirmada | PASS, normalización y composición disponibles sin dependencia inversa |
| SE1/B10-S1 | ArranqueManual, ArranqueAutomatico | PASS; primer candidato, intervalo-1 e intervalo exacto |
| SE1/B10-S2 | RebotesAmbosSentidos, CeroYMarcasRepetidas | PASS; conservar confirmado, reiniciar candidato, cero inmediato |
| SE2/B10-S3 | RetrocesoYNuevaEpoca, RolloverTemporal, IntervaloMaximoYSemirrango, historial | PASS; iguales/wrap/semirrango/error y época nueva sin modo viejo |
| SE3/B10-S4 | AusenciaExigeEstabilidadNueva, FalloFuenteExigeEstabilidadNueva, ModoInvalidoYRecuperacion, ArgumentosYPrioridades reforzado | PASS; NULL/prioridades/anclaje, NINGUNO y recalificación |
| SE3/B10-S5 | ConfiguracionYReinicio, InstanciasIndependientes y máximos | PASS; configuración excesiva persiste hasta reinicio válido, independencia |
| SE4/B10-S6 | Cuatro casos integración + host/san/ARM/Doxygen/revisión | PASS por modalidad; transición confirmada, cero/nueva consigna, auto activo estable y órdenes independientes del rollover |

14 casos selector + cuatro integración + 161 anteriores = 179 C. Los 17 CTest
host incluyen 16 ejecutables C y el oráculo Python B03-R1. OraculoHistorial compara
4096 muestras con 400 rollovers: referencia por historial completo, búsqueda del
inicio del tramo y tiempo absoluto uint64_t, sin copiar metadatos incrementales del
servicio. Configuraciones 0/1/20/INT32_MAX ms, saltos 0/1/4/19/INT32_MAX y pérdidas
de fuente. Regresiones conservan 5096 pares codec y 11199 ternas velocidad.

## Recursos y revisión

Tres funciones nuevas ARM -O0: .text 398 bytes, .data/.bss añadidos cero.
Pila propia: inicializar 16, actualizar 24, InvalidarSelector 16; máxima anidada
40 antes del llamador/IRQ/otros módulos. Contexto 16 bytes, enum estado 1 con ABI
actual, sin prometer tamaño portable. No bucles, heap, reloj propio o globals mutables.
Las dos secciones .text de B04 conservan tamaño; único indefinido del objeto sigue
siendo __aeabi_uldivmod de la normalización previa, sin dependencia nueva.
Fixture B10+B04/B01/B08 Debug: Flash 3628, SRAM local 2128 bytes;
Release: 2776/2124. Incluyen pila reservada 2048; no miden profundidad real.
APIs de selector/B04/B01/B08 y proveedor emulado retenidos con --gc-sections;
libgcc existente, sin libc, indefinidos de enlace o pila ejecutable.

Diseño respetado: intervalo configurado sin default TPI, modo NINGUNO inicial,
candidato/confirmado separados, resta modular unsigned y prioridad documentada.
Pérdidas de fuente/modo descartan confirmación y conservan último timestamp aceptado;
error temporal descarta anclaje. Configuración inválida requiere reinicio válido.
Contexto inicializado/vivo y acceso serializado son precondiciones; no detecta falta
de llamadas, rebotes entre muestras o vueltas completas omitidas del contador.

Cuerpos/tipos/constantes y documentación de las dos USW B04 preservados byte por
byte; descripción del módulo ampliada. Aplicacion/Procesamiento/main, Driver_Motor,
otros servicios, dispositivos, Drivers, pruebas previas, startup/linker y contratos/
archivos aceptados intactos según snapshot/hash/diff. Única composición en tests/fixture.
Sólo transiciones confirmadas manual→auto crean entrada automática B01; durante
indisponibilidad, adaptador de prueba conserva diagnóstico sin nuevas órdenes.
La política física de fallo y reentrada de aplicación tras pérdida no se define aquí.

Sin incidencias críticas o desviaciones pendientes. No hubo ejecución ARM, simulación
MCU, placa, flasheo, instalación o cambios sensibles MCU. GPIO/polaridad, reloj/timer,
muestreo físico y main siguen pendientes; pruebas host no acreditan antirrebote
eléctrico o tiempos reales. Diagramas finales diferidos según CONTEXTO.
Próxima acción: revisar resultado B10. Sin push ni sincronizar/archivar hasta orden.
