# Verificación B13 — UART0

2026-11-10. Alcance/UART0/pines y baudios configurables autorizados. Implementado/
verificado, resultado pendiente de aceptación. Base e95205e, main. Commit local
autorizado, sin push/sincronización/archivo ni avance de bloque posterior.

## Contrato y diseño comprobados

UART_Read/UART_Write (.2.1.16.1/.2) validan NULL/tamaño cero antes de delegar;
retornan0 sin driver/memoria. Petición válida: una llamada a UART_Receive/Send
original con UART0/NONE_BLOCKING, retorno de cantidad sin transformarla/reintentar.
Sin buffers propios, estado, heap, esperas nuevas, ISR, protocolo o inicialización.
Doxygen documenta unidades, límites, progreso parcial, vida/estabilidad de buffers,
exclusividad con ISR/DMA y precondiciones físicas. No hay comentarios en cuerpos.

Asignación fija autorizada: TXD0 P0.2/RXD0 P0.3, función01 PINSEL0. Confirmada en
manual aportado UM10360 rev4.1 §8.5.1 tabla80/p118, renderizado/inspeccionado.
Driver UART_PinConfig ya contempla esos pines. No se ejecuta esa configuración:
inicialización e integración finales siguen diferidas por instrucción del usuario.
El render produjo avisos fontconfig de caché no escribible, imagen legible revisada;
no se instalaron fuentes/dependencias ni se cambiaron permisos para ocultarlos.

UART_BAUDRATE está en UART.h, default115200 modificable en header o -D del compilador.
#if/#error rechaza cero/negativo/>UINT32_MAX, sin _Static_assert en header. Parámetro
preparado para UART_CFG_T.baudRate futuro; no es velocidad aplicada al hardware ni
garantía de divisores/error de tasa. La aclaración amplía sólo datos de configuración,
manteniendo dos operaciones y excluyendo init. Sin otras desviaciones del diseño.

## Evidencia realmente ejecutada

Logs/objetos fuera de Git: `/tmp/ed3-b13-uart-ku1mtzv7`. C11 estricto -Wall -Wextra -Werror
-Wconversion -Wshadow -Wstrict-prototypes, pruebas/extras también -pedantic-errors.
Se conservan controles, fuentes de firmware y hooks. Sin instalación/flasheo.

| Control/modalidad | Resultado | Evidencia externa |
|---|---|---|
| B13 individual |1/1 suite,11 casos |b13-*.log |
| Host completo |19/19,194 casos C (183 previos+11) y oráculo Python |host-*.log |
| ASan/LSan/UBSan |19/19 sin hallazgos;detect_leaks=1 fuera de ptrace |san-*.log |
| ARM Debug |Build/13 controles imagen-enlace pasan; no ejecución MCU |arm-*.log |
| UART ARM-O0/O2 |Compila con headers originales; sólo UART_Receive/Send externos |UART-*.log |
| Baud default/overrides |115200 por defecto;9600/57600/230400 compilan C11/C++17; módulo ARM9600 compila |header-*.log, UART-override-arm.log |
| Baud inválido |0,-1,4294967296 rechazados al compilar en ambos lenguajes |header-*.log |
| Doxygen HTML/XML |40 funciones únicas (2 UART nuevas), macro/contratos sin avisos |doxygen.log, extra-results.json |
| Negativos externos |Detectan longitud TX ficticia, RX excediendo capacidad y modo bloqueante |*-run.log |
| Preservación |252 archivos protegidos intactos,90 Drivers originales a21cc1e |before.json, preservation.json y diff |

Once casos: ArgumentosVacios, RxVacio, RxCompletaBinaria (los256 bytes posibles),
RxParcialYCanarios, RxCapacidadYContinuacion, TxNoDisponible, TxCompletaBinaria,
TxParcialYCanarios, TxContinuacionSinDuplicar, RxTxIntercalados y
RecuperacionTrasInvalido. Verifican contenido/orden/canarios/resto, retorno real,
origen inmutable, UART0/NONE_BLOCKING y ausencia de reintentos/delegación en vacíos.
Regresiones ejecutadas:11199 ternas velocidad,5096 pares codec,4096 muestras/400 vueltas selector.

## Matriz de aceptación

| Contrato/escenario | Evidencia ejecutada | Resultado |
|---|---|---|
| UD1/S1 |ArgumentosVacios/RecuperacionTrasInvalido, NULL0/1/MAX y buffer0 |Conforme |
| UD2/S2 |RxCompletaBinaria/ParcialYCanarios/CapacidadYContinuacion/Intercalados |Conforme |
| UD2/S3 |RxVacio, destino íntegro y una delegación sin espera propia |Conforme |
| UD3/S4 |TxNoDisponible/CompletaBinaria/ParcialYCanarios/Intercalados |Conforme |
| UD3/S5 |TxContinuacionSinDuplicar, progreso2+0+7 y offset del consumidor |Conforme |
| UD4/S7 |Manual/headers: pines asignados; overrides válidos/errores de compilación |Conforme a asignación/parámetro; aplicación física pendiente |
| UD4/S6 |Host/san/ARM/Doxygen/hashes/diff |Conforme por modalidad |

7/7 escenarios según alcance y modalidad; cobertura mínima no configurada.
El doble reemplaza sólo UART_Send/UART_Receive y modela buffers/capacidad. No ejecuta
la biblioteca original ni simula CPU, UART eléctrica, transmisión PC o latencias.
Los tests comprueban el contrato del wrapper contra ese doble, no el driver físico.

## Recursos, límites y próxima acción

Objetos propios UART: .text104 bytes-O0/48-O2, .data/.bss0. Marcos propios16/16
bytes-O0,0/0-O2 por llamadas de cola; no cota total de pila/tiempo con driver/caller/IRQ.
No recursión, caché, estado compartido ni aritmética adicional de tamaños en producto.
Los tamaños válidos deben corresponder al objeto real; uint32_t no prueba su capacidad.

Main/servicios/otros dispositivos/plataforma/Drivers/contratos y archivos aceptados
conservados. Firmware mínimo descarta por GC los módulos sin llamadas; no acredita
enlace global de biblioteca. UART_Read/Write no llaman PinConfig/Init/TX enable ni
aplican UART_BAUDRATE; esas operaciones siguen pendientes junto con reloj, formato,
FIFO/DLAB y cableado/conversor. Sin otro consumidor ISR/DMA y acceso serializado.
El retorno TX mide bytes entregados al transmisor, no fin de envío/recepción PC.
Diagnóstico de overrun/paridad/framing, cadencia/colas y uso en Comunicacion/main
no implementados aquí. No ejecución en placa/ARM, simulación MCU ni publicación.

Reproducción: cmake -S "$R" -B "$O/host" -DCMAKE_BUILD_TYPE=Debug;
cmake --build "$O/host" --parallel 4; ctest --test-dir "$O/host" --verbose.
San: -DTPI_HOST_SANITIZERS=ON, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 y
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM: -DTPI_BUILD_ARM=ON y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake". Comandos/exits de controles
en host/san/arm/extras-commands.json; Doxygen vigente con salida externa.

Veredicto: PASS para alcance acordado.11/11 tareas terminadas. Revisar/aceptar B13
antes de sincronizar/archivar; no confundir este resultado con aceptación física.
