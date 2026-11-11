# Cierre B05-R1 — 2026-11-11

El usuario acepta la implementación `94d31344d3cf523a71af4afd4aaba90b0eda046e`.
Reauditoría externa local de Claude `B05-R1-implementacion-r2.md`: APROBAR,
sin hallazgos abiertos. [Resumen portable](review-notes.md).
La aceptación autoriza sincronización, archivo y commit local de cierre; no push.

## Sincronización y archivo

- Temperatura: TE1/TE3/TE4 reemplazados por el delta aceptado, conservando todos sus
  escenarios y cláusulas. Sólo se omiten las dos notas históricas «Antes» del
  contrato vigente; el delta original completo se conserva aquí.
- TE2/S4/S5 conservados byte a byte. SHA-256 de su sección completa:
  `85fa151f33a31b0642e14d3e1c4ea9775682d7ef1685bbbbedcb12a20270665a`. Contrato resultante:4 requisitos y8 escenarios.
- Cambio movido a `openspec/changes/archive/2026-11-11-b05-r1-lectura-lm35`; carpeta activa retirada,16/16 tareas completas.
  Se conservan proposal, design, exploration, tasks, state, delta, verify-report,
  review-notes y este informe. Los pendientes de auditoría/aceptación en el informe
  previo son históricos y quedan resueltos por este cierre.
- ESTADO, CONTEXTO, MAPA_REQUISITOS, README y openspec/config.yaml actualizados.
  Código, pruebas, build, Drivers e historial archivado previo intactos.

## Verificación propia nueva del cierre

Todas las ejecuciones siguientes se hicieron sobre94d3134 con código/pruebas sin
cambios. Se distinguen de la evidencia externa de Claude y de la corrección previa.
Logs, comandos completos, snapshot y resultados fuera del árbol versionado:
`/tmp/ed3-b05r1-cierre-qe5tm7u1`.

| Modalidad/control | Resultado ejecutado | Evidencia externa al árbol |
|---|---|---|
| Host Debug completo |29/29 suites,214 ejecuciones C, sin omisiones |host-commands.json,host-tests.log |
| ASan/LSan/UBSan |29/29, sin hallazgos, detect_leaks=1 |san-commands.json,san-tests.log |
| Oráculo LM35 |20480 igualdades exactas por modalidad host/san;4096 códigos×5 VREF, máscara0xFFFF000F |host/san-tests.log |
| Configuración VREF |4 probes positivos/4 negativos por diagnóstico esperado, en host y san |host/san-tests.log |
| ARM Debug |Compilación y13/13 controles de imagen/enlace, incluido fixture LM35 |arm-commands.json,arm-tests.log |
| Doxygen HTML/XML |43 funciones/USW, incluidos11.3/11.4, sin avisos |docs-commands.json,docs-results.json,doxygen.log |
| Cierre documental |Delta/contrato y TE2 exactos, archivo completo, enlaces, hashes y diff revisados |closure-checks.json,diff-check.log |

Host usa CMake Debug y ctest --verbose; sanitizadores agregan
TPI_HOST_SANITIZERS=ON, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 y
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM usa TPI_BUILD_ARM=ON y
cmake/arm-none-eabi.cmake. Doxygen conserva su configuración con salida externa.
Las invocaciones exactas y retornos están en los JSON indicados.
Las214 ejecuciones C incluyen194 base y20 repeticiones de variantes. Sin pruebas
omitidas ni cambios para obtener resultados verdes. Los mutantes y medidas O0/O2
del informe previo no se repiten en este cierre.

## Límites y próxima acción

Compilar/enlazar ARM no acredita ejecución en MCU. Pruebas en RAM no acreditan
adquisición ADC/DMA, frescura ni electricidad. Sin simulación, pruebas físicas,
instalaciones o flasheo. VREF real, pin/canal, primera muestra y estado genérico de
fuente quedan para integración. Evaluar ADDRn por canal frente al riesgo de mezcla
ADGDR/dos destinos contra UM10360/erratas/API original, sin modificar Drivers ahora.
Inicialización e integración al final. Acordar el siguiente bloque; ninguno nuevo
autorizado. Commit local documental; sin push.
