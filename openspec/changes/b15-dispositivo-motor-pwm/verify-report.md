# Verificación B15 — dispositivo Motor

2026-11-10, main, base62eef53. Alcance/PWM1.1/P2.0 acordados, implementado y
verificado; resultado pendiente de aceptación. Commit normal local autorizado;
sin push/sincronización/archivo ni avance de otros bloques.

## Implementación y diseño

Motor_Write recibe mando uint16_t 0..1000 y período uint32_t 1..UINT32_MAX-1,
en ticks y coincidente con MR0 efectivo/estable. Valida antes de llamar al driver;
false rechaza sin escritura/cero automático. Un pedido válido redondea al tick
más cercano con empate arriba, y despacha una vez PWM_MatchUpdateSingle(PWM_MATCH_1).
true confirma despacho lógico, no detección de fallos físicos. Sin contexto,
estado mutable, registros propios, init, reset, ISR, heap o esperas.

El máximo compartido sigue en Limites.h, intacto. Producto máximo
1000*(2^32-2)+500 cabe en uint64_t; resultado en 0..período. El extremo completo
usa match=período+1 para alto continuo, sin depender de igualdad MR1/MR0;
período UINT32_MAX se rechaza para que ese valor sea representable.
ESW.2.1.9/USW.2.1.9.1 documenta parámetros, retorno, unidades, errores,
precondiciones, concurrencia y latch; sin comentarios dentro del cuerpo.
Diseño acordado seguido, sin desvíos de comportamiento o alcance.

## Evidencia ejecutada

Artefactos/logs fuera del repositorio: `/tmp/ed3-b15-motor-e9u__4fp`. C11 con -Wall -Wextra -Werror
-Wconversion -Wshadow -Wstrict-prototypes; pruebas y objeto individual también
-pedantic-errors. No dependencias instaladas, dispositivos flasheados ni push.

| Modalidad/control | Resultado | Evidencia externa |
|---|---|---|
| B15 individual | 1/1 suite, 6/6 casos | b15-build/tests.log |
| Host completo | 21/21; 206 casos C (200 previos+6) y oráculo Python | host-*.log |
| ASan/LSan/UBSan | 21/21, sin hallazgos, detect_leaks=1 fuera de ptrace | san-*.log |
| ARM Debug | Build y 13/13 controles de imagen/enlace; sin ejecución MCU | arm-*.log |
| Motor ARM O0/O2 | Compila con headers originales, recursos/referencias inspeccionados | motor-*.log, *.su |
| Header aislado C11/C++17 | Compila con sólo ruta Dispositivos | header-*.log |
| Doxygen HTML/XML | 42 funciones únicas (1 nueva), sin avisos; USW9.1 verificada | doxygen.log, extra-results.json |
| Controles negativos externos | Detectan canal equivocado, producto32, truncamiento y extremo alto incorrecto | *-run.log |
| Preservación | 270 archivos protegidos intactos, 90 Drivers originales a21cc1e | before.json, preservation.json, diff |

Casos realmente ejecutados en tests/host/test_motor.c: DominioYExtremos,
RedondeoYEmpates, PeriodosGrandes, InvalidosYRecuperacion,
RepeticionYPeriodoExterno y PropiedadesDeCuantizacion.

La prueba de propiedades recorre 13013 combinaciones (13 períodos por 1001
mandos) y comprueba cota de error por desigualdad entera, empate hacia arriba,
monotonía y extremos; no vuelve a implementar la fórmula de división/redondeo.
El barrido rechaza todos los 64535 valores uint16_t entre1001 y65535 sin llamadas.
Tablas numéricas explícitas comprueban empates, períodos mínimos y máximos,
0/50/100 %, repetición y recuperación. El doble registra la API de match;
no reproduce LER/MMIO/biblioteca ni simula CPU, pulsos o motor físico.
Regresiones preservadas: 11199 ternas velocidad, 5096 pares codec y 4096
muestras/400 rollovers del selector. No pruebas omitidas ni umbral de cobertura configurado.

## Matriz de cumplimiento

| Contrato/escenario | Pruebas y evidencia ejecutadas | Resultado |
|---|---|---|
| MD1/S1 | DominioYExtremos: período1000, 0/500/1000 → MR1=0/500/1001 | Conforme |
| MD1/S2 | RedondeoYEmpates, PeriodosGrandes y 13013 combinaciones por desigualdad/monotonía | Conforme |
| MD2/S3 | InvalidosYRecuperacion: barrido1001..65535, período0/MAX, ninguna escritura y recuperación | Conforme |
| MD3/S4 | RepeticionYPeriodoExterno y conservación de otros matches en cada despacho | Conforme |
| MD4/S5 | Host/san/ARM/header/Doxygen y diff/hashes; configuración física excluida/documentada | Conforme por modalidad |

5/5 escenarios con evidencia ejecutada. Las variantes negativas sólo modifican
copias externas de Motor.c; cada defecto produce un fallo en las pruebas reales.

## Recursos y observaciones de verificación

Objeto Motor: .text156 bytes-O0/64-O2, .data/.bss0. Marco propio56 bytes-O0/8-O2;
no es cota total incluyendo driver, división de libgcc, llamadores o interrupciones.
Relocaciones reales sólo a PWM_MatchUpdateSingle y __aeabi_uldivmod.
O2 también declara el símbolo __aeabi_ldivmod sin relocación/call hacia él.
La auditoría externa inicial esperaba únicamente división unsigned y se detuvo;
se inspeccionó el desensamblado y se completó la regla con símbolos de libgcc
y control explícito de relocaciones. Sin cambios al firmware/pruebas para pasar
ese control; intento/notas conservados en extras-attempt1-*.
Conteo de casos agregado corregido leyendo la salida real GPIO (ocho casos,
no siete); no hubo fallo funcional, ver counting-note.txt.

## Límites y reproducción

PWM1.1/P2.0 función01 acordado, sin aplicar pinmux. Para uso físico: PWM exclusivo,
single-edge activo alto, MR0 efectivo/estable igual al período recibido, sin update
de MR0 pendiente, reset por MR0, sin otros matches que detengan/reinicien TC y
acceso privilegiado serializado. El latch se aplica al próximo ciclo; pueden
prevalecer órdenes posteriores antes de ese límite. No verifica período real ni
detecta configuración inválida/fallo eléctrico, movimiento o detención del motor.

Frecuencia, reloj/prescaler/pinmux/arranque, potencia/dirección/freno, adaptación
de Driver_Motor y main siguen para integración final. No regulación/rampas.
Fuentes de firmware siguen18 objetos; main mínimo permite eliminar por GC los
módulos sin llamadas. El enlace global de biblioteca sigue pendiente. Compilar
o probar un doble no acredita duty eléctrico; no ejecución ARM/placa ni simulación MCU.

Comandos/exits exactos: host/san/arm/extras-commands.json. R=raíz del proyecto,
O=directorio externo anterior: cmake -S "$R" -B "$O/host" -DCMAKE_BUILD_TYPE=Debug;
cmake --build "$O/host" --parallel 4; ctest --test-dir "$O/host" --verbose.
San añade -DTPI_HOST_SANITIZERS=ON, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 y
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM añade -DTPI_BUILD_ARM=ON y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake". Doxygen vigente con salida externa.

Veredicto: PASS para alcance acordado, sin fallos funcionales ni desviaciones
pendientes. 10/10 tareas completas. Revisar/aceptar B15 antes de sincronizar/archivar.
