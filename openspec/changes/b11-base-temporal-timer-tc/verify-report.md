# Verificación B11 — corrección de drivers y nombres

Fecha 2026-11-10. Base main/86107b9, revisión3 del candidato TC1–TC5/S1–S8.
Implementado/verificado; resultado pendiente de aceptación, sin sincronizar/archivar/push.
Commit normal local autorizado: `refactor: simplificar Timer y reutilizar drivers LPC17xx`;
consultar Git para hash. Evidencia anterior preservada en
`git show 86107b9:openspec/changes/b11-base-temporal-timer-tc/verify-report.md`.

## Resultado y alcance

Un solo dispositivo Timer.h/.c, cuatro APIs Init/Read/Stop/Source. Eliminados puerto
y adaptador LPC1769. TIM_InitFreeRunning amplía el driver existente sin alterar las
APIs anteriores; TIM_Enable/Disable/ReadTimer y CLKPWR_ConfigPPWR se reutilizan.
Servicio Tiempo, main y contratos aceptados intactos. Default1ms configurable,
normalización ms modular, prioridades de error, parada/épocas y semirrango conservados.

## Comprobaciones realmente ejecutadas

Logs/artefactos externos: `/tmp/ed3-b11-drivers-20261110-qhso3a72`. C11 con -Wall -Wextra -Werror -Wconversion
-Wshadow -Wstrict-prototypes; pruebas/fixture además -pedantic-errors. Ningún control
desactivado. GCC host13.3, ARM13.2.1, CMake/CTest3.28.3, Doxygen1.9.8.

| Modalidad/control | Resultado | Evidencia externa |
|---|---|---|
| Consumidores antes del reemplazo | RED: Timer.h ausente | red.log |
| Host Debug | 21/21, 212 casos C; driver real y registros modelados | host-tests-final.log |
| ASan/LSan/UBSan | 21/21 sin hallazgos, fugas habilitadas fuera de ptrace | san-tests-final.log |
| ARM Debug | Build y15/15 PASS; después del assert TC se repitió B11 2/2 | arm-tests.log, arm-b11-final.log |
| ARM Release | Build y15/15 PASS, B11 repetido2/2 | release-tests.log, release-b11-final.log |
| Doxygen HTML/XML estricto | Sin avisos;38 funciones propias únicas, seis Timer/Tiempo | doxygen.log, doxygen-functions.json |
| Mutantes en copias externas | Detecta PR fuera de uno, lectura que borra TC y cambio PCLK | extra-results.json, *-build/run.log |
| Configuración | Bases1/2/5 compilan/ejecutan; cero rechazado al compilar | config-*-build/run.log |
| Aislamiento/preservación | Tiempo sin símbolos indefinidos; módulos/contratos aceptados intactos | final-results.json, Tiempo-undefined.log |
| Inspección ARM | APIs fuertes, IRQ SysTick/TIMER0 débiles por defecto; sin indefinidos; CHECK_PARAM vigente | *-b11-final.log, *-check-assert.log |

212 C =179 previos +16 TC +9 Tiempo +5 integración +3 driver.
Las 21 entradas CTest host son20 ejecutables C y un oráculo Python. Regresiones:
velocidad11199 ternas, codec5096 pares, selector4096 muestras/400 vueltas.
Oráculo temporal:2048 muestras/1235 vueltas nativas TC, comparación con tiempo absoluto.
El nuevo test del driver recorre cuatro timers; comprueba PR0/UINT32_MAX, estado
detenido, limpieza de match/captura, preservación de reloj/MRx y enable/read/disable.
Regresión TIM_InitTimer en ticks y microsegundos, con frecuencias distintas por timer;
la máscara PCLK se comprueba en bits0 y30. CLK/PINSEL/check_failed son dobles host;
en ARM se enlazan TIM/CLKPWR/check_failed reales. No se ejecuta firmware ARM.

## Matriz de aceptación

| Contrato/escenario | Evidencia ejecutada | Resultado |
|---|---|---|
| TC1/S1 | ConfiguracionYProteccion, Bases, PrescalerExtremos, FreeRunning y overrides | Prescaler exacto/default1, PR0/MAX, bases2/3/5, períodoINT32_MAX, registros ajenos intactos |
| TC1/S2 | RelojYPeriodoInvalidos, PrescalerInvalido, PrioridadYFueraRango | Prioridades, invalidación/reset/parada y recuperación |
| TC2/S3 | CuentaSinConsultas, ConsultasNoAlteran, DosConsumidores | Avance sin consultas/ISR y lecturas sin reiniciar TC |
| TC2/S4 | SinInicializar, DetenerCongela, ReinicioYRecuperacion, NullYProveedor, ParadaIdempotente | Cero/ausencia/error, parada y nuevas épocas |
| TC3/S5 | Nueve casos wrapper compilado sólo con Servicios | Contexto exacto, una llamada, argumentos/fallo sucio/recuperación |
| TC4/S6 | RolloverTC/MS, OraculoTiempoAbsoluto, IntervaloRollover, Semirrango, PrioridadFuente | Unidades ms, rollover y límite INT32_MAX; fuente precede rango |
| TC5/S7 | Cinco casos Timer→Tiempo→Mandos | Bases1/2/5, frontera20ms, base3 con rollover, rebotes/fallo y reinicio de consumidor |
| TC5/S8 y aclaración drivers | LegacyTick/Microseconds, FreeRunning, host/san/ARM/Doxygen/diff | API corta y driver existente; comportamiento previo comprobado sin promover aceptación física |

## Recursos y revisión

Fixture con APIs completas: huella Flash3684 bytes Debug,2988 Release; SRAM2108
en ambos, incluyendo reserva de pila2048. Main mínimo sigue sin iniciar timer;
elimina módulos no usados por GC (Flash844 Debug). No confundir fixture con integración.
Objetos propios: Timer .text570/.bss5 Debug y .text324/.bss5 Release; Tiempo
.text192/152 y .data/.bss0. Textos de objetos no incluyen driver/libgcc/alineación.
Marcos .su Timer_Init56/24 y Timer_Read24/16 bytes Debug/Release; no son una cota
total de pila ni medición física. Inicialización mantiene división64 de libgcc;
lectura usa multiplicación32 y una carga TC en TIM_ReadTimer, sin escrituras MMIO.
Driver conserva comprobación de puntero: no afirmar ausencia de llamadas/stack en
TIM_ReadTimer, que incluye rama de fallo check_failed. No hay ISR/acumulador temporal.
SystemInit/SystemCoreClock/CLKPWR_SetPCLKDiv no quedan enlazados en fixture B11.

Revisión: Aplicacion/Servicios/Configuracion/plataforma, specs vigentes, archivo
aceptado y candidato SysTick intactos contra86107b9. Licencias del driver intactas;
extensión nueva más correcciones de portabilidad: includes LPC17xx.h, mapeo de
punteros con comparaciones, máscaras unsigned, inicializadores openDrain=DISABLE
explícitos (antes cero implícito), argumentos de check_failed marcados usados.
B00 S7 conserva19 objetos/14casos; cambia sólo nombre Base_Tiempos.c.o→Timer.c.o.

Incidencias resueltas: biblioteca inicialmente no compilaba con controles actuales
por include en minúsculas, máscaras signed/UL64, campo openDrain implícito y parámetros
sin uso. ARM detectó máscara PCLK signed; corregida a UINT32_C(3). Builds finales
verdes sin suprimir advertencias/aserciones ni cambiar configuración sensible de placa.

## Reproducción y límites

Desde raíz R, salida externa O: `cmake -S "$R" -B "$O/host" -DCMAKE_BUILD_TYPE=Debug`,
`cmake --build "$O/host" --parallel 4`, `ctest --test-dir "$O/host" --verbose`.
San: directorio propio, -DTPI_HOST_SANITIZERS=ON, ASAN_OPTIONS=detect_leaks=1:halt_on_error=1,
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM: directorio propio,
-DTPI_BUILD_ARM=ON -DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake"; Debug/Release.
Doxygen: configuración docs/doxygen/Doxyfile con OUTPUT_DIRECTORY externo; XML
deduplicado por nombre para contar38 funciones (header/source pueden repetir entradas).

No simulación MCU, ejecución ARM, hardware, flasheo ni instalación. Modelo host no
acredita latencias, atomicidad de bus, reloj/electricidad ni pausa durante debug/sleep.
25MHz es entrada de prueba, no reloj confirmado. TIMER0/PCONP/configuración requieren
exclusividad/serialización desde lazo privilegiado. Marcas misma época y duración real
<2^31ms; no detectar épocas mezcladas/vueltas omitidas. Falta PCLK/precisión físicos,
composición main, adquisición y ventanaRPM1s. Próximo paso: revisión/aceptación B11;
sin sincronizar/archivar/push ni iniciar otro bloque hasta orden.
