# Verificación B15 — mando directo con MR0=1000

2026-11-10, main, base cf17359. Usuario fija MR0=1000 y ordena enviar el mando
sin cálculo de match. Implementado/verificado, resultado pendiente de aceptación.
Commit local según autorización vigente; sin push/sincronización/archivo.

## Implementación actual

Motor_Write(uint16_t mando_por_mil) valida0..1000 y llama exactamente
PWM_MatchUpdateSingle(PWM_MATCH_1,mando_por_mil). 1000 se envía como1000.
No lee/escribe MR0, calcula match, usa uint64_t, multiplica/divide/redondea o
transforma el extremo en1001. El máximo compartido de Limites.h sigue intacto.
false rechaza sin llamada/cero automático; true indica despacho, no electricidad.
Sin contexto/caché/estado propio/init/ISR/heap/esperas. MR0=1000 es condición de
configuración para la inicialización final; no se ejecuta arranque ahora.
USW.2.1.9.1/Doxygen actualizado, sin comentarios dentro del cuerpo.

Modelo RAM de MR0 y preinclude de pruebas_motor retirados por no ser necesarios.
La prueba sólo dobla la API de escritura, sin ejecutar biblioteca/registros.
Las versiones de conversión anteriores y sus informes quedan en Git; copia del
informe previo en previous-verify-report.md externo. No acreditan contrato actual.

## Evidencia ejecutada

Logs/objetos: `/tmp/ed3-b15-directo-2yz3lskk`. C11 estricto -Wall -Wextra -Werror -Wconversion -Wshadow
-Wstrict-prototypes; pruebas/objeto individual además -pedantic-errors.

| Modalidad/control | Resultado | Logs externos |
|---|---|---|
| B15 individual | 1/1 suite,4/4 casos | b15-build/tests.log |
| Host completo | 21/21,204 casos C y oráculo Python | host-*.log |
| ASan/LSan/UBSan | 21/21 sin hallazgos,detect_leaks=1 fuera de ptrace | san-*.log |
| ARM Debug | Build y13/13 controles de imagen/enlace; no ejecución MCU | arm-*.log |
| Motor ARM O0/O2 | Compila; único externo/relocación PWM_MatchUpdateSingle | motor-*.log,*.su |
| Header aislado | C11/C++17, una entrada | header-*.log |
| Doxygen HTML/XML | 42 funciones sin avisos, USW9.1 con un parámetro | doxygen.log,extra-results.json |
| Negativos externos | Detectan canal incorrecto,1000→1001 y omisión del rechazo | *-run.log |
| Preservación | 270 archivos protegidos,90 Drivers originales a21cc1e | before.json,preservation.json |

Cuatro casos: ValoresDeReferencia, DominioCompleto, RechazoYRecuperacion y
RepeticionYCero. Se recorren1001 mandos válidos con envío exacto y64535 mandos
inválidos sin llamada, con recuperación. No se conserva un error previo ni se
omiten solicitudes repetidas. Los casos de conversión a períodos variables se
retiran porque el usuario reemplazó ese contrato, sin reducir controles vigentes.
Regresiones:11199 ternas velocidad,5096 pares codec,4096 muestras/400 rollovers
selector. Sin pruebas omitidas ni umbral de cobertura configurado.

| Contrato/escenario | Evidencia actual | Resultado |
|---|---|---|
| MD1/S1 | ValoresDeReferencia,0/250/500/1000 exactos | Conforme |
| MD1/S2 | DominioCompleto,0..1000 íntegros,1001 despachos | Conforme |
| MD2/S3 | RechazoYRecuperacion,1001..65535 sin despacho y recuperación | Conforme |
| MD3/S4 | RepeticionYCero,una llamada por solicitud,inspección sin acceso propio a MR0 | Conforme |
| MD4/S5 | Host/san/ARM/header/Doxygen,diff/hashes y retiro de modelo/preinclude | Conforme por modalidad |

5/5 escenarios actuales con evidencia. Diseño seguido sin desviaciones.
Negativos usan sólo copias externas; no alteran firmware/biblioteca versionados.

## Recursos y límites

.text propio42 bytes-O0/24-O2,.data/.bss0,marco propio16/8 bytes; no cota total
incluyendo driver/llamador/ISR. Desensamblado/símbolos sin helpers aritméticos ni
acceso a MR0/modelo. compile_commands confirma retiro del preinclude en todos
los objetos de Motor. Fuente firmware conserva18 objetos; main mínimo elimina
por GC módulos sin llamadas y el enlace global de biblioteca sigue pendiente.

Precondición física: PWM1.1/P2.0 función01, MR0 efectivo1000 estable, single-edge
alto, reset por MR0 sin otros matches que alteren TC, acceso exclusivo/serializado.
Motor_Write no verifica ni corrige esa configuración. Latch en próximo ciclo;
varias solicitudes previas pueden dejar sólo la última.

Fuente/fallo PWM.1 y corrección de la explicación previa del100 % registrados en
[exploración](exploration.md). Se mantiene envío literal por orden del usuario;
validación física de ese límite queda para integración. El doble no simula CPU,
latch/temporización/electricidad o motor; no acredita transitorios reales.
Frecuencia/reloj/prescaler/pinmux/arranque/potencia/adaptación B08/main pendientes.

Reproducción exacta: host/san/arm/extras-commands.json. R=raíz,O=directorio externo:
cmake -S "$R" -B "$O/host" -DCMAKE_BUILD_TYPE=Debug;
cmake --build "$O/host" --parallel 4; ctest --test-dir "$O/host" --verbose.
San agrega -DTPI_HOST_SANITIZERS=ON, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
y UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM agrega -DTPI_BUILD_ARM=ON y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake". Doxygen con salida externa.
Sin instalaciones, flasheo, ejecución ARM/placa, simulación MCU o publicación.

Veredicto: PASS del alcance lógico,10/10 tareas completas; aceptación física
pendiente. Revisar/aceptar resultado antes de sincronizar/archivar.
