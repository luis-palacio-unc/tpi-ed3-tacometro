# Verificación B15 corregido — período leído de MR0

2026-11-10, main, base c952892. Usuario autoriza simplificar Motor_Write a un
parámetro y leer MR0, actualizar especificaciones/Doxygen/pruebas y crear commit
sin push. Resultado implementado/verificado, pendiente de aceptación.

## Comportamiento y diseño

bool Motor_Write(uint16_t mando_por_mil) valida mando0..1000, lee una vez
LPC_PWM1->MR0 a variable local const y rechaza MR0=0/UINT32_MAX. Mantiene el
redondeo cercano/empate arriba mediante uint64_t y el extremo alto MR1=MR0+1.
El único despacho válido es PWM_MatchUpdateSingle(PWM_MATCH_1,match).
false rechaza sin escritura/cero automático; true significa despacho, no
diagnóstico eléctrico. Sin contexto/caché/estado propio/init/ISR/heap/esperas.
Sin escribir MR0 ni configurar frecuencia, pines o reloj. La lectura directa es
específica de Dispositivos; el driver no ofrece getter de período y no se modifica.
USW.2.1.9.1 conserva su ID y documenta un parámetro, retorno/errores/unidades,
concurrencia y precondición de MR0 efectivo/estable, sin update pendiente.

Esta orden del usuario reemplaza el período explícito de c952892. Su informe
anterior queda en Git y copia externa previous-verify-report.md; no acredita
por sí mismo el comportamiento corregido. Se ejecutó toda la evidencia siguiente
contra la firma nueva. Contratos aceptados y archivos archivados intactos.

## Evidencia actual ejecutada

Logs/objetos externos: `/tmp/ed3-b15-mr0-y8gcpwyc`. C11 estricto -Wall -Wextra -Werror -Wconversion
-Wshadow -Wstrict-prototypes; tests/objeto individual también -pedantic-errors.
Modo estándar; no se declara ciclo RED previo ni pruebas físicas.

| Modalidad/control | Resultado | Evidencia externa |
|---|---|---|
| B15 individual | 1/1 suite, 6/6 casos actualizados | b15-build/tests.log |
| Host completo | 21/21, 206 casos C y oráculo Python | host-*.log |
| ASan/LSan/UBSan | 21/21, detect_leaks=1 sin hallazgos fuera de ptrace | san-*.log |
| ARM Debug | Build y 13/13 controles de imagen/enlace; sin ejecución MCU | arm-*.log |
| Motor ARM O0/O2 | Compila con puntero original, lectura de MR0 inspeccionada | motor-*.log,*.su |
| Header aislado | C11/C++17, firma de un parámetro | header-*.log |
| Doxygen HTML/XML | 42 funciones únicas sin avisos; una entrada en USW9.1 | doxygen.log, extra-results.json |
| Negativos externos | Detectan MR1 equivocado, período fijo, canal equivocado, producto32, truncamiento y extremo alto | *-run.log |
| Preservación | 270 archivos protegidos intactos;90 Drivers originales a21cc1e | before.json,preservation.json |

Casos actualizados: DominioYExtremos, RedondeoYEmpates, PeriodosGrandes,
InvalidosYRecuperacion, RepeticionYLecturaMR0 y PropiedadesDeCuantizacion.
Se conservan 13013 combinaciones de mando/período con cota de error por
desigualdad entera, empate arriba y monotonía; no duplican fórmula de división.
Los 64535 valores1001..65535 se rechazan sin escritura. MR0=0/MAX también se
rechaza y la siguiente llamada válida recupera. Tablas verifican 0/50/100 %,
períodos mínimos/máximos, redondeo y repetición. Regresiones:11199 ternas velocidad,
5096 pares codec,4096 muestras/400 rollovers del selector. Sin pruebas omitidas;
umbral de cobertura no configurado. Las seis variantes sólo alteran copias externas.

## Matriz de cumplimiento

| Contrato/escenario | Prueba ejecutada | Resultado |
|---|---|---|
| MD1/S1 | DominioYExtremos, MR0=1000, mando0/500/1000 → MR1=0/500/1001 | Conforme |
| MD1/S2 | RedondeoYEmpates, PeriodosGrandes y propiedades13013 | Conforme |
| MD1/S6 | RepeticionYLecturaMR0: mismo mando250, MR0=1000→2000, salida250→500 | Conforme |
| MD2/S3 | InvalidosYRecuperacion: MR0 inválido/mando inválido, sin despacho y recuperación | Conforme |
| MD3/S4 | RepeticionYLecturaMR0 y OtrosConservados en cada orden | Conforme |
| MD4/S5 | Host/san/ARM/header/Doxygen, compile_commands, diff/hashes | Conforme por modalidad |

6/6 escenarios con evidencia actual. No desvíos del diseño corregido.

## Modelo host y recursos

tests/host/dobles/pwm_registros.h incluye LPC17xx.h original y redirige LPC_PWM1
a una estructura RAM de su tipo original. Preinclude privado exclusivamente a
pruebas_motor; compile_commands confirma que no afecta firmware ni otros targets.
El doble de escritura sólo registra despacho, sin copiar implementación de LER
o biblioteca. Las pruebas modifican MR0 en el modelo entre llamadas y comprueban
su conservación. No es simulación de CPU, latch, tiempo, salida eléctrica o motor.

ARM usa la dirección real: desensamblado O2 muestra base0x40018000 y carga de
MR0 en offset24, sin símbolo del modelo. Únicas relocaciones de llamada:
PWM_MatchUpdateSingle y __aeabi_uldivmod. O2 declara __aeabi_ldivmod sin referencia.
.text propio168 bytes-O0/72-O2, .data/.bss0, marco propio64 bytes-O0/8-O2;
no cota total incluyendo driver/libgcc/llamador/ISR ni medida de latencia real.
El modelo RAM sólo existe en pruebas; no añade memoria al firmware.

## Límites y reproducción

Leer MR0 obtiene valor programado y no detecta si ya pasó al latch activo.
Uso físico exige PWM accesible/preconfigurado/arrancado/exclusivo, MR0 efectivo/
estable sin update pendiente, single-edge alto, reset por MR0 y sin otros matches
que alteren TC; acceso privilegiado serializado. Actualización en próximo ciclo,
varias órdenes pueden dejar sólo la última. El código no comprueba reloj, nivel,
duty real, movimiento o detención. Configuración/frecuencia/potencia/adaptación
Driver_Motor/main al final; TIMER0 y TIMER1/sensor óptico conservados.

Firmware sigue18 objetos con main mínimo; GC elimina módulos sin llamadas y
enlace global de biblioteca sigue pendiente. No ejecutar ARM/placa, simulación
MCU, instalación, flasheo, publicación ni editar Drivers para realizar estas pruebas.

Comandos/exits exactos en host/san/arm/extras-commands.json. R=raíz y O=directorio
externo anterior: cmake -S "$R" -B "$O/host" -DCMAKE_BUILD_TYPE=Debug;
cmake --build "$O/host" --parallel 4; ctest --test-dir "$O/host" --verbose.
San agrega -DTPI_HOST_SANITIZERS=ON con ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
y UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM agrega -DTPI_BUILD_ARM=ON y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake". Doxygen con salida externa.

Veredicto: PASS para B15 corregido,10/10 tareas completas, sin incidencias
funcionales pendientes. Commit local sin push; aceptar resultado antes de sincronizar/archivar.
