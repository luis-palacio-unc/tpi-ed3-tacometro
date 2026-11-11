# Verificación B05-R1 — corrección de auditoría H1/H2

2026-11-11, main, baseb8c3c37. Auditoría externa local de Claude: aprobar con
cambios, sin bloqueantes; LM35.c correcto y hueco en pruebas de bits reservados.
Usuario autoriza reforzar pruebas/README y commit local, sin cerrar, sincronizar,
archivar o publicar. [Resumen portable de auditoría](review-notes.md).

## Cambio revisado

LM35.c, restante producción, drivers y configuración se conservan byte a byte.
test_lm35.c agrega a campos0x00FF0000 (23:16),0x38000000 (29:27) y0xFFFF000F
(todo bit fuera de RESULT15:4). Los tres valores0/512/2048 se prueban con9 máscaras,
incluidas las anteriores; se mantienen actualización/repetición y RAM intacta.
EmitirTabla usa0xFFFF000F para los4096 códigos de cada referencia.
Fixture ARM usa0xFFFF200F (muestra512) con VREF de prueba2000mV, resultado2500.
TE1/S1 candidato explicita todo bit fuera de15:4; no cambia comportamiento del módulo.
README declara entrega directa de centésimas de LM35_Read, sin reescalar/redondear.

## Mutante requerido: reproducción antes/después

Sólo en copias externas: reemplazar la extracción original por
`(uint16_t)(ADC_DMA_LM35 >> 4)` en la llamada a LM35_Convert. Original versionado intacto.
Se compila con VREF2000mV y C11 estricto; el mismo mutante se prueba contra el
test anterior copiado y después contra el test reforzado.

| Fixture del mutante | Unidad | Oráculo4096 códigos |
|---|---|---|
| Antes, máscaras antiguas |Exit0,4/4 casos: sobrevive |Exit0,4096/4096: sobrevive |
| Después, reservados activos |Exit1,FAIL S1 RESULT y campos ajenos |Exit1,primer código: esperado0,obtenido300000 |

El fallo esperado demuestra que el nuevo test detecta extracción incompleta.
No se atribuyen estos fallos a la implementación correcta ni se edita LM35.c.
Fuentes/copias, comandos, retornos y salida en mutante-before/after-*.log y
mutante-before/after-commands.json. No se agregan mutantes al árbol versionado.

## Evidencia propia nueva

Directorio externo: `/tmp/ed3-b05r1-bits-11qd108k`. Snapshot before.json y copia del informe previo.
Invocaciones completas: host/san/arm/docs/preservation-commands.json.
C11 con -Wall -Wextra -Werror -Wconversion -Wshadow -Wstrict-prototypes;
pruebas host además -pedantic-errors. No instalaciones o dependencias nuevas.

| Modalidad/control | Resultado ejecutado | Logs externos |
|---|---|---|
| Target2000/composición host |2/2 suites,4+3 casos |b05r1-build/tests.log |
| Host completo |29/29 suites,214 ejecuciones C |host-*.log |
| ASan/LSan/UBSan |29/29 sin hallazgos,detect_leaks=1 |san-*.log |
| Oráculo reforzado |20480 igualdades exactas,4096×VREF1000/2000/2540/3300/5000 en host y san, con0xFFFF000F |host/san-tests.log |
| Configuración |4 probes positivos y4 negativos por diagnóstico específico |host/san-tests.log |
| ARM Debug |Build y13/13 controles de imagen/enlace, incluido fixture reforzado |arm-*.log |
| Doxygen HTML/XML |43 funciones/USW, sin avisos |doxygen.log,docs-results.json |
| Preservación/enlaces |LM35.c/producción/contratos/archivo intactos;90 drivers originales a21cc1e y revisiones externas preservadas |preservation-results.json,diff-check.log |

214 ejecuciones C:194 en suites base y20 repeticiones en variantes; no casos nuevos.
Regresiones anteriores de velocidad/codec/selector y TE2 incluidas. Sin pruebas
omitidas ni umbral de cobertura configurado. El aumento es en palabras/bit-patterns,
no en cantidad de suites o funciones. Compilación ARM no equivale a ejecución en MCU.

| Requisito/escenario | Evidencia actual | Resultado |
|---|---|---|
| TE1/S1 |ValoresYCampos,9 máscaras por muestra; mutante ahora falla por reservados |Conforme |
| TE1/S2 |EscalaConfigurada/default/overrides y8 probes |Conforme |
| TE1/S3 |Fraction/floor,20480 comparaciones exactas con todos los bits ajenos activos |Conforme |
| TE3/S6 |ActualizacionYRepeticion, secuencia y cero sin escritura |Conforme |
| TE3/S7 |PropagacionDirecta/FuentesIndependientes/DisponibilidadExternaYCero; README corregido |Conforme |
| TE4/S8 |Host/san/ARM/Doxygen/mutante/diff/hashes |Conforme por modalidad |

6/6 escenarios del delta con evidencia;16/16 tareas al commit de esta entrega.
La ejecución inicial b8c3c37 se conserva en Git y /tmp/ed3-b05r1-impl-3lr7invk.
Recursos O0/O2/headers de ese informe son anteriores, informativos y no repetidos
en esta corrección: LM35.c y headers productivos no cambiaron. No confundirlos con
verificación nueva ni atribuir a estas ejecuciones las pruebas externas de Claude.

## Pendientes y reproducción

Auditoría de la corrección/aceptación del usuario pendientes. H3 informativo de
auditoría: definir origen del estado genérico de fuente/primera muestra al integrar;
cero inicial no señala disponibilidad. También siguen pendientes ADC/DMA/VREF/pin,
mezcla de canales ADGDR/posible ADDRn contra manual/erratas, integración y placa.
Ninguno se resuelve añadiendo estados o cambios al módulo en esta corrección.

R=raíz,O=directorio externo: cmake -S "$R" -B "$O/host" -DCMAKE_BUILD_TYPE=Debug
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON; cmake --build "$O/host" --parallel 4;
ctest --test-dir "$O/host" --verbose. San agrega -DTPI_HOST_SANITIZERS=ON,
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 y
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM agrega -DTPI_BUILD_ARM=ON
y -DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake"; Doxygen con salida externa.
Detalle exacto en *-commands.json. Veredicto propio: PASS de corrección H1/H2.
Sin ejecución MCU/placa, simulación, flasheo, cierre/sincronización/archivo/push.
