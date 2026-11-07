# Verificación B04 — 2026-11-06

Resultado: **PASS para el alcance lógico acordado**; resultado pendiente de aceptación
del usuario. Base main: db0c305b8f2533884c00e416f52c3dd75acf652b. Cierre con asunto
`feat: implementar B04 de normalizacion del mando manual`; hash consultable en Git.
Commit autorizado, sin push. Se incorpora propuesta/contexto locales previos.

## Completitud y contrato

13 tareas técnicas/de cierre; código, pruebas, documentación y revisión completos.
Mandos (ESW.2.1.5, anexo B del anteproyecto, p.5 revisada en texto e imagen) implementa
Inicializar (USW.2.1.5.1) y NormalizarManual (USW.2.1.5.2). Entradas uint32_t lectura
y fondo de escala; salida uint16_t 0..1000 por mil, redondeo cercano/empate arriba.
Estado explícito SIN_DATOS/VALIDO/ERROR_ARGUMENTO/ERROR_ESCALA/ERROR_LECTURA.
Prioridad: NULL, escala cero y lectura excesiva. Error con salida escribible borra
valor previo y permite recuperación con la siguiente entrada válida. Sólo consumir VALIDO.

TPI_MANDO_MANUAL_MAX=1000 tiene una fuente común en Configuracion/Limites.h;
Mandos y PROCESAMIENTO_MANDO_MAX usan aliases. B01 mantiene valor, API y comportamiento.
Servicios sin dependencia de Aplicacion; la adaptación a B01 vive sólo en prueba/fixture.
No hay heap, flotantes, globals mutables, acceso físico, esperas, selector ni PWM.
Captura coherente, errores previos del ADC y acceso serializado son responsabilidad
del futuro integrador. No se detecta un ADC defectuoso que entregue valores dentro de rango.

## Matriz de aceptación

Todas las funciones de prueba siguientes se ejecutaron en tests/host/test_mandos.c.

| Requisito / escenario | Prueba ejecutada | Resultado |
|---|---|---|
| MA1 / S1 extremos/recorrido | ExtremosDeEscalas, MitadDelRecorrido, RecorridosMonotonos | PASS |
| MA1 / S2 equivalencia/redondeo | EscalasEquivalentes, RedondeoYEmpates, oráculo de distancias en recorridos/barrido | PASS |
| MA2 / S3 ausencia/reinicio | AusenciaInicialYCeroValido, Reinicio | PASS |
| MA2 / S4 invalidación/recuperación | EscalaCeroInvalida, LecturaExcesivaInvalida, RecuperacionYCambioDeEscala | PASS |
| MA2 / S5 argumentos/fronteras | PunterosNulosYPrioridad, ExtremosDeTiposYMatriz, BarridoDeterminista | PASS |
| MA3 / S6 aislamiento/B01 | InstanciasIndependientes, AdaptacionManualB01; revisión de dependencias | PASS |
| MA4 / S7 verificación | CTest host/ARM, sanitizadores, fixture Release y Doxygen descritos abajo | PASS |

15 casos B04; 100 combinaciones de fronteras, 10 000 pares deterministas y 71 680
puntos de recorrido completo en escalas 1/3/16/1000/1023/4095/65535. El oráculo comprueba
distancia entera al valor exacto, empate hacia arriba, errores y rango; no recalcula
el cociente de producción. Casos explícitos incluyen 341/1023=1365/4095 →333,
682/1023=2730/4095 →667, 1/16 →63; 1/2000 →1 y 1/2001 →0 válidos.
UINT32_MAX/UINT32_MAX →1000, 1/UINT32_MAX →0; MAX/(MAX-1) se rechaza.
Recorridos verifican monotonía, rango y redondeo en cada punto. Integración host B01
entrega fuente MANUAL con 0/500/1000; cero válido se distingue de ausencia/error.

## Comandos y resultados reales

Desde raíz; OUT=/tmp/ed3-b04-20261106-01ljgnod. Herramientas existentes: GCC host 13.3.0, GCC ARM 13.2.1,
CMake 3.28.3 y Doxygen 1.9.8. Advertencias como errores, C11 sin extensiones en pruebas.

| Comando/configuración ejecutados | Resultado |
|---|---|
| `cmake -S . -B "$OUT/host/build" -DTPI_BUILD_ARM=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug`; build; `ctest --test-dir "$OUT/host/build" -V --output-on-failure` | CTest 4/4; B01 17 + B02 17 + B03 15 + B04 15 = 64 casos |
| Mismo flujo en `$OUT/san/build` agregando `-DTPI_HOST_SANITIZERS=ON` | 64 casos, sin hallazgos ASan/LeakSanitizer/UBSan |
| Sanitizadores fuera de sandbox/ptrace, `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest ...` | Controles completos, sin suprimir errores |
| ARM Debug en `$OUT/arm-debug/build`: `-DTPI_BUILD_ARM=ON -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake" -DCMAKE_BUILD_TYPE=Debug`; build/CTest | 6/6; B00 incluye 14 casos y fixtures B01–B04 |
| ARM Release build y `ctest --test-dir "$OUT/arm-release/build" -R b04.enlace_arm -V --output-on-failure` | API Mandos/B01 retenida y enlace -O3 correcto |
| `doxygen -` con Doxyfile versionado + OUTPUT_DIRECTORY temporal; inspección XML | Sin avisos; 2 USW Mandos, 17 funciones totales; límite común/aliases presentes |
| `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=c11 -O0 -ffreestanding -fstack-usage -c Servicios/Mandos.c -IServicios -IConfiguracion`; size/nm/objdump | Recursos abajo; sin global mutable |
| `git diff --check`, revisión de diff/índice y SHA-256 contra inventario inicial | Sin cambios ajenos; preservación y formato correctos |

Sin fallos de build, pruebas o documentación observados. Umbral de cobertura no
configurado; no se ejecutó medición de cobertura. No hay CI incorporado.
Logs completos de comandos/resultados fuera del árbol versionado, bajo OUT.

## Aritmética, enlace y recursos estáticos

Escala>0 y lectura<=escala se comprueban antes de dividir. Producto de 64 bits más
redondeo <=4 297 114 778 647 <UINT64_MAX. Cociente redondeado 0..1000: estrechamiento
a uint16_t seguro por esa cota. No hay saturación ni división por cero.
Libgcc del toolchain ya integrado en B03 resuelve __aeabi_uldivmod; sin dependencias
nuevas ni libc. Fixture comprueba símbolos de las dos USW Mandos y las dos API B01,
auxiliar de división, ELF B00 y GNU_STACK RW; `nm -u` vacío en Debug y Release.

| Recurso ARM | Resultado |
|---|---|
| Mandos_MandoManual | 4 bytes, sin buffers ni heap; tamaño del ABI utilizado |
| Objeto Mandos -O0 | .text 292 bytes, .data/.bss 0; único auxiliar pendiente antes de enlace: __aeabi_uldivmod |
| Stack propio -O0 (-fstack-usage) | Inicializar 16 bytes; NormalizarManual 88 bytes |
| Cadena interna -O0 (disassembly) | 88 + 16 (__aeabi_uldivmod) + 32 (__udivmoddi4) = hasta 136 bytes; excluye llamador/IRQ |
| Fixture Mandos+B01 Debug / Release | Huella flash 2820 / 2216 bytes; SRAM local 2104 / 2100, incluidos datos/padding y pila reservada 2048 |
| main mínimo Debug / Release | Flash 844 / 840 bytes, SRAM 2048; API sin integrar eliminada por GC, por eso existe el fixture |

No hay recursión ni llamadas propias adicionales en Mandos. Tamaños y pila se inspeccionaron
estáticamente; no acreditan profundidad real, tiempo de ejecución o comportamiento eléctrico.
La propiedad GNU_STACK es metadata ELF y no configura protección física del MPU.

## Revisión y límites

Diseño seguido sin desviaciones. Preservados main, Procesamiento.c, Comunicacion,
Medicion_Velocidad, drivers/dispositivos, pruebas previas, contratos vigentes y archivos
archivados. Sólo header de B01 cambia a alias numéricamente idéntico; regresiones pasan.
USW con parámetros, retornos, unidades, errores y concurrencia Doxygen; sin comentarios
dentro del cuerpo de funciones de producción. Sin instalaciones ni cambios sensibles.
Propuesta previa y contexto locales incorporados al bloque.

Contrato candidato y seguimiento B04 quedan activos, sin sincronizar ni archivar antes
de aceptación del resultado. Commit local autorizado, sin push. Próxima acción: aceptar
B04; luego sincronizar/archivar sólo por indicación. No iniciar B05 automáticamente.
Sin ejecución ARM, simulación ni hardware: ADC/DMA, selector/antirrebote, PWM, pines,
VREF, reloj y recorrido físico del potenciómetro siguen pendientes.
