# Verificación B18 — integración

## I1 — Arranque y periféricos

Fecha: 2026-11-11. Rama `integracion-i1`, base 6072762, cambios sin commit.
Entorno: Linux, cc 13.3.0, arm-none-eabi-gcc 13.2.1, CMake 3.28.3, Doxygen 1.9.8.
Salidas de build fuera del árbol (`<out>` = directorio temporal de la sesión).
**Sólo host y compilación/enlace ARM. Nada se ejecutó en MCU, simulador ni placa.**

### Comandos y resultados

| Modalidad | Comando (desde la raíz) | Resultado |
|---|---|---|
| Host + ASan/UBSan | `cmake -S . -B <out>/host -DTPI_BUILD_ARM=OFF -DTPI_HOST_SANITIZERS=ON && cmake --build <out>/host -j && ctest --test-dir <out>/host --output-on-failure` | 36/36 suites, 0 avisos de compilación |
| Host sin sanitizadores | ídem sin `-DTPI_HOST_SANITIZERS=ON`, en `<out>/host-nosan` | 36/36 suites |
| ARM Debug | `cmake -S . -B <out>/arm -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-none-eabi.cmake && cmake --build <out>/arm -j && ctest --test-dir <out>/arm --output-on-failure` | 17/17 controles |
| ARM Release | ídem con `-DCMAKE_BUILD_TYPE=Release`, en `<out>/arm-release` | 17/17 controles |
| Doxygen | `{ cat docs/doxygen/Doxyfile; echo 'OUTPUT_DIRECTORY = <out>/doxygen'; } \| doxygen -` | 0 avisos; 62 funciones documentadas (46 previas + 16) |
| Biblioteca | `git status --short -- Drivers` y `git diff --stat -- Drivers` | sin cambios |

Las 33 suites host previas siguen pasando; se agregan `i1.timer`, `i1.reloj` e
`i1.adquisicion_adc_dma`. Los 15 controles ARM previos siguen pasando; se agregan
`i1.enlace_arm` (-O0) e `i1.enlace_arm_release` (-O3).

Casos C de las suites tocadas: `b13.uart` 13, `i1.timer` 11,
`i1.adquisicion_adc_dma` 7, `i1.reloj` 6 valores iniciales, `b15.motor` 5, `b14.sensor_optico` 7,
`b12.dispositivos_gpio` 11.

Mutación manual en una copia fuera del árbol (no versionada), hecha antes de las
correcciones de revisión: 32 mutantes de Timer, UART, ADC/DMA, Motor, Sensor y GPIO;
32 detectados (31 por las suites, 1 por `-Werror` al compilar). No se repitió
después de agregar `Reloj_Init` ni del cambio de P1.18.

### Evidencia estática ARM

- `firmware.elf` Debug: FLASH 2812 bytes, SRAM local 2280 bytes (incluye 2048 de
  pila). Contiene las 3 ISR porque la tabla de vectores las referencia; `main`
  sigue vacío hasta I2 y por eso el firmware todavía no llama `Reloj_Init` ni se
  verifica con `--exigir-reloj` (queda como tarea 2.2 de I2).
- El startup no cambió respecto de la base: no llama `SystemInit`.
- Fixture I1: `verificar_elf.py --exigir-reloj` aprueba en -O0 y -O3. En el
  desensamblado de `Reloj_Init` se ve `bic #0xf000`, `orr #0x4000`, `str` a
  FLASHCFG, luego `bl SystemInit` y la llamada a `SystemCoreClockUpdate` (`bl` en
  -O0, `b.w` de cola en -O3); `Reloj_Init` es la primera llamada de main.
  Control negativo manual: el mismo verificador rechaza `firmware.elf` y el fixture
  B17 con "Falta simbolo fuerte de reloj: Reloj_Init".
- Fixture I1: text 15112 (-O0) / 10056 (-O3), data 36, bss 2328 (con pila). Todas
  las APIs e ISR presentes como símbolos fuertes; vectores 17, 21 y 42 apuntan a
  `TIMER0_IRQHandler`, `UART0_IRQHandler` y `DMA_IRQHandler`; `ADC_IRQHandler`,
  `TIMER1_IRQHandler`, `PWM1_IRQHandler` y `UART1_IRQHandler` siguen débiles; sin
  símbolos indefinidos, sin `__aeabi_f*`/`__aeabi_d*`, sin libc.

### Cambios en controles existentes

- `scripts/verificar_elf.py`: opción nueva `--exigir-reloj` (símbolos fuertes
  `Reloj_Init`/`SystemInit`/`SystemCoreClockUpdate`, escritura previa y orden de
  llamadas en `Reloj_Init`, `Reloj_Init` primera llamada de main). Sin la opción el
  comportamiento es el anterior; hoy sólo la usa el fixture I1.
- `scripts/pruebas_b00.py`: único cambio, `Reloj.c.o` agregado al conjunto esperado
  de objetos host. La afirmación de ausencia de `SystemInit` en el fixture de
  arranque se conserva. Ningún control fue desactivado.
- `tests/host/test_uart.c`: reescrito. Los casos de B13 modelaban
  `UART_Send`/`UART_Receive`, que el dispositivo ya no llama; se conservan S1–S5 sobre
  los buffers y se agregan inicialización, ISR, cebado, lleno y vueltas de índice.
  La lectura/escritura de 256 bytes de una sola vez deja de aplicar: los buffers son
  de 64 y 128 bytes.
- `tests/host/test_motor.c`, `test_sensor_optico.c`, `test_dispositivos_gpio.c`: se
  agregan dobles de las funciones de biblioteca que ahora referencia cada `.c` y un
  caso de inicialización; los casos previos no cambian.
- `scripts/verificar_b17.py` no se modificó: es la reproducción archivada de B17 y
  codifica sus cifras (46 funciones Doxygen). No forma parte de ctest.

### Valores de registro esperados (por lectura de la biblioteca, no medidos)

| Periférico | Valores |
|---|---|
| Reloj | FLASHCFG[15:12] 4 antes del PLL (`Reloj_Init`); PLL0CFG 0x00050063, CCLKCFG 3, PCLKSEL0/1 0 (de `system_LPC17xx.c`) |
| PWM1 | PR 24, MR0 1000, MR1 0, MCR 0x002, PCR 0x200, TCR 0x09, PINSEL4[1:0] 01 |
| TIMER1 | CTCR 0x01, TCR 0x01, PINSEL3[5:4] 11, PINMODE3[5:4] 00 (pull-up) |
| TIMER0 | PR 24999, MR0 500 (+500 por ventana), MCR 0x001 |
| UART0 | DLL 93, DLM 0, FDR 0x43, LCR 0x03, FCR 0x07, TER 0x80, IER 0x03, PINSEL0[7:4] 0101, PINMODE0[7:6] 00 y [5:4] 10 |
| ADC | AD0CR = PDN \| CLKDIV 255 \| SEL 0x03 \| BURST, AD0INTEN 0x003, PINSEL1[17:14] 0101, PINMODE1[17:14] 1010 |
| GPDMA canal 0 | SrcAddr &AD0DR0, DestAddr &ADC_DMA_Pot, Control 0x00480001, LLI → descriptor con Control 0x80480001; Config: ITC, P2M, SrcPeripheral 4 (DestPeripheral 4, ignorado con destino en memoria) |
| NVIC | TIMER0 prioridad 1, UART0 2, DMA 3; ADC deshabilitada |

Divisores de UART calculados reproduciendo en Python el algoritmo de
`uart_set_divisors` con PCLK 25 MHz: 9600,6 baudios (+0,006 %).

### Riesgos y dudas respecto del manual (sin verificar en placa)

1. **FLASHCFG:** resuelto en software. `SystemInit` original lo escribe después de
   conectar el PLL0 y el valor de reset (4 ciclos) sólo vale hasta 80 MHz (UM10360
   tabla 49); `Reloj_Init` lo deja en 5 ciclos antes de llamar a `SystemInit`.
   Orden verificado en host (`i1.reloj`) y por desensamblado en el fixture. Falta
   la comprobación en placa (`SystemCoreClock == 100000000`).
2. **`SystemInit` también configura PLL1 (USB) y espera su enganche**, y escribe
   PCONP = 0x042887DE. No se usa USB; no se modificó.
3. **`GPDMA_SetupChannel` desplaza por una cantidad negativa** para pedidos menores
   que 8 (`1 << (srcConn - 8)` con ADC = 4): comportamiento indefinido en C. El
   implementador inspeccionó `GPDMA_ConfigDMAReqSel` en el fixture -O0: `lsl.w` por
   registro. Claude inspeccionó el fixture Release: dentro de `GPDMA_SetupChannel`
   los desplazamientos de DMAREQSEL también quedan como `lsl.w` por registro. En
   ARM un corrimiento de 32 o más da 0, así que DMAREQSEL no cambia. El
   comportamiento indefinido sigue siendo un riesgo residual de la biblioteca, no
   corregible sin tocar `Drivers/`.
4. **Pedido DMA del ADC en ráfaga:** se siguió UM10360 29.6.4 (pedido = línea de
   interrupción, ADGINTEN = 0, interrupción del ADC deshabilitada en NVIC). Falta
   confirmar en placa que cada DONE de canal produce una transferencia y que el
   contador de pares avanza a ~763/s.
5. **PWM:** con MR0 = 1000 y reset por MR0 el periodo real es de 1001 ticks
   (≈999 Hz), y el mando 1000 (MR1 = MR0) deja un pulso bajo de 1 µs por ciclo
   (UM10360 24.4.1). Aceptado; pendiente de osciloscopio, igual que MR1 = 0.
6. **P2.0 y P2.7 son entradas con pull-up desde el reset hasta `Motor_Init` y
   `Buzzer_Init`:** la etapa de potencia y el buzzer ven un nivel alto débil durante
   el arranque del PLL.
7. **THRE de UART0:** el cebado no depende de que el periférico genere THRE al
   habilitarla; falta confirmar en placa la salida de la trama completa.
8. **MCUXpresso (I3):** no definir `__USE_CMSIS` (si `cr_startup` llamara a
   `SystemInit`, `Reloj_Init` reconfiguraría el PLL0 ya conectado) y en Linux hace
   falta el puente `plataforma/compat/lpc17xx.h` en las rutas de inclusión.
9. **P1.18 con pull-up interno:** niveles y alimentación del módulo XD-51 sin
   verificar; si su salida llega a 5 V revisar la hoja de datos del pin.

### Erratas pendientes de revisar

No hay hoja de erratas local (ES_LPC176x). Antes de la puesta en marcha revisar en
ella, sin que este informe afirme su contenido: ADC en modo ráfaga con DMA y
exactitud del ADC; UART (THRE/FIFO y divisor fraccional); PWM (errata PWM.1 citada
en B15, mando igual a MR0); PLL0/arranque del oscilador principal.

## I2 — Servicios simples, Aplicación y main

Fecha: 2026-11-11. Rama `integracion-i2`, base ff07e3e (I1), cambios sin commit.
Entorno: el de I1. Salidas de build fuera del árbol (`<out>`).
**Sólo host y compilación/enlace ARM. Nada se ejecutó en MCU, simulador ni placa.**

### Comandos y resultados

| Modalidad | Comando (desde la raíz) | Resultado |
|---|---|---|
| Host + ASan/UBSan | `cmake -S . -B <out>/host -DTPI_BUILD_ARM=OFF -DTPI_HOST_SANITIZERS=ON && cmake --build <out>/host -j && ctest --test-dir <out>/host --output-on-failure` | 29/29 suites, 0 avisos de compilación |
| Host sin sanitizadores | ídem sin `-DTPI_HOST_SANITIZERS=ON`, en `<out>/host-nosan` | 29/29 suites |
| ARM Debug | `cmake -S . -B <out>/arm -DTPI_BUILD_ARM=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm-none-eabi.cmake && cmake --build <out>/arm -j && ctest --test-dir <out>/arm --output-on-failure` | 9/9 controles, 0 avisos |
| ARM Release | ídem con `-DCMAKE_BUILD_TYPE=Release`, en `<out>/arm-release` | 9/9 controles, 0 avisos |
| Doxygen | `{ cat docs/doxygen/Doxyfile; echo 'OUTPUT_DIRECTORY = <out>/doxygen'; } \| doxygen -` | 0 avisos; 60 funciones documentadas |
| Preservación | `git status --short -- Drivers`; `git diff --quiet ff07e3e -- Drivers Dispositivos plataforma Configuracion/Control.h tests/host/test_regulador.c tests/host/consulta_regulador.c tests/host/verificar_regulador.py tests/host/verificar_configuracion_control.py` | sin cambios |

Suites host (29): B17 `b17.regulador` (10 casos), `b17.oraculo_planta`,
`b17.configuracion_control`; I2 `i2.capas`, `i2.sistema` (3), `i2.mandos` (2),
`i2.medicion_velocidad` (3), `i2.temperatura` (2), `i2.temperatura_lm35` (1),
`i2.drivers` (2), `i2.comunicacion` (6), `i2.procesamiento` (5), `i2.aplicacion`
(8); dispositivos sin cambios de alcance: `b05.lm35`, 5 × `b05r1.lm35_vref_*`,
`b05r1.oraculo_lm35`, `b05r1.configuracion_lm35`, `b12.dispositivos_gpio` (11),
`b13.uart`, `i1.timer`, `i1.reloj`, `i1.adquisicion_adc_dma`, `b14.sensor_optico`
(7), `b15.motor`, `b16.potenciometro`.
Controles ARM (9): `b00.imagen` (con `--exigir-reloj`), `i2.firmware`, `i2.capas`,
`b00.contratos` (15 pruebas, 1 nueva), `b17.enlace_arm`, `b17.enlace_arm_release`,
`i1.enlace_arm`, `i1.enlace_arm_release`, `b05.lm35_enlace_arm`.

De 36 a 29 suites host y de 17 a 9 controles ARM: bajan porque se eliminan las
suites y fixtures de las APIs retiradas (ver "Eliminados"), no porque se desactive
algún control vigente.

### Escenarios de la prueba de sistema (`i2.aplicacion`)

main.c, Procesamiento.c y los 7 servicios reales, con los dispositivos simulados
de `tests/host/fakes_dispositivos.*` (no simulan la biblioteca). `main.c` se
compila con `-Dmain=Aplicacion_MainFirmware`; la prueba llama
`Aplicacion_Inicializar` y `Aplicacion_Iterar`.

| Caso | Qué comprueba |
|---|---|
| S1 | Orden exacto de inicialización; arranque en AUTO con mando 0 y alarma apagada; sin tramas ni escrituras entre ventanas; `RPM 100` → AUTO_ACTIVO con mando 400 y luego 100; tramas exactas; 50 pulsos → `RPM=100.000`, 1 pulso → `RPM=2.000` |
| S2 | La consigna del periodo de entrada a AUTO y la línea parcial pendiente se descartan; el potenciómetro no actúa en AUTO; una consigna posterior activa |
| S3 | MANUAL: potenciómetro 0/2048/4095 → mando 0/500/1000 entre periodos; consignas ignoradas; `ESTADO=MANUAL` |
| S4 | AUTO → MANUAL → AUTO: vuelve a ESPERA con mando 0, sin reutilizar la consigna, y con el PI reiniciado (400, no 700) |
| S5 | Histéresis sobre el promedio de 4: 45,00 no enciende; 45,01 enciende; 44,00 y 43,04 siguen; 43,00 apaga; 45,00 no reenciende; alarma activa no detiene el motor y funciona en AUTO |
| S6 | `RPM 186`, `RPM x`, línea larga, `RPM 1000`, minúsculas se ignoran; `RPM 0` es válida (AUTO_ACTIVO, mando 0); gana la última consigna válida del periodo |
| S7 | Consigna 185 con eje detenido: el mando llega a 1000 y no lo supera |
| S8 | Fallo de `Sistema_Init`: retorno false, motor 0, alarma encendida, resto sin inicializar |

### Firmware ARM

| Build | FLASH (imagen) | SRAM local (.data + .bss + pila de 2048) | text / data / bss |
|---|---|---|---|
| Debug (-O0 -g) | 19 636 / 524 288 bytes (3,7 %) | 2 456 / 32 768 bytes (7,5 %) | 19 040 / 36 / 2 420 |
| Release (-O3) | 13 124 / 524 288 bytes (2,5 %) | 2 448 / 32 768 bytes (7,5 %) | 12 528 / 36 / 2 412 |

Datos de `memoria.txt` (post-build de `verificar_elf.py`) y de `firmware.map`:
.text en FLASH desde 0x300, .data/.bss en SRAM local desde 0x10000000, pila de
2 kB en 0x10007800; AHB0/AHB1 (32 kB) sin uso. Entra en el LPC1769 (512 kB de
flash, 32 kB + 32 kB de SRAM). La profundidad real de pila no se midió.

- Cadena de arranque comprobada por desensamblado en ambos builds: primera
  llamada de main `Aplicacion_Inicializar`, cuya primera llamada es `Sistema_Init`,
  cuya primera llamada es `Reloj_Init` (seguida de `Adquisicion_ADC_DMA_Init`).
  En Release `Aplicacion_Iterar` queda en línea dentro de main.
- `i2.firmware`: `TIMER0_IRQHandler`, `UART0_IRQHandler` y `DMA_IRQHandler` fuertes
  en los vectores 17, 21 y 42; `ADC/TIMER1/PWM1/UART1_IRQHandler` débiles; sin
  símbolos indefinidos, sin `__aeabi_f*`/`__aeabi_d*`, sin libc. De libgcc sólo
  entra la división entera de 64 bits (`__aeabi_uldivmod`/`__aeabi_ldivmod`), que
  usan `Medicion_Velocidad_Actualizar` y el PI de B17.

### Control de capas

`i2.capas` revisa 54 inclusiones de `Aplicacion/` y `Servicios/`. Control negativo
manual en una copia fuera del árbol: agregar `#include "Motor.h"` a `main.c` falla
con "Aplicacion no puede incluir "Motor.h" de Dispositivos"; agregar
`#include "Procesamiento.h"` a `Mandos.c` falla con "Servicios no puede incluir".

### Mutación manual (copia fuera del árbol, no versionada)

48 mutantes, uno por vez, con build host sin sanitizadores y `ctest`: 45 detectados
y 3 equivalentes.

| Grupo pedido | Mutantes | Detectan |
|---|---|---|
| Histéresis (umbral de apagado) | sin histéresis; `<` por `<=`; `>=` por `>`; histéresis 100 | 4/4: `i2.procesamiento`, `i2.aplicacion` |
| Descarte de consigna al entrar en AUTO | aceptar la consigna del mismo periodo; sin `Comunicacion_Descartar`; Descartar sin vaciar línea parcial; sin vaciar RX; MANUAL sin borrar consigna; sin ningún reinicio del PI | 6/6: `i2.procesamiento`, `i2.comunicacion`, `i2.aplicacion` |
| Saturación del mando a 1000 | sin límite; límite 999; límite 1001 | 3/3: `i2.drivers` (y `i2.aplicacion` el de 999) |
| Parser de consigna (rango 185) | `>=` 185; sin rango; rango 186; Procesamiento sin rango; Procesamiento `<` 185; 4 dígitos; sin CR opcional; línea larga sin descartar | 8/8: `i2.comunicacion`, `i2.procesamiento`, `i2.aplicacion` |
| Fórmula de RPM | constante 6000; sin saturación; producto en 32 bits; 30 pulsos por vuelta; Actualizar siempre true | 5/5: `i2.medicion_velocidad`, `i2.aplicacion` |
| Otros | redondeo y polaridad de Mandos; divisor y filtro de Temperatura; decimales, signo y retorno de telemetría; manual sólo por periodo; potenciómetro fuera de MANUAL; primera consigna en vez de última; sin mando 0 inicial; fallo de arranque (alarma, continuar); orden de `Medicion_Velocidad_Init`; estado en telemetría; alarma atada al modo; `Sistema_Init` sin espera o sin mirar el DMA; alarma sólo LED | 19/19 |

Equivalentes (el comportamiento no cambia, por redundancia del diseño):
1. Quitar sólo el reinicio del PI al entrar en AUTO: `AUTO_ESPERA` lo reinicia en
   el mismo periodo.
2. Quitar sólo el reinicio del PI en `AUTO_ESPERA`: el de la entrada alcanza, porque
   a ESPERA sólo se llega desde MANUAL o desde el arranque. Quitar ambos sí se detecta.
3. Quitar el `mando = 0` cuando `Procesamiento_Regular` no devuelve VALIDO ni
   RETENIENDO: B17 ya deja el mando en 0 en esos casos.
El mutante de 4 dígitos (`RPM 0100`) sobrevivió en la primera pasada; se agregó el
caso a `i2.comunicacion` S2 y quedó detectado.

### Decisiones fuera del diseño detallado

1. `verificar_elf.py --exigir-reloj` exigía que `Reloj_Init` fuera la primera
   llamada *directa* de main. Con Aplicación sin Dispositivos eso es imposible:
   ahora sigue la cadena de primeras llamadas (hasta 3 niveles). El fixture I1
   (llamada directa) sigue aprobando y un main sin reloj se sigue rechazando
   (`b00.contratos`, prueba nueva `test_reloj_exigido`).
2. `Aplicacion/main.h` nuevo, con `Aplicacion_Inicializar` y `Aplicacion_Iterar`,
   para que la prueba host pueda declararlas; `main()` se renombra sólo en la
   prueba, por opción de compilación.
3. Fallo de `Sistema_Init`: el diseño sólo fijaba el retorno. Se deja el motor en
   0, se enciende la alarma (LED y buzzer) como aviso y `main()` queda detenido.
4. `Procesamiento_InicializarModo` devuelve el estado inicial (el diseño decía
   `void`), para que main no tenga que deducirlo.
5. `Comunicacion_Init` también vacía la línea parcial.
6. `Temperatura_Leer` promedia con división entera truncada; con muestras
   positivas el error es menor que 0,01 °C hacia abajo.
7. Se agregó un delta `regulacion-velocidad` (sólo RV5/S7: quién compone) y se
   ampliaron los deltas `build` (R6, S11, R7/S12) y `sensor-optico` (SO3), que
   citaban composiciones eliminadas.
8. `Sistema_Init` sin USW: soporte de plataforma sin ESW, igual que `Reloj_Init`.

### Eliminados y cambios en controles existentes

- Servicios y aplicación: API anterior completa de los 6 servicios, B01
  (`Procesamiento_Inicializar/Actualizar`) y B06 (`EvaluarAlarmaTemperatura`).
- Suites host eliminadas (APIs retiradas): `b01.modos`, `b02.comunicacion`,
  `b03.medicion_velocidad`, `b03.oraculo_velocidad` (`verificar_velocidad.py`,
  `consulta_medicion_velocidad.c`), `b04.mandos`, `b05.temperatura`,
  `b06.alarma_temperatura`, `b07.driver_alarmas`, `b07.integracion_alarmas`,
  `b08.driver_motor`, `b08.integracion_motor`, `b09.telemetria`,
  `b09.integracion_telemetria`, `b10.selector`, `b10.integracion_selector`,
  `b17.integracion_regulador` (reemplazada por `i2.aplicacion`).
  `b05.integracion_temperatura` pasa a `i2.temperatura_lm35`, reescrita sobre
  `Temperatura_Leer` con `LM35.c` real.
- Fixtures ARM eliminados: `verificar_enlace_b01..b10.cmake` (10). Conservados:
  `b05_lm35` (ahora usa `Temperatura_Leer`), `b17` (reducido a inicializar/regular/
  reiniciar, -O0 y -O3) e `i1` (sin cambios).
- `scripts/pruebas_b00.py`: `Sistema.c.o` agregado al conjunto esperado de objetos
  host; `verify()` acepta opciones extra; prueba nueva `test_reloj_exigido`.
  Ningún control desactivado; `test_fixture_data_bss_y_llamada_main` conserva la
  ausencia de `SystemInit` en el startup.
- `tests/host/test_sensor_optico.c`: los casos S3/S4 y dos más llamaban al
  servicio B03 eliminado con duración y factor explícitos; conservan la
  comprobación de la diferencia modular de pulsos y pierden la conversión a RPM,
  que ahora cubre `i2.medicion_velocidad`. 7 casos, igual que antes.
- `tests/host/test_dispositivos_gpio.c`: el caso S9 llama al
  `Driver_Alarmas_Aplicar(bool)` nuevo, con `LED.c`/`Buzzer.c` reales. 11 casos.
- `scripts/verificar_b17.py` no se modificó ni forma parte de ctest: es la
  reproducción archivada de B17 y codifica cifras de entonces (33 suites, 15
  controles ARM, 46 funciones); hoy fallaría por esas cifras.

### Sin verificar o con dudas

1. Nada físico: reloj, PWM, UART, ADC/DMA, sensor, LED y buzzer quedan para I4.
2. `SISTEMA_ESPERA_ADC_CONSULTAS` = 2 000 000 se eligió por cuenta de ciclos
   estimada (≥ 100 ms a 100 MHz frente a ~2 ms del primer par); el tiempo real del
   límite no se midió y depende de la optimización.
3. En MANUAL el mando se escribe en cada vuelta del lazo (miles de veces por
   segundo): `Motor_Write` actualiza MR1 y LER cada vez. Por lectura del manual
   sólo se aplica el último valor en el próximo ciclo de PWM; falta confirmarlo con
   osciloscopio.
4. Si el buffer de transmisión no tiene lugar, `UART_Write` encola una trama
   truncada y `Comunicacion_EnviarTelemetria` devuelve false; main ignora el
   retorno. Con 53 bytes cada 500 ms a 9600 baudios (960 bytes/s) y buffer de 128
   no debería ocurrir; no se ensayó con una PC que frene la línea (no hay control
   de flujo).
5. El buffer de recepción es de 64 bytes y se vacía una vez por periodo: más de 64
   bytes recibidos en 500 ms se pierden en el dispositivo (comportamiento de I1).
6. La alarma por fallo de arranque (decisión 3) no distingue su causa de una
   alarma térmica; es sólo un aviso de que el sistema no arrancó.
7. Sintonía del PI y sensor desconectado: fuera de alcance, como en B17.

## I3 — Proyecto MCUXpresso

Fecha: 2026-11-11. Rama `integracion-i3`, base e3a279c (I1+I2), cambios sin commit.
Entorno: Linux, MCUXpresso IDE 25.6.136 (arm-none-eabi-gcc 14.2.1 propio), y el de
I1/I2 para CMake (arm-none-eabi-gcc 13.2.1, Doxygen 1.9.8). Salidas fuera del árbol.
**Sólo compilación y enlace sin interfaz gráfica y revisión estática del `.axf`. No se
abrió la IDE, no se importó por la interfaz, no se conectó la sonda ni la placa.**

### Comandos y resultados

| Modalidad | Comando | Resultado |
|---|---|---|
| MCUXpresso Debug y Release | `rsync -a --exclude .git ./ "<out>/copia con espacios/Firmware/"` y luego `/usr/local/mcuxpressoide-25.6.136/ide/mcuxpressoide -nosplash --launcher.suppressErrors -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data <out>/ws -import "<out>/copia con espacios/Firmware" -cleanBuild all` (sin `DISPLAY`) | salida 0; `Build Finished. 0 errors, 0 warnings` en Debug y en Release; 33 fuentes por configuración |
| Estabilidad del proyecto | `cmp` de `.cproject` y `.project` de la copia contra los del árbol, después de importar y compilar | idénticos: la IDE no los reescribe |
| `.axf` Debug y Release | `python3 scripts/verificar_elf_mcuxpresso.py <copia>/<cfg>/TPI_ED3_Tacometro.axf --tool-prefix /usr/local/mcuxpressoide-25.6.136/ide/tools/bin/arm-none-eabi-` | correcto en ambos |
| Host + sanitizadores | el de I2 | 29/29 suites, 0 avisos |
| ARM Debug / Release (CMake) | los de I2 | 9/9 y 9/9 controles, 0 avisos; tamaños iguales a los de I2 |
| Doxygen | el de I2 | 0 avisos; 60 funciones (Doxyfile y fuentes sin cambios) |
| Biblioteca | `git status --short -- Drivers` | sin cambios |

La ruta de la copia lleva espacios a propósito, como la del repositorio.

`verificar_elf_mcuxpresso.py` comprueba sobre el `.axf`: sin símbolos indefinidos;
palabra 0 = `_vStackTop` = 0x10008000; vector de reset = `ResetISR`; checksum de los 8
primeros vectores; vectores 17, 21 y 42 = `TIMER0_IRQHandler`, `UART0_IRQHandler` y
`DMA_IRQHandler` con definición fuerte, y el resto de las IRQ en `IntDefaultHandler`;
`CRP_WORD` en 0x2FC con 0xFFFFFFFF; `ResetISR` llama a `main` y no a `SystemInit`;
cadena `main` → `Aplicacion_Inicializar` → `Sistema_Init` → `Reloj_Init` con la misma
función `require_clock` de `verificar_elf.py`; sin `__aeabi_f*`/`__aeabi_d*`, `printf`,
`malloc`, `free` ni `_sbrk`. `verificar_elf.py` no se aplica tal cual: exige los
símbolos del arranque propio (`Reset_Handler`, `__stack_top__`…).

### Tamaños

| Imagen | Compilador y optimización | text | data | bss | Flash | SRAM |
|---|---|---|---|---|---|---|
| MCUXpresso Debug | gcc 14.2.1, `-O0` | 18 996 | 36 | 356 | 19 032 | 396 (sin pila reservada) |
| MCUXpresso Release | gcc 14.2.1, `-Os` | 10 616 | 36 | 356 | 10 652 | 396 (sin pila reservada) |
| CMake Debug | gcc 13.2.1, sin `-O` | 19 040 | 36 | 2 420 | 19 636 | 2 456 (con 2 048 de pila) |
| CMake Release | gcc 13.2.1, `-O3` | 12 528 | 36 | 2 412 | 13 124 | 2 448 (con 2 048 de pila) |

Las cifras de MCUXpresso salen de `-print-memory-usage` y `arm-none-eabi-size`. No son
comparables una a una: cambian el compilador, la optimización de Release, el arranque y
que el script administrado no reserva una sección de pila (crece desde 0x10008000).

### Decisiones

- **NewlibNano sin E/S en lugar de Redlib.** Con Redlib el código propio no compila:
  su `stdint.h` define `UINT32_C` con una conversión de tipo y fallan los `#if` de
  `Timer.h`, `UART.h` y `Analogica.h` («missing binary operator before token "int"»).
  Se probó y se descartó. `ResetISR` llama entonces a `main` directamente.
- **Sin `DEBUG` en la configuración Debug.** Ese símbolo activa `CHECK_PARAM` en la
  biblioteca; CMake no lo define y se mantiene el mismo comportamiento.
- **`-isystem` además de `-I` para la biblioteca y `plataforma/compat`.** Las rutas
  figuran como inclusión normal (para el indexador) y como `-isystem "../…"` en *Other
  flags*, relativas a la carpeta de compilación; gcc trata entonces esos encabezados
  como de sistema y no generan avisos con `-Werror`, igual que `SYSTEM` en CMake.
- **Opciones por carpeta.** `Drivers` y `plataforma/mcuxpresso` compilan sin `-Wextra
  -Werror -Wconversion -Wshadow -Wstrict-prototypes`; les queda el `-Wall` que agrega
  la IDE. `cr_startup_lpc175x_6x.c` declara `__valid_user_code_checksum()` sin
  prototipo, y no se modifica.
- **Arranque y CRP sin cambios.** Coinciden byte a byte (sha256) con los del asistente
  `Wizards/System/NXP/LPC175x_6x/Common` de la IDE. Llevan el aviso de licencia de NXP.
- **`.cproject` normalizado por la IDE.** Se escribió a mano a partir de la plantilla
  `LPCX176x_cmsis2_timers` y de las opciones de `com.crt.advproject` 25.6; el archivo
  versionado es el que la propia IDE reescribió en la primera importación. La
  descripción del LPC1769 (`com.crt.config`) sale de `ide/binaries/nxp_lpc17xx.xml`.
- **`.settings/` y `*.launch` no se versionan.** `language.settings.xml` lleva un hash
  del entorno de cada máquina. El `.launch` de LinkServer lo crea la IDE en la primera
  depuración; no se escribió uno a mano.
- **`plataforma/cmsis_compat.c` se compila también aquí**: sólo contiene
  comprobaciones estáticas y no choca con el arranque de la IDE.

### Sin verificar o con dudas

1. La importación por la interfaz gráfica y que el proyecto no pida ninguna
   configuración manual. La importación sin interfaz usa el mismo mecanismo.
2. La depuración: detección de la sonda, creación del `.launch`, programación de la
   flash y parada en `main`. La sección `com.crt.config` (controlador de flash
   `LPC175x_6x_512.cfx`, memoria y periféricos) se armó sin la IDE y sólo se comprobó
   que el enlace generado usa el mapa de memoria correcto y que `${TargetChip}` vale
   LPC1769.
3. En Release (`-Os`) el enlace toma `memset` de NewlibNano; en Debug no. Además
   quedan `__aeabi_uldivmod`/`__aeabi_ldivmod` de libgcc, como en CMake.
4. El script administrado no reserva pila ni la comprueba; el build CMake reserva
   2 048 B. La profundidad real de pila no se midió en ninguno de los dos.
5. El usuario no pertenece al grupo `dialout`: para abrir `/dev/ttyUSB0` del conversor
   USB-UART hará falta agregarlo o usar otro medio (no se cambió nada del sistema).
6. Todo lo físico de I1/I2 sigue pendiente (I4): lista en `docs/MCUXPRESSO.md`.

## I5 — Regulador simple y redundancias de I2

Fecha: 2026-11-11. Rama `integracion-i5`, base a5a888e. Mismo entorno que I3.
Pedido del usuario: simplificar el regulador de B17 al estilo de I2 y quitar las
redundancias que dejó I2 en `main.c`, antes de la puesta en marcha.
**Sólo host y compilación/enlace ARM. Nada se ejecutó en MCU, simulador ni placa.**

### Qué cambia

- `uint16_t Procesamiento_Regular(uint32_t consigna_rpm, uint32_t rpm_milesimas)`
  devuelve el mando. La integral es una estática de `Procesamiento.c`; las ganancias
  salen de `Control.h`. Las líneas de la cuenta (error, mando, redondeo, anti-windup,
  límites de la integral) son las de B17 con los campos de la instancia reemplazados
  por la estática y las macros.
- Se retiran `Procesamiento_InicializarRegulador`, `Procesamiento_ReiniciarRegulador`,
  los tipos `Procesamiento_Regulador`/`ConfigRegulador`/`EstadoRegulador`, la
  retención de mediciones no válidas, `TPI_CONTROL_MAX_PERIODOS_SIN_MEDICION` y
  `Medicion_Velocidad_Estado`/`Medicion_Velocidad_Lectura`.
- La integral se borra con consigna 0, en `Procesamiento_InicializarModo` y con el
  modo MANUAL. `main.c` pierde los dos reinicios del PI, el `mando = 0` posterior
  al regulador y la variable `Aplicacion_Regulador`.
- Comportamiento del firmware: sin cambios para toda secuencia de I2 (los escenarios
  S1–S7 de `i2.aplicacion` pasan sin modificar). Una consigna mayor que 185 ya no
  es un error del regulador: `Procesamiento_ActualizarModo` la filtra antes.

### Comandos y resultados

| Modalidad | Resultado |
|---|---|
| Host + ASan/UBSan | 29/29 suites |
| Host sin sanitizadores | 29/29 suites |
| ARM Debug | 9/9 controles; firmware 18 212 B de text, 36 de data, 2 384 de bss |
| ARM Release | 9/9 controles; firmware 12 084 B de text, 36 de data, 2 388 de bss |
| Doxygen | 0 avisos; 58 funciones documentadas (60 − 2) |
| MCUXpresso sin interfaz | Debug y Release: 0 errores, 0 avisos; `.axf` 18 204 B y 10 256 B de flash, 360 B de SRAM sin pila; `verificar_elf_mcuxpresso.py` pasa en ambos |
| Biblioteca y dispositivos | `Drivers/`, `Dispositivos/` y `plataforma/` sin cambios |

Los comandos son los de I1–I3. El número de suites no cambia: `b17.regulador`,
`b17.oraculo_planta`, `b17.configuracion_control` y los dos `b17.enlace_arm`
conservan el nombre.

### Pruebas del regulador

- `b17.regulador` (`test_regulador.c`, reescrito, caja negra): 7 casos con las
  ganancias por defecto: paso y empate, anti-windup, mando exactamente cero,
  consigna cero, borrado por el modo y extremos de 32 bits.
- `b17.oraculo_planta`: 8 emisores, uno por juego de ganancias (`-D` sobre
  `Control.h`), contra la referencia en fracciones: 66 160 pasos exactos
  (8 × 8 224 aleatorios más los deterministas). Los juegos con Ki > Kp
  (1/1000 con 1000/1, y 1/7 con 5/3) son los que alcanzan los límites 0 y 1000 de
  la integral: con Ki ≤ Kp, como en los valores por defecto, el anti-windup actúa
  antes y esos límites no se alcanzan. Planta de prueba: sobrepico 0 y 200
  milésimas de RPM, establecimiento 14 y 14 pasos (igual que en B17).
- `b17.configuracion_control`: 4 configuraciones válidas y 18 inválidas.
- Se pierde respecto de B17: la comparación directa de la integral (ahora se
  observa por el mando de los pasos siguientes) y todo lo relativo a punteros
  nulos, configuración en ejecución y retención, que ya no existen.

### Mutación manual (copia fuera del árbol, no versionada)

17 mutantes, uno por vez, build host sin sanitizadores y `ctest`: 16 detectados y
1 equivalente.

| Mutante | Detectan |
|---|---|
| Truncar en vez de redondear | `b17.regulador`, `b17.oraculo_planta` |
| Sin límite superior / sin límite inferior de la integral | `b17.oraculo_planta` |
| Sin anti-windup; sólo sin el superior; sólo sin el inferior; `n < 0` por `n <= 0` | `b17.regulador`, `b17.oraculo_planta` |
| `n > límite` por `n >= límite` en el anti-windup | `b17.oraculo_planta` |
| Consigna 0 sin borrar la integral | `b17.regulador`, `b17.oraculo_planta`, `i2.aplicacion` |
| `InicializarModo` sin borrar la integral | `b17.regulador` |
| MANUAL sin borrar la integral | `b17.regulador`, `i2.aplicacion` |
| Ki con el numerador de Kp; integrar antes de calcular; mando máximo 999 | `b17.regulador`, `b17.oraculo_planta`, `i2.aplicacion` |
| `main` sin `Comunicacion_Descartar`; AUTO_ACTIVO sin regular | `i2.aplicacion` |

Equivalente: regular también en AUTO_ESPERA. En ese estado la consigna siempre es
0 y el regulador devuelve 0. Se conserva la comprobación explícita del estado por
claridad.

Los tres equivalentes de I2 (doble reinicio del PI y `mando = 0` posterior)
desaparecen con el código que los causaba.

### Sin verificar o con dudas

1. Nada físico. Las ganancias por defecto siguen siendo provisionales.
2. Con el sensor desconectado la medición es 0 válida y el PI sube el mando hasta
   1000: fuera de alcance por decisión del usuario, igual que antes.
