# Verificación B05-R1 — implementación pendiente de auditoría

2026-11-11, main, base978af35 con12 documentos de propuesta/contexto locales previos.
Usuario acordó implementar propuesta aprobada por Claude r2, incorporar m1–m3 y
crear commit de implementación. Sin cierre, sincronización de contratos, archivo o push.
La aprobación r2 fue de propuesta; auditoría del código/aceptación del resultado pendientes.
[Resumen portable de revisiones externas locales](review-notes.md).

## Implementación revisada

LM35_Read(void) devuelve int32_t centésimas, sin estados ni parámetros. Lee una vez
ADC_DMA_LM35, extrae RESULT original y llama LM35_Convert interna. No toca registros,
inicia ADC/DMA, filtra, valida flags/canal o conserva temperatura anterior.
Muestra volatile32bits en LM35.c; extern interno y único escritor DMA al integrar.
Sin Adquisicion_ADC_DMA.c en los seis targets de test del dispositivo (m2).

m1: LM35_ADC_BITS/CUENTAS, LM35_MV_POR_GRADO y LM35_CENTESIMAS_POR_MV, derivado
de100 centésimas/°C entre10mV/°C. Numerador uint64_t; cociente/resto con divisor4096,
redondeo B05 cercano/empate arriba, error aritmético≤0,005°C. El guard limita VREF
a1..INT32_MAX/10, garantizando salida representable. Sin float/double o estados nuevos.
USW11.1/11.2 retiradas;11.3 getter,11.4 conversión interna, documentadas con Doxygen.

Analogica.h usa #ifndef/default2540mV provisional/#error; no es VREF confirmada de
placa. Overrides PRIVATE por target. Temperatura y restantes servicios/aplicación
intactos; fixture pasa las centésimas directamente, sin adaptar unidades.
m3: resumen de revisión dentro del cambio; sin enlaces al harness fuera del clon.

## Evidencia propia ejecutada

Logs/objetos: `/tmp/ed3-b05r1-impl-3lr7invk`. Invocaciones completas y códigos de salida en
host/san/arm/extras/preservation-commands.json. Snapshot inicial before.json/initial.diff.
Modo estándar, sin afirmar TDD. C11 con -Wall -Wextra -Werror -Wconversion -Wshadow
-Wstrict-prototypes; tests/objetos aislados además -pedantic-errors.

| Modalidad/control | Resultado | Evidencia externa |
|---|---|---|
| Target LM352000 y fixture host |2/2 suites:4 casos del dispositivo y3 de composición |b05r1-build/tests.log |
| Host completo |29/29 suites;214 ejecuciones C y oráculos Python |host-*.log |
| ASan/LSan/UBSan |29/29 sin hallazgos; detect_leaks=1 |san-*.log |
| S3/oráculo nuevo |20480 comparaciones exactas,4096×5 referencias, sin tolerancia |host/san-tests.log |
| Configuración |4 probes positivos y4 negativos con diagnóstico ADC_VREF_MV |host/san-tests.log |
| ARM Debug |Build y13/13 controles de imagen/enlace, fixture LM35 actualizado retenido |arm-*.log |
| ARM O0/O2 |Compila; ADC_DMA_LM35 único símbolo B de4 bytes; sin externos pendientes |lm35-*-build/symbols/externals.log |
| Headers aislados |C11/C++17 correctos |header-*.log |
| Doxygen HTML/XML |43 funciones/USW, incluidas11.3/11.4 y conversión estática; sin avisos |doxygen.log,extra-results.json |
| Controles negativos externos |Detectan empate truncado, extracción omitida y muestra borrada |*-run.log |
| m2/defines |6 targets con sólo test_lm35.c+LM35.c; macros correctas en ambas unidades, firmware sin overrides |preservation-results.json |
| m3/preservación |Enlaces internos válidos, sin rutas al harness;90 drivers originales a21cc1e, módulos/contratos/archivo intactos |preservation-results.json |

Los214 casos C son194 en las suites base revisadas y20 repeticiones en las cinco
variantes; no214 casos nuevos. Se reemplazan los17 casos de la API LM35 anterior
por4 de la nueva; composición mantiene3. Suite autónoma Temperatura conserva10.
Regresiones:11199 ternas velocidad,5096 pares codec,4096 muestras/400 rollovers selector.
Sin pruebas omitidas; sin umbral de cobertura porcentual configurado.

| Requisito/escenario | Prueba/evidencia | Resultado |
|---|---|---|
| TE1/S1,H4 |ValoresYCampos a2000 y otros builds:0/512/2048, campos aislados/OR0xC700000F |Conforme |
| TE1/S2,H2/H7 |EscalaConfigurada, compile_commands y default1024→6350; guard0/−1/214748365/0x7FFFFFFF |Conforme |
| TE1/S3,H1 |Redondeo128@1000→313; verificar_lm35.py con Fraction/floor en20480 casos exactos |Conforme |
| TE3/S6,H4 |ActualizacionYRepeticion, secuencia requerida y cero, RAM intacta |Conforme |
| TE3/S7 |PropagacionDirecta/FuentesIndependientes/DisponibilidadExternaYCero; sin convertir unidades |Conforme |
| TE4/S8 |Host/san/ARM/header/Doxygen/diff/hashes y evidencia separada |Conforme por modalidad |

6/6 escenarios del delta con evidencia; TE2/S4/S5 vigente conserva pruebas autónomas.
13/13 tareas de implementación/entrega al hacer commit, sin cerrar el cambio SDD.
Negativos de mutaciones sólo en copias externas; no se cambia firmware para obtener verde.
Ensayos que Claude informó sobre propuesta/base se distinguen en review-notes.md y no
se cuentan como ejecución propia ni auditoría del código actual.

## Recursos y límites

LM35 .text268 bytes-O0/32-O2, .bss4 alineada4 y .data0. O0: marco96 en Convert y8
en Read (104 propios anidados); O2: Convert inline, marco propio0. No son cotas de
pila global o tiempos reales. Sin __aeabi_uldivmod ni auxiliares de punto flotante
en esos objetos; divisor potencia de2 resuelto por el compilador. Revisar logs de símbolos.
Firmware conserva18 objetos; main mínimo elimina módulos no referenciados por GC.
Fixture ARM retiene LM35_Read/ADC_DMA_LM35/Temperatura y usa VREF de prueba2000;
su enlace no ejecuta instrucciones en MCU. No se agregó configuración o enlace productivo.

H3 sigue pendiente: ADGDR con dos destinos no garantiza ruteo de canal. Evaluar
ADDRn/UM10360/GPDMA/erratas/API original al integrar; no cambiar B16 o Drivers ahora.
Primer dato, pin/canal/VREF físicos y configuración ADC/DMA al final. RAM de pruebas
no acredita conversión, frescura, sensor conectado, exactitud eléctrica o tiempo real.

Reproducción: R=raíz,O=directorio externo; cmake -S "$R" -B "$O/host"
-DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON; cmake --build "$O/host"
--parallel 4; ctest --test-dir "$O/host" --verbose. San agrega
-DTPI_HOST_SANITIZERS=ON y ASAN_OPTIONS=detect_leaks=1:halt_on_error=1,
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM agrega -DTPI_BUILD_ARM=ON y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake". Doxygen con salida externa.
Comandos exactos y resultados completos en *-commands.json/*.log externos.

Veredicto propio: PASS del alcance implementado; sin hallazgos críticos. Auditoría
posterior de Claude/aceptación pendientes. No sincronizar, cerrar, archivar o publicar.
Sin instalaciones, flasheo, simulación MCU o ejecución física.
