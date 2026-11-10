# Verificación B11 TC/Tiempo — 2026-11-10

**PASS para alcance host/compilación**, resultado pendiente de aceptación.
Alcance acordado el 2026-11-10; base db88286afb8a02dc855765a2a138c3a9280f1ede.
11/11 tareas completas; commit normal local autorizado, sin push, asunto
`feat: reemplazar SysTick por TIMER0 y wrapper temporal`. Contrato candidato
TC1–TC5/S1–S8; no sincronizado/archivado. No reescribir historia anterior.

## Ejecución

Logs/comandos/artefactos externos: /tmp/ed3-b11-tc-20261110-j1_ngpam. Snapshot incluye los cambios locales de
planificación que ya existían. Evidencia SDD SysTick conservada; sus temporales no
estaban disponibles en esta sesión y no se usan para acreditar el nuevo diseño.
Versiones realmente ejecutadas en version-*.log. C11 estricto: -Wall -Wextra -Werror
-Wconversion -Wshadow -Wstrict-prototypes; tests/fixture además -pedantic-errors.
Sin instalar dependencias ni desactivar controles.

| Verificación ejecutada | Resultado | Logs externos |
|---|---|---|
| Nuevos consumidores antes de implementar | RED esperado, API/configuración ausentes o incompatibles | tc-red.log, tiempo-red.log |
| Suites nuevas individuales | GREEN: 16 dispositivo, nueve wrapper, cinco integración | *-green-build/run.log |
| Host Debug completo | 20/20; 209 C, oráculo temporal 2048 muestras/1235 vueltas TC | host-configure/build/ctest.log |
| ASan/LeakSanitizer/UBSan completo | 20/20, sin hallazgos; fugas habilitadas fuera de ptrace | san-configure/build/ctest.log, san-options.txt |
| Refuerzo de extremos posterior | TC host/san recompilado y repetido 1/1, 16 casos y mismo oráculo | tc-host/san-build/ctest.log |
| ARM Debug build/CTest final | 15/15; B00 conserva 14 casos y fixture B11 válido | arm-build.log, arm-ctest-final.log |
| Fixture ARM API completa Debug/Release | PASS, APIs retenidas, sin indefinidos, GNU_STACK RW, sin ISR propia | arm-b11-debug-final.log, arm-b11-release-final.log |
| Doxygen estricto HTML/XML | 42 funciones/USW, diez de dispositivo/Tiempo, sin avisos | doxygen.log, doxygen-warnings.log, doxygen-audit.json |
| PR fuera de uno en copia | Compila y falla CuentaSinConsultas | prescaler-fuera-uno-build/run.log |
| Lectura que reinicia TC en copia | Compila y falla ConsultasNoAlteran | consulta-reinicia-tc-build/run.log |
| Overrides de base 2/5 ms | Compilan/ejecutan y conservan ms; override cero falla compilación | config-*-build/run.log, config-cero-negativo.log |
| ISR propia en fixture negativo | Compila; verificador detecta violación | isr-propia-build.log, isr-propia-negativo.log |
| Inspección/aislamiento/recursos | TC es lectura única, wrapper sin dependencia física, preservación PASS | lectura-tc-*.txt, recursos.json, preservacion.json |

Regresiones ejecutadas: velocidad 11199 ternas, codec 5096 pares y selector
4096 muestras/400 rollovers en host/san. Total 179 C aceptados +16 TC +9 Tiempo +5
integración =209; 20 CTest host son 19 ejecutables C y oráculo Python B03-R1.
Tras pase completo se añadieron sólo aserciones de período INT32_MAX válido y
proveedor sin inicializar; se recompiló/repitió únicamente TC host/san 1/1.
Código productivo sin cambios posteriores, excepto aclaraciones documentales.

Incidencias resueltas, sin ocultar fallos:
- Primer ARM tuvo 14/15: B00 S7 exigía 18 objetos, ahora Tiempo agrega el 19.
  Se adaptó test_objetos_host para exigir conjunto independiente de los 19 nombres
  y cantidad, conservando control de startup y los 14 casos. Repetición ARM final 15/15.
- Primer Release detectó Base_Tiempos_Leer eliminada por inline/GC al usarse sólo
  indirectamente. Fixture agregó llamada explícita; Debug/Release repetidos PASS.
  No se relajó el verificador ni se cambió lógica productiva para retener símbolos.

Reproducción desde raíz R y salida externa O: host
`cmake -S "$R" -B "$O/host" -DTPI_BUILD_ARM=OFF -DCMAKE_BUILD_TYPE=Debug`,
`cmake --build "$O/host" --parallel 2`, `ctest --test-dir "$O/host" --verbose`.
San usa carpeta independiente y -DTPI_HOST_SANITIZERS=ON; ejecutar fuera de ptrace,
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 y
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1.
ARM usa -DTPI_BUILD_ARM=ON y -DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake";
build/CTest iguales. Release: cmake -P cmake/verificar_enlace_b11.cmake con SOURCE_DIR,
OUTPUT_DIR, TOOL_PREFIX, PYTHON y BUILD_TYPE=Release. Doxygen usa configuración vigente
con OUTPUT_DIRECTORY/WARN_LOGFILE externos. Comandos exactos de cada paso en *.json/log.

## Cumplimiento por escenario

| Contrato | Pruebas realmente ejecutadas | Resultado |
|---|---|---|
| TC1/S1 | ConfiguracionYProteccion, Bases, PrescalerExtremos, overrides | PASS: default1, 2/3/5 ms, PR0/MAX, período INT32_MAX, registros ajenos preservados |
| TC1/S2 | RelojYPeriodoInvalidos, PrescalerInvalido, PrioridadYFueraRango | PASS: prioridad reloj/período/prescaler, división inexacta/overflow, invalidación/recuperación |
| TC2/S3 | CuentaSinConsultas, ConsultasNoAlteran, DosConsumidores | PASS: hardware modelado avanza sin ISR/lecturas, consultas sin efectos y marcas independientes |
| TC2/S4 | SinInicializar, DetenerCongela, ReinicioYRecuperacion, NullYProveedor, ParadaIdempotente | PASS: ausencia/cero/error, parada/reinicio y proveedor sólo activo |
| TC3/S5 | Nueve casos test_tiempo, standalone sólo Servicios | PASS: NULL/función, contexto exacto, llamada única, fuente sucia/fallida y recuperación |
| TC4/S6 | RolloverTC/MS, OraculoTiempoAbsoluto, IntervaloRollover, Semirrango, PrioridadFuente | PASS: ms consistentes, doble rollover, frontera INT32_MAX/semirrango, fuente antes de intervalo |
| TC5/S7 | Cinco casos integración TIMER0→Tiempo→Mandos | PASS: bases1/2/5, frontera20ms, rollover con base3, rebotes, fallo y época/consumidor nuevos |
| TC5/S8 | Modelos/CTest/ARM/negativos/Doxygen/dependencias/preservación | PASS por modalidad; sin aceptación física |

Modelo host compila el backend real contra dobles CMSIS; avanza PC/TC según PCLK/PR
sin llamar ISR/API. Oráculo independiente parte de tiempo absoluto, redondea a la
resolución y compara 2048 snapshots a través de 1235 vueltas completas de TC.
No se afirma que un modelo de registros simule CPU/bus/latencias/electricidad.
25 MHz en tests/fixture es ejemplo de PCLK, no frecuencia confirmada de la placa.

## Recursos y revisión

ARM -O0: texto dispositivo 426 bytes, puerto 204 más helpers CMSIS 104, servicio
192; total propio/con helpers CMSIS 926, excluye libgcc y alineación de imagen.
.data nueva cero; .bss privada dispositivo5 bytes antes de alineación (período/estado),
servicio sin .data/.bss. Ningún acumulador temporal software ni ISR propia.
Inicialización usa __aeabi_uldivmod de libgcc existente para aritmética acotada;
lectura usa multiplicación uint32_t modular y puerto con una carga de palabra TC.
Objeto Tiempo sin indefinidos; dependencias sólo Tiempo.h/std, sin MCU/dispositivo.

Pila estática para proveedor concreto: cadena Transcurrido→Leer→proveedor→lectura→TC
108 bytes Debug antes de llamador/IRQ. Init tiene marco56 más 48 del camino libgcc
(16 __aeabi_uldivmod +32 __udivmoddi4, sin llamadas adicionales por divisor1000).
Release: cota de caminos conocidos80 incluyendo libgcc de init. No acota proveedores
arbitrarios, IRQ anidadas ni mide pila física. Datos en .su y desensamblados.
Fixture Debug Flash3116/SRAM2108 bytes; Release2568/2108; incluyen reserva pila2048.
Main mínimo Flash844/SRAM2048: no inicia timer; linker descarta módulos sin uso.

Release LeerTC: mov dirección 0x40004000, ldr palabra offset8 y retorno, sin stores/
llamadas. Init conserva bits PCONP ajenos bajo precondición de alimentación
serializada; no toca PLL/PCLKSEL/flash/pines. TIMER0 sin match/captura/IRQ; handlers
SysTick/TIMER0 conservan alias por defecto. Registros sólo en dispositivos.
Se retiran .17.4–.8 del candidato SysTick; dispositivo conserva .17.1/.2/.3,
agrega .17.9–.13; wrapper ESW.2.1.18 agrega .18.1/.2 (extensiones locales).

Servicios previos/aplicación/main/Drivers/startup/linker/tests C aceptados, contratos
vigentes y archivos aceptados preservados por hash. Única adaptación de control
anterior: B00 S7, de conteo18 a lista exacta19 por servicio añadido. Historial SDD
SysTick conserva diseño/spec/tareas/informe; planificación local previa incorporada.

## Límites y siguiente acción

Sin ejecución ARM, simulación MCU, placa, flasheo, instalaciones ni cambios sensibles
del MCU. Precisión, PCLK real, suspensión/debug y aceptación física pendientes.
Mayor período reduce resolución; marcas siguen en ms modulares. Misma época,
duración real <2^31 ms y gestión PCONP serializada son precondiciones. No detecta
todas las vueltas omitidas ni mezcla de épocas; inicializar requiere marcas nuevas.
Sin integración main/adquisición/ventanaRPM1s. Próxima acción: revisar resultado B11;
sin sincronizar/archivar, push o bloque posterior hasta orden del usuario.
