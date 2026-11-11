# Verificación B16 — lectura del potenciómetro desde memoria

2026-11-10, main, HEAD de incorporación14b0521. Alcance corregido acordado por
«procede a implementar B16». Implementado y verificado; resultado aún no aceptado.
Commit local autorizado al terminar; sin push, sincronización de specs o archivo.

## Implementación revisada

Pot_Read(void) devuelve uint16_t mediante ADC_GDR_RESULT(ADC_DMA_Pot) original.
Una consulta volatile de32 bits, extrae RESULT15:4 y entrega0..4095 cuentas.
No recibe palabra DMA, salida por puntero, contexto o estado. Sin validadores,
DONE/OVERRUN/canal, espera, copia persistente, limpieza, conversión/transferencia,
inicialización o normalización. Sin comentarios en el cuerpo de la función.
USW.2.1.12.1 documentada con retorno/precondición/concurrencia y límites.

El soporte existente posee la única palabra ADC_DMA_Pot (4 bytes, alineación4),
en .bss de SRAM local. DMA será el productor al integrar; ahora no hay adquisición.
Canal AD0.0/P0.23 función01 acordado. La secuencia final debe asociar ese canal
al destino dedicado, con transferencias de32 bits y sin escritores CPU concurrentes.
El getter no comprueba esa precondición. Cero inicial no indica disponibilidad.

## Ejecuciones y evidencia

Directorio externo: `/tmp/ed3-b16-getter-33p91_4l`. Comandos completos y retornos en
host/san/arm/extras-commands.json; snapshot inicial con12 documentos de propuesta
ya modificados en before.json/initial.diff. Se preservó esa corrección del usuario.
Modo estándar, sin afirmar ciclo TDD. C11 estricto; -Wall -Wextra -Werror
-Wconversion -Wshadow -Wstrict-prototypes; tests/objetos aislados además -pedantic-errors.

| Modalidad/control | Resultado ejecutado | Evidencia externa |
|---|---|---|
| B16 individual | 1/1 suite,3/3 casos | b16-build.log,b16-tests.log |
| Host completo | 22/22 suites,207 casos C y oráculo Python | host-*.log |
| ASan/LSan/UBSan | 22/22 sin hallazgos,detect_leaks=1 | san-*.log |
| ARM Debug | Build y13/13 controles de imagen/enlace previos | arm-*.log |
| Getter/soporte ARM O0/O2 | Compilan y enlazan parcialmente sin externos restantes | *-build.log,b16-*-partial-link.log |
| Headers aislados | Potenciometro/soporte en C11 y C++17 | header-*.log |
| Doxygen HTML/XML | 43 funciones,USW12.1; sin avisos | doxygen.log,doxygen-final.log,extra-results.json |
| Negativos en copias externas | Detectan dato sin extraer,copia obsoleta y borrado de muestra | *-run.log |
| Preservación/diff | 90 drivers originales a21cc1e; módulos/contratos/plataforma intactos | preservation-results.json,diff-check.log |

LecturasDeReferencia usa palabras constantes para0/2048/4095, sin exigir DONE.
ActualizacionYRepeticion cambia la misma posición entre llamadas, incluso retorna
a un valor anterior, y comprueba repetición sin escritura. SoloResultadoSinEscritura
cambia flags sin cambiar el significado de RESULT y comprueba retorno/inalteración
de la palabra completa; no agrega un modelo de estados. No se enlazan funciones
del periférico en el test del getter. El doble es la memoria, no ADC/DMA ejecutados.

Regresiones existentes:11199 ternas de velocidad contra oráculo independiente,
5096 pares codec,4096 muestras/400 rollovers selector. Sin pruebas omitidas.
Cobertura porcentual sin umbral configurado. Negativos esperadamente fallidos sólo
en copias externas; no se modifica ni sustituye biblioteca/firmware para obtener verde.

| Requisito/escenario | Prueba ejecutada/revisión | Resultado |
|---|---|---|
| PD1/S1 | LecturasDeReferencia | Conforme |
| PD1/S2 | ActualizacionYRepeticion | Conforme |
| PD2/S3 | SoloResultadoSinEscritura y repetición; objetos sin llamadas a periféricos | Conforme |
| PD3/S4 | Host/san/ARM/header/Doxygen,diff/hashes y separación de modalidades | Conforme por modalidad |

4/4 escenarios con evidencia para el alcance del getter. Diseño seguido sin
desviaciones, sin hallazgos críticos.9/9 tareas cumplidas al cierre técnico/commit.

## Recursos y límites

Getter .text32 bytes-O0/16-O2; marco propio4/0 bytes según -fstack-usage, sin
datos persistentes propios. Soporte .bss4 bytes, alineación4; sin funciones.
O2 muestra un LDR de la palabra y UBFX de12 bits, sin stores/llamadas/esperas.
Único símbolo externo del getter: ADC_DMA_Pot; resuelto al enlazar con soporte.
No hay helpers aritméticos/heap. No es una cota de pila del firmware ni de tiempo real.

Build conserva18 objetos y main mínimo. El linker elimina módulos sin referencias;
las13 pruebas ARM existentes y el enlace parcial no ejecutan instrucciones en MCU.
Enlace global de biblioteca/configuración/main se dejan al final.
UM10360 rev4.1 §1.9/p12 permite SRAM local con MCU activo; si se agrega Sleep,
§4.8.1 requiere revisar ubicación, porque DMA no accede a SRAM principal allí.
ADC.2/elección de secuencia/canales/VREF/tasa quedan para integrar, sin añadir
validaciones dentro del getter. No se garantiza edad ni coherencia multicanal.

Reproducción: R=raíz,O=directorio externo. cmake -S "$R" -B "$O/host"
-DCMAKE_BUILD_TYPE=Debug; cmake --build "$O/host" --parallel 4;
ctest --test-dir "$O/host" --verbose. San agrega -DTPI_HOST_SANITIZERS=ON,
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 y
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM agrega -DTPI_BUILD_ARM=ON
y -DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake". Doxygen usa Doxyfile
con OUTPUT_DIRECTORY externo. Detalle exacto en los archivos *-commands.json.

Veredicto: PASS del alcance de lectura de memoria. Configuración, conversiones,
DMA real y ensayos físicos pendientes. Sin simulación MCU, instalaciones, flasheo
o publicación. Próxima acción: revisar/aceptar B16 antes de sincronizar/archivar.
