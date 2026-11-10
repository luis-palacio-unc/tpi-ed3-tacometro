# Verificación B12 — dispositivos GPIO

2026-11-10. Corrección autorizada del resultado de4260249, pendiente de aceptación.
Commit local autorizado; sin push/sincronización/archivo ni avance a otro bloque.

## Contrato comprobado

Selector_Read (.13.1) conserva lectura de P2.6 alto mediante GPIO_ReadValue.
LED_Write (.14.1) y Buzzer_Write (.15.1) ahora son void(bool activar): sin contexto
ni retorno ficticio. GPIO_SetPinState conserva LEDP0.22 activo bajo y buzzerP2.7 alto.
Headers/cuerpos con Doxygen; no lectura de confirmación ni diagnósticos inventados.
Sin cambios en Drivers, servicios, main, Timer/LM35 o contratos aceptados.

Driver_Alarmas/B07 conserva bool(void*,bool). Esas firmas no admiten directamente
las salidas nuevas: su revisión queda pendiente por instrucción del usuario.
Se retiraron ContextosIgnorados (API eliminada) y ComposicionAlarmas (contrato por
revisar), y el target GPIO ya no enlaza Driver_Alarmas. No se reemplazaron por
adaptadores/casts ni se deshabilitaron las pruebas independientes B07, que pasan.
Los siete casos directos conservan polaridad, bits ajenos, repetición e intercalado.

## Evidencia ejecutada para esta corrección

Logs fuera de Git: `/tmp/ed3-b12-salidas-simples-ns5j3oo1`.
Build C11 estricto -Wall -Wextra -Werror -Wconversion -Wshadow
-Wstrict-prototypes; pruebas/extras también -pedantic-errors. Sin controles relajados.

| Control/modalidad | Resultado | Log externo |
|---|---|---|
| Host Debug completo |18/18;186 casos C (179 previos+7 B12) y oráculo Python |host-*.log, host/Testing/Temporary/LastTest.log |
| ASan/LSan/UBSan |18/18, sin hallazgos, detect_leaks=1 fuera de ptrace |san-*.log |
| ARM Debug build y controles |13/13; compilación, sin ejecutar MCU |arm-*.log |
| Dos salidas ARM-O0/O2 |Sólo símbolo externo GPIO_SetPinState |*-build/symbols/size.log |
| Headers LED/Buzzer C11/C++17 |Aislados sin headers CMSIS, correctos |*-header-*.log |
| Doxygen HTML/XML estricto |38 funciones únicas, sin avisos; ambas firmas void(bool), sin @return |doxygen.log |
| Negativos en copias externas |Detecta inversión de polaridad de LED y buzzer |*-negativo-*.log |
| Preservación |224 archivos protegidos intactos;90 Drivers iguales a a21cc1e; B07 intacto |before.json, preservation.json, diff |

Casos B12: SelectorNiveles, SelectorIgnoraOtrosBits (64 patrones),
SelectorIgnoraOtrosPuertos, LedActivoBajo, BuzzerActivoAlto,
RepeticionRestauraSalida y SalidasIntercaladas. Salidas sobre0/MAX/alternos,
preservando otros pines/puertos y el selector vecino. Tests invocan void(bool).
El doble host reemplaza API GPIO; no ejecuta biblioteca ni simula CPU/electricidad.

## Matriz de escenarios candidatos

| Contrato | Prueba ejecutada | Resultado |
|---|---|---|
| DG1/S1 lectura aislada |Tres casos selector |Conforme |
| DG2/S2 polaridad/aislamiento |LedActivoBajo/BuzzerActivoAlto |Conforme |
| DG2/S3 repetición/independencia |RepeticionRestauraSalida/SalidasIntercaladas |Conforme |
| DG3/S4 evidencia y preservación |Host/san/ARM/Doxygen/hashes/diff |Conforme por modalidad |

## Recursos y límites

Objetos de las dos salidas: LED.text36/Buzzer30 bytes-O0;16/12-O2; .data/.bss0.
Marcos propios16/16 bytes-O0,0/0-O2 por llamada de cola; no cota de pila total
porque excluyen driver/caller/IRQ. Sin heap/estado/ISR/esperas/caché.
Selector no cambió; su medición previa permanece en evidencia de4260249.

Inicialización GPIO/PINSEL/dirección/polarización/FIOMASK, enlace global y uso
desde main permanecen diferidos. Acceso privilegiado serializado sobre configuración
estable. No diagnóstico eléctrico/ensayo físico ni garantía antes de configurar.
Firmware mínimo descarta por GC las funciones no llamadas; el enlace completo de
la biblioteca original sigue pendiente, sin cambiar Drivers para resolverlo.
No simulación MCU, programación, instalación o publicación. Revisión física/cableado pendientes.

Reproducción: cmake -S "$R" -B "$O/host" -DCMAKE_BUILD_TYPE=Debug;
cmake --build "$O/host" --parallel 4; ctest --test-dir "$O/host" --output-on-failure.
San: -DTPI_HOST_SANITIZERS=ON, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 y
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM: -DTPI_BUILD_ARM=ON y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake". Comandos exactos san/arm/extras
en *-commands.json externos. Doxygen usa configuración vigente con salida externa.

Resultado: conforme al alcance corregido; B12 pendiente de aceptación.
Próxima acción: revisar/aceptar B12 y acordar por separado la revisión B07.
Evidencia anterior (API descartada) en /tmp/ed3-b12-gpio-implementacion-m6evumxd;
no usar sus nueve casos/composición B07 como evidencia del contrato actual.
