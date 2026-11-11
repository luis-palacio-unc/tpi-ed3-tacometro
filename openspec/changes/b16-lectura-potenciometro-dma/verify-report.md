# Verificación B16 — muestra propiedad de Potenciometro

Actualizado2026-11-11; verificaciones ejecutadas2026-11-10.
main, base de esta corrección24adfe3. Usuario ordena mover ADC_DMA_Pot
a Potenciometro.c y actualizar especificaciones/Doxygen/pruebas/commit sin push.
Implementado/verificado; B16 sigue pendiente de aceptación del resultado.
Specs principales/archivos aceptados no se sincronizan ni archivan todavía.

## Cambio y revisión

ADC_DMA_Pot tiene una sola definición volatile uint32_t en Potenciometro.c.
Adquisicion_ADC_DMA.h conserva el extern interno para que el soporte configure
su dirección como destino DMA al integrar. Adquisicion_ADC_DMA.c no define datos.
Pot_Read(void), USW.2.1.12.1, conserva exactamente su cuerpo/API y retorno0..4095
mediante ADC_GDR_RESULT original; sin validadores, estado o inicialización añadidos.
Propiedad de la variable documentada con Doxygen en definición y declaración.

La prueba CMake enlaza test_potenciometro.c y Potenciometro.c, sin el .c de soporte.
El fixture escribe mediante extern como productor de prueba. Se conservan los
tres casos de memoria: referencias0/2048/4095, actualización/repetición y extracción
de RESULT sin modificar la palabra. No se agregan casos que sólo repitan el código.

## Evidencia ejecutada tras el traslado

Directorio externo: `/tmp/ed3-b16-propiedad-pot-wve2trjj`. Snapshot antes de editar: before.json;
comandos y retornos reales en host/san/arm/extras/preservation-commands.json.
Modo estándar; GCC host13.3.0, ARM13.2.1, Doxygen1.9.8; C11 estricto -Wall -Wextra
-Werror -Wconversion -Wshadow -Wstrict-prototypes; tests/objetos además -pedantic-errors.

| Modalidad/control | Resultado actual | Logs externos |
|---|---|---|
| B16 individual | 1/1 suite,3/3 casos, enlazados sin soporte | b16-build.log,b16-tests.log,link-pure-pot.txt |
| Host completo | 22/22 suites,207 casos C y oráculo Python | host-*.log |
| ASan/LSan/UBSan | 22/22 sin hallazgos; detect_leaks=1 | san-*.log |
| ARM Debug | Build y13/13 controles previos de imagen/enlace; no ejecución | arm-*.log |
| Objetos ARM O0/O2 | ADC_DMA_Pot símbolo B de4 bytes sólo en Potenciometro; soporte sin símbolos/datos | *-symbols.log,*-size.log,pot-sections.log |
| Doxygen HTML/XML | 43 funciones/USW, variable documentada en Potenciometro.c; sin avisos | doxygen.log,extra-results.json |
| Preservación/diff | Drivers90 originales a21cc1e; módulos/plataforma/contratos aceptados intactos | preservation-results.json,diff-check.log |

Regresiones:11199 ternas velocidad contra oráculo,5096 pares codec y4096 muestras/
400 rollovers selector. Sin fallos ni pruebas omitidas; sin umbral de cobertura.

| Requisito/escenario | Evidencia ejecutada | Resultado |
|---|---|---|
| PD1/S1 | LecturasDeReferencia,0/2048/4095 | Conforme |
| PD1/S2 | ActualizacionYRepeticion, lectura actual sin escritura/caché | Conforme |
| PD2/S3 | SoloResultadoSinEscritura; test enlazado sin soporte; propiedad única en nm O0/O2 | Conforme |
| PD3/S4 | Host/san/ARM/Doxygen/diff/hashes, con modalidades separadas | Conforme por modalidad |

4/4 escenarios vigentes.12/12 tareas al cierre técnico/commit, incluida corrección.
Diseño actualizado por orden del usuario y seguido sin desviaciones. Sin hallazgos críticos.

## Recursos y pendientes

Potenciometro: .text32/16 bytes-O0/O2, .bss4 bytes alineada4, .data0; marco propio4/0.
Soporte: .text/.data/.bss0. Ambos objetos sin símbolos externos pendientes.
Memoria total reservada por la muestra se mantiene en4 bytes; getter sin cambios.
O2 sigue leyendo la palabra y extrayendo RESULT con LDR/UBFX, sin stores o llamadas.
No son cotas de pila global o tiempo real. Se mantienen18 objetos de firmware;
main mínimo elimina por GC módulos sin referencias; enlace/configuración global al final.

Configuración, conversión ADC y DMA reales pendientes. La integración garantizará
AD0.0/P0.23 función01, origen/canal correcto y transferencias32bits alineadas,
sin escritores CPU concurrentes. La prueba PC escribe RAM, no acredita ADC/DMA,
antigüedad, coherencia entre canales o comportamiento eléctrico. Cero de arranque
no indica disponibilidad. .bss local con MCU activo; revisar ubicación si se usa
Sleep (UM10360 §1.9/p12 y §4.8.1); secuencia ADC.2/VREF/tasa al integrar.

Reproducción: cmake -S "$R" -B "$O/host" -DCMAKE_BUILD_TYPE=Debug;
cmake --build "$O/host" --parallel 4; ctest --test-dir "$O/host" --verbose.
San agrega -DTPI_HOST_SANITIZERS=ON, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
y UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM agrega -DTPI_BUILD_ARM=ON
y -DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake". Doxygen con salida externa.
Detalle exacto de cada invocación en *-commands.json. Verificación anterior24adfe3
en Git y /tmp/ed3-b16-getter-33p91_4l; no sustituye evidencia de esta corrección.

Veredicto: PASS del alcance B16 corregido. Sin instalaciones, simulación MCU,
ejecución ARM/placa, flasheo o publicación. Commit local autorizado, sin push.
Próxima acción: revisar/aceptar B16 antes de sincronizar/archivar.
