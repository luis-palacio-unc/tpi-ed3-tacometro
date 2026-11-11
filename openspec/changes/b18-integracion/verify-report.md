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

## I2

Pendiente.

## I3

Pendiente.
