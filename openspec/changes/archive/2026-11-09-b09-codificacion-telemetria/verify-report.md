# Verificación B09 — 2026-11-09

**PASS para alcance lógico**; resultado pendiente de aceptación. Base main
98635281ac6f79204a01dcf2946c92479e4824bf. Commit local autorizado, sin push;
asunto previsto `feat: codificar telemetria ASCII portable`. Delta candidato:
CT1–CT4 y modificación B02-R5; tareas 11/11 completas, revisión/índice preparados; contratos vigentes/archivos aceptados sin cambios.

## Ejecución y herramientas

Logs/comandos exactos/binarios fuera de Git: /tmp/ed3-b09-20261109-wdrfl0hk.
GCC host 13.3.0, GCC ARM 13.2.1, CMake 3.28.3, Python 3.12.3, Doxygen 1.9.8.
C11, -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes;
pruebas/fixture también -pedantic-errors. Sin relajar controles o instalar dependencias.

| Verificación ejecutada | Resultado | Evidencia externa |
|---|---|---|
| Consumidores antes de API | RED esperado en ambas suites por tipos/API ausentes | test_*.c.red.log |
| Mismos consumidores tras implementación | GREEN: 11 codificación + cuatro integración | test_*.c.green-build/run.log |
| Host Debug: build/CTest | 15/15; 161 casos C; 5096 pares codec contra libc y 11199 ternas B03-R1 | host-configure/build/ctest.log |
| ASan/LeakSanitizer/UBSan | 15/15, mismas cantidades, sin hallazgos | san-build-*.log, san-ctest.log, san-options.txt |
| ARM Debug: build/CTest | 12/12, incluidos 14 casos B00 y fixtures anteriores | arm-configure/build/ctest.log |
| Fixture B09 Release -O3 | Enlace, API retenida, ELF, sin indefinidos y GNU_STACK RW PASS | arm-b09-release.log |
| Doxygen estricto HTML/XML | 29 funciones/USW; .2.1.7.7/.8/.9/.10 documentadas, sin avisos | doxygen.log, doxygen-warnings.log, doxygen-audit.json |
| Mutante sin fracción de ancho fijo | Compiló; suite falló como se esperaba con decimales sin padding | mutante-decimales-build/run.log |
| Mutante capacidad sin reservar NUL | Compiló; suite falló como se esperaba en frontera N | mutante-capacidad-nul-build/run.log |
| Aislamiento y recursos ARM -O0 | Servicio sólo usa Servicios/Configuracion/std; sin indefinidos/libc nuevos | servicio-dependencias.log, arm-*-size/build.log, extra.json |

Sanitizadores ejecutados sin ptrace, por limitación conocida de LeakSanitizer:
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. Todos los controles habilitados;
sin fallos ambientales o funcionales pendientes. Doxygen se repitió al completar
@details de helpers; último cambio sólo documental, sin modificar cuerpos probados.

Reproducción: desde raíz R, crear O externo nuevo. Host/san:
`cmake -S "$R" -B "$O/host" -DTPI_BUILD_ARM=OFF -DCMAKE_BUILD_TYPE=Debug`,
`cmake --build "$O/host" --parallel 2`, `ctest --test-dir "$O/host" --verbose`.
San usa otro directorio, -DTPI_HOST_SANITIZERS=ON y opciones anteriores.
ARM configura -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Debug y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake", luego build/CTest.
Release ejecuta cmake -P cmake/verificar_enlace_b09.cmake con SOURCE_DIR,
OUTPUT_DIR, TOOL_PREFIX, PYTHON y BUILD_TYPE=Release. Doxygen usa Doxyfile vigente
con OUTPUT_DIRECTORY/WARN_LOGFILE externos; INPUT/avisos estrictos ya incluían Comunicacion.

## Matriz de cumplimiento

| Requisito / escenario | Prueba realmente ejecutada | Resultado |
|---|---|---|
| B02-R5/S9 | Regresión de recepción: 17 casos B02 originales, CTest host/san/ARM y Doxygen | PASS; delta sólo amplía alcance lógico |
| CT1/B09-S1 | EjemploYEstados, CerosYFracciones, OraculoNumerico | PASS host/san; trama/LF/NUL/longitud exactos, tres estados |
| CT1/B09-S2 | FronterasNumericas, CerosYFracciones, 5096 pares contra snprintf host | PASS; UINT32_MAX, INT32_MIN/MAX, negativos subunidad y >185 RPM |
| CT2/B09-S3 | Disponibilidad (nueve combinaciones), AusenciaErrorYCero | PASS; magnitudes no válidas ignoradas, ND/ERR distintos de cero |
| CT3/B09-S4 | ArgumentosNulosYPrioridad (siete combinaciones), EstadosInvalidos | PASS; punteros antes de enums y capacidad, limpieza permitida |
| CT3/B09-S5 | CapacidadesYCanarios (0..54), CapacidadExactaTramaCorta | PASS; N bytes fallan, N+1 entran, sin escrituras adicionales ante error |
| CT3/B09-S6 | RecuperacionYRepeticion, IndependenciaEInmutabilidad | PASS; sin retención, buffers independientes, entrada const preservada |
| CT4/B09-S7 | Cuatro casos integración B01/B03/B05/RX | PASS; manual/espera/activo, medidas genéricas, FIFO/parser/órdenes intactos |
| CT4/B09-S8 | Host/san, ARM, Doxygen, negativos, recursos y revisión de diff/preservación | PASS en cada modalidad; sin ejecución ARM/transmisión física |

11 casos codec + cuatro integración + 146 anteriores = 161 C. Los 15 CTest host
son 14 ejecutables C y el oráculo Python B03-R1 de 11199 ternas. Un caso codec
contiene 1000 fracciones y 4096 pares deterministas contra snprintf/enteros amplios
del host, algoritmo de formato independiente del conversor manual de producción.
Integración adapta contratos en pruebas/fixture; no incorpora ensamblado a main.
La consigna 185 RPM y medición 300 RPM del caso automático permanecen distintas.

## Recursos y revisión de diseño

Codec y tres helpers ARM -O0: .text 892 bytes; .rodata añadida 65; .data/.bss
añadidos cero. Las seis secciones .text de RX mantienen tamaño respecto de base
con mismos flags/header. No símbolos externos directos en el objeto; sin libc nueva.
Pila propia GCC: CodificarTelemetria 104, AgregarTexto 24, AgregarEntero 40,
AgregarFijo 32 bytes; ruta anidada máxima 176 antes del llamador/IRQ/otros módulos.
No es profundidad total de firmware ni garantía de tiempos reales.
ABI actual ARM: instantánea 12 bytes, resultado enum 1, size_t 4; no contrato portable de tamaño.
Fixture B09+B01/B03/B05/RX Debug: Flash 5572, SRAM local 2264 bytes;
Release: 4616/2264. Incluyen pila reservada 2048, no medida física.
Libgcc existente resuelve aritmética de Medicion_Velocidad del fixture.

Formato/cota acordados: 4+11+6+12+8+11+1=53 bytes, más NUL=54. Literales internos,
enums cerrados y fracciones 2/3 posiciones acotan helpers. Sin texto externo arbitrario.
Magnitud de INT32_MIN se calcula por conversión/resta unsigned definida, sin negación
con signo, flotantes o overflow. El destino sólo recibe la trama si capacidad>longitud;
no suma cantidades a capacidad arbitraria. Memoria válida, no solapada y serializada
es precondición del llamador. El servicio no retiene punteros ni requiere init RX.

Diseño respetado: DTO/enums propios, longitud LF incluido/NUL excluido, prioridad y
limpieza de errores, datos const y recepción separada. No se cambia Procesamiento,
otros servicios, dispositivos, Drivers, pruebas anteriores, startup/linker ni contratos
vigentes/archivos aceptados; comprobación por snapshot/hash y diff. Cuerpos, tipos,
valores de enums/constantes RX y documentación de sus USW se preservan exactamente.
Sólo se amplían descripción del módulo, include size_t y API nueva en Comunicacion.

Sin incidencias críticas o desviaciones pendientes. No hubo ejecución ARM, simulación
de MCU, placa, envío UART, flasheo, instalación ni cambios sensibles MCU. La capa de
aplicación reunirá datos y llamará al servicio en un bloque posterior; codificación
no acredita entrega en PC. UART/ISR/DMA/TX/periodicidad/main y validación eléctrica/
temporal permanecen pendientes. Diagramas finales diferidos según CONTEXTO.
Próxima acción: revisar resultado B09. Sin push ni sincronizar/archivar hasta orden.
