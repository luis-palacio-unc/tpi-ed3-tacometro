# Verificación B14 — lectura de conteo óptico

2026-11-10. Alcance/TIMER1/CAP1.0/P1.18 autorizados, implementado/verificado;
resultado pendiente de aceptación. Base7409bee, main. Commit normal local
autorizado, sin push/sincronización/archivo ni avance de otros bloques.

## Código y coherencia

SensorOptico_Read devuelve TIM_ReadTimer(LPC_TIM1), una llamada sin aritmética,
estado mutable, contexto, ISR, heap, esperas, init o reset. Cuenta eventos uint32_t
acumulados, incluido cero/MAX, sin convertir a tiempo/RPM ni recortar a185.
Doxygen ESW.2.1.10/USW.2.1.10.1 documenta unidades, retorno, época/rollover,
acceso serializado y precondiciones; ningún comentario dentro del cuerpo.
TIMER0/base temporal y Medicion_Velocidad se conservan; diferencias/ventanas sólo
viven en fixture host, sin integración nueva en producto. Diseño seguido sin desviaciones.

Actualización del usuario: sensor XD-51, reemplaza HC-89 del anteproyecto. Se
registra esa fuente/decisión sin deducir fabricante, variante, pinout, alimentación
o niveles de señal desde el nombre. No se necesita esa información para leer TC;
se verificará la señal del módulo real antes de usar físicamente la entrada.

## Evidencia realmente ejecutada

Logs/objetos externos: `/tmp/ed3-b14-optico-ibtde439`. C11 estricto -Wall -Wextra -Werror -Wconversion
-Wshadow -Wstrict-prototypes; tests/extras también -pedantic-errors. Biblioteca y
controles conservados. Ninguna dependencia instalada ni dispositivo programado.

| Modalidad/control | Resultado | Evidencia externa |
|---|---|---|
| B14 individual |1/1 suite,6 casos |b14-*.log |
| Host completo |20/20,200 casos C (194 previos+6) y oráculo Python, sin omisiones |host-*.log |
| ASan/LSan/UBSan |20/20 sin hallazgos;detect_leaks=1 fuera de ptrace |san-*.log |
| ARM Debug |Build y13/13 controles de imagen/enlace pasan, sin ejecución MCU |arm-*.log |
| Sensor_Optico ARM-O0/O2 |Compila con headers originales; único externo TIM_ReadTimer |optico-*.log |
| Header C11/C++17 aislado |Compila con sólo ruta Dispositivos |header-*.log |
| Doxygen HTML/XML |41 funciones únicas (1 nueva), sin avisos; USW10.1 verificada |doxygen.log, extra-results.json |
| Negativos externos |Detectan leer TIMER0, multiplicar por1000 y recortar185 |*-run.log |
| Preservación |261 archivos protegidos intactos,90 Drivers originales a21cc1e |before.json,preservation.json,diff |

Casos: LecturasExactasYBaseIndependiente, RepeticionYAvance, VentanaUnSegundo,
VentanaCruzaRollover, NuevaEpocaReferenciaExterna y DuracionYFactorExplicitos.
Los valores0/1/60/185/200/MAX detectan confusión de unidad/recurso/recorte; el
fixture compone lecturas con servicio B03 real y expectativas numéricas explícitas.
Regresiones ejecutadas:11199 ternas velocidad,5096 pares codec,4096 muestras/400 vueltas selector.

## Matriz de cumplimiento

| Contrato/escenario | Evidencia ejecutada | Resultado |
|---|---|---|
| SO1/S1 |LecturasExactasYBaseIndependiente: seis extremos/referencias, TIMER0 distinto |Conforme |
| SO2/S2 |RepeticionYAvance; nueva referencia elegida externamente en NuevaEpocaReferenciaExterna |Conforme |
| SO3/S3 |VentanaUnSegundo:0/60/185/200→0/60000/185000/200000 válidos; factor12/duración2s→30000 |Conforme |
| SO3/S4 |MAX-29→30, diferencia modular60 y lecturaB03=60000 |Conforme |
| SO4/S5 |Host/san/ARM/header/Doxygen/hashes/diff; fuente/configuración acordadas |Conforme por modalidad |

5/5 escenarios con evidencia ejecutada, sin umbral de cobertura configurado.
El doble reemplaza sólo TIM_ReadTimer y proporciona valores de conteo; no copia
rutinas de registro/init de biblioteca ni ejecuta driver/CPU/señal eléctrica.
No es una repetición de tests del driver. La referencia nueva tras reset es
responsabilidad del consumidor, demostrada en el fixture, sin detección automática.

## Recursos y límites

Objeto propio .text20 bytes-O0/12-O2, .data/.bss0. Marco propio8 bytes-O0/0-O2,
por llamada de cola; no cota de pila/tiempo incluyendo driver/caller/IRQ. No otros
símbolos externos, helpers o estructuras nuevos. Fuente de firmware sigue18 objetos.

Recurso acordado: TIMER1/CAP1.0/P1.18, PINSEL3 función11 y un flanco ascendente
por ranura. Las condiciones del manual/driver revisadas en exploración permanecen
externas: contador exclusivo/configurado/arrancado, CCR del canal000, sin match
que reinicie/detenga y señal compatible. Para diferencias: misma época y menos
de2^32 eventos entre muestras; vueltas completas omitidas son indistinguibles.
El código no configura pinmux/contador ni fija ventana, factor, timeout o RPM.

La señal física del XD-51 y su cableado/niveles se validarán al integrar, junto con
reloj/PinConfig/InitCounter/Enable, ventana1s, servicios/main y hardware. No afirmar
captura de pulsos real por compilar. Firmware mínimo elimina por GC módulos sin
llamadas; enlace del conjunto de biblioteca sigue pendiente. No ejecución ARM/
placa, simulación MCU, instalación, flasheo o publicación. Drivers, Timer, B03,
aplicación/configuración/otros dispositivos y contratos/archivos aceptados intactos.

Reproducción: cmake -S "$R" -B "$O/host" -DCMAKE_BUILD_TYPE=Debug;
cmake --build "$O/host" --parallel 4; ctest --test-dir "$O/host" --verbose.
San: -DTPI_HOST_SANITIZERS=ON con ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 y
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1. ARM: -DTPI_BUILD_ARM=ON y
-DCMAKE_TOOLCHAIN_FILE="$R/cmake/arm-none-eabi.cmake". Comandos/exits exactos en
host/san/arm/extras-commands.json. Doxygen vigente con OUTPUT_DIRECTORY externo.

Veredicto: PASS para alcance acordado, sin fallos críticos ni desviaciones.
10/10 tareas terminadas; revisar/aceptar B14 antes de sincronizar/archivar.
