# Verificación B12 — dispositivos GPIO

2026-11-10. Alcance autorizado; resultado pendiente de aceptación. Base cb380bc
con planificación local conservada. Commit normal local autorizado al cerrar,
sin push/sincronizar/archivar. No avanzar bloque siguiente.

## Cambios y contrato

Tres USW, anexoB: Selector_Read (.13.1) consulta GPIO_ReadValue(PORT_2) y aísla
PIN_6 activo por alto; LED_Write (.14.1) usa GPIO_SetPinState(P0.22) bajo al activar;
Buzzer_Write (.15.1) usa GPIO_SetPinState(P2.7) alto al activar. Headers y cuerpos
documentados con Doxygen. Operaciones/set-clear delegadas a biblioteca original.
LED/Buzzer aceptan contexto ignorado/NULL y retornan true tras delegar: aceptación
de llamada compatible con B07, sin afirmar confirmación física ni inventar fallos.

Sin inicialización de GPIO, dirección, multiplexado, polarización, FIOMASK, reloj,
main o enlace global. Los pines requieren configuración previa al uso físico.
No se modificaron servicios, Timer/LM35 aceptados, startup ni biblioteca Drivers.

## Evidencia realmente ejecutada

Logs/snapshot fuera del árbol: `/tmp/ed3-b12-gpio-implementacion-m6evumxd`. C11 estricto -Wall -Wextra -Werror
-Wconversion -Wshadow -Wstrict-prototypes; tests/módulos externos además
-pedantic-errors. No instalar dependencias ni desactivar controles.

| Control/modalidad | Resultado | Evidencia externa |
|---|---|---|
| Tests antes del código | RED esperado por interfaces ausentes | red.log |
| B12 individual | GREEN9 casos | b12-green.log |
| Host completo |18/18,188 casos C (179 previos+9 B12) y oráculo Python | host-tests.log |
| ASan/LSan/UBSan |18/18 sin hallazgos, fugas habilitadas fuera de ptrace | san-*.log |
| ARM Debug build/regresiones |13/13; módulos con headers reales; no ejecución MCU | arm-*.log |
| Módulos ARM-O0/O2 | Compilan, sólo símbolos externos GPIO_ReadValue/SetPinState | *-build/symbols/disassembly.log |
| Headers aislados C11/C++17 | Compilan sin headers CMSIS ni drivers | *-header-*.log |
| Doxygen HTML/XML estricto |38 funciones propias únicas,3 nuevas, sin avisos | doxygen.log |
| Negativos externos | Detecta selector en bit7 y buzzer con polaridad invertida | extra-results.json, *-run.log |
| Preservación |220 archivos protegidos intactos;90 Drivers originales a21cc1e | protected-before.json y revisión final |

Nueve casos B12: SelectorNiveles, SelectorIgnoraOtrosBits (64 patrones de un bit
alto/bajo), SelectorIgnoraOtrosPuertos, LedActivoBajo, BuzzerActivoAlto,
ContextosIgnorados, RepeticionRestauraSalida, ComposicionAlarmas, SalidasIntercaladas.
Las salidas se verifican con bancos iniciales0/MAX/alternos, preservando vecinos y
otros puertos. Composición real con Driver_Alarmas y ambas polaridades opuestas;
el selector vecino P2.6 conserva su valor al accionar buzzerP2.7.
Regresiones:11199 ternas velocidad,5096 pares codec,4096 muestras/400 vueltas selector.

## Matriz

| Candidato | Evidencia | Resultado |
|---|---|---|
| DG1/S1 | Tres casos selector +64 patrones independientes | Contacto alto/bajo correcto, bit6, sin escritura |
| DG2/S2 | Dos casos de salida +contextos/repetición/intercalado | LED bajo y buzzer alto; pines ajenos preservados; sin caché |
| DG2/S3 | Composición B07 con funciones reales de dispositivo | Acepta ambas órdenes, contextos ignorados, selector sin afectar |
| DG3/S4 | Host/sanitizadores/ARM/Doxygen/aislamiento/hashes | Conforme por modalidad; sin ensayo eléctrico ni inicialización |

El doble host reemplaza sólo funciones de la API GPIO y modela valores de puertos;
no ejecuta ni modifica el driver original, no simula CPU/bus/latencias/electricidad.
Pruebas detectan traducción de polaridad, selección errónea de pin y contaminación
de otras señales; no reproducen simplemente las llamadas de producción.

## Recursos y revisión

Objetos propios ARM-O0: Selector.text30, LED40, Buzzer34, total104 bytes.
ARM-O2:16/20/16,total52 bytes. Todos .data/.bss0. Marcos .su propios8/16/16
bytes-O0 y8/8/8-O2; no cota total de pila porque excluyen driver/caller/IRQ.
Sin heap, estado mutable propio, ISR, esperas, caché ni nuevo módulo genérico.
Selector depende sólo de GPIO_ReadValue; las salidas sólo de GPIO_SetPinState.
El firmware mínimo sigue sin llamarlos; GC elimina funciones no utilizadas.
No se valida aún enlace del conjunto de biblioteca ni inicialización física, por
instrucción expresa de dejarlos al final. No tocar Drivers para resolver integración.

Fuentes: UM10360 rev4.1 tabla84/p120 y GPIO §9.5/9.6.2; esquema OM13085 revD
hoja2 para LEDP0.22 bajo. Selector/buzzer altos y pinesP2 sin EINTx fueron autorizados.
Revisión física de placa/cableado, dirección/polarización/FIOMASK y estado real
permanecen pendientes. Mantener uso privilegiado serializado y configuración estable.

Reproducción: cmake -S "$R" -B "$O/host" -DCMAKE_BUILD_TYPE=Debug;
cmake --build "$O/host" --parallel 4; ctest --test-dir "$O/host" --verbose.
San: carpeta propia y -DTPI_HOST_SANITIZERS=ON, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1,
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM: -DTPI_BUILD_ARM=ON y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake". Extras exactos en
extra-commands.json. Doxygen usa configuración vigente con salida externa.

No simulación MCU, pruebas en placa, programación, instalación ni publicación.
Próxima acción: revisión/aceptación del resultado B12, antes de promover contratos.
