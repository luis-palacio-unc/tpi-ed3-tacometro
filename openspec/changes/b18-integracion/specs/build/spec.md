# Delta B18 — build

Candidato de la vía rápida B18. B00-R1, B00-R2, B00-R3 y B00-R4 sin cambios: el
startup sigue sin configurar reloj ni llamar `SystemInit`. Sólo sincronizar al
cerrar la integración.

## ADDED Requirements

### Requirement B18-R5: Biblioteca original enlazada

El build ARM MUST compilar y enlazar las fuentes usadas de la biblioteca original
(`system_LPC17xx.c`, `lpc17xx_adc/clkpwr/gpdma/gpio/libcfg_default/pinsel/pwm/timer/
uart.c`) sin modificarlas y con opciones propias, sin relajar `-Wall -Wextra -Werror
-Wconversion -Wshadow -Wstrict-prototypes` del código propio. Como dos encabezados
de la biblioteca incluyen `lpc17xx.h` en minúsculas, el build MUST aportar el puente
`plataforma/compat/lpc17xx.h` como ruta de inclusión de sistema, fuera de `Drivers/`.

#### Scenario S9: Fixture de enlace I1
- GIVEN un main de prueba que llama `Reloj_Init` y luego todas las inicializaciones
  y APIs de dispositivos
- WHEN se compila y enlaza con la biblioteca y el startup, en -O0 y en -O3
- THEN el ELF MUST pasar `--exigir-reloj`; MUST definir como símbolos fuertes
  `TIMER0_IRQHandler`, `UART0_IRQHandler` y `DMA_IRQHandler`, con los vectores 17, 21
  y 42 apuntando a ellos; MUST conservar débiles las ISR no usadas; MUST NOT contener
  símbolos indefinidos ni auxiliares `__aeabi_f*`/`__aeabi_d*`; no se ejecuta en MCU.

### Requirement B18-R6: Arranque único del reloj

`Dispositivos/soporte/Reloj.c/.h` MUST exponer `void Reloj_Init(void)`, único punto
que configura el reloj para el startup propio y para MCUXpresso. MUST, en orden:
escribir FLASHCFG = (FLASHCFG & ~0x0000F000) | 0x00004000 (5 ciclos, UM10360 tabla
49) antes de tocar el PLL; llamar `SystemInit` original (CCLK = 100 MHz con cristal
de 12 MHz, PCLK = CCLK/4); llamar `SystemCoreClockUpdate`. MUST conservar los bits
11:0 de FLASHCFG. main MUST llamarla antes de cualquier inicialización (I2); en
MCUXpresso MUST NOT definirse `__USE_CMSIS`. `verificar_elf.py --exigir-reloj` MUST
fallar si `Reloj_Init`, `SystemInit` o `SystemCoreClockUpdate` no son símbolos
fuertes, si `Reloj_Init` no escribe antes de llamar a `SystemInit` y luego a
`SystemCoreClockUpdate`, o si la primera llamada de main no lleva a `Reloj_Init`:
directamente (fixture I1) o como primera llamada de la primera función llamada,
hasta 3 niveles (I2: main → `Aplicacion_Inicializar` → `Sistema_Init` →
`Reloj_Init`, porque Aplicación no incluye Dispositivos). La opción MUST aplicarse
al fixture I1 y, desde I2, al post-build del firmware y a `b00.imagen`.

#### Scenario S10: Orden de FLASHCFG
- GIVEN LPC_SC redirigido a RAM con FLASHCFG 0x303A, 0x003A, 0xF03A, 0x403A,
  0xFFFFFFFF y 0, y dobles de `SystemInit`/`SystemCoreClockUpdate`
- WHEN se llama `Reloj_Init`
- THEN ambos dobles MUST observar FLASHTIM = 4 con el resto de bits intacto, y
  ejecutarse una vez cada uno en ese orden.

#### Scenario S11: Imagen sin arranque de reloj (`b00.contratos`)
- GIVEN un fixture con main vacío, sin biblioteca, y el firmware de I2
- WHEN se verifican con `--exigir-reloj`
- THEN el fixture MUST rechazarse por falta de `Reloj_Init` y aprobar sin la
  opción; el firmware MUST aprobar con ella.

### Requirement B18-R7: Firmware integrado (I2)

El firmware ARM MUST enlazar `main` real, Aplicación, Servicios (incluido
`Sistema.c`), Dispositivos y la biblioteca original, en Debug y Release. El build
host MUST compilar los mismos fuentes como objetos (`Sistema.c.o` incluido) y las
pruebas MUST enlazar `main.c` con `main` renombrado, sin modificar el fuente.
Se retiran los fixtures ARM `verificar_enlace_b01..b10` de APIs eliminadas; se
conservan `b05.lm35_enlace_arm`, `b17.enlace_arm*` (sólo regulador) e `i1.enlace_arm*`.

#### Scenario S12: Firmware I2 (`i2.firmware`, `b00.imagen`)
- GIVEN `firmware.elf` Debug o Release
- WHEN se verifica
- THEN MUST pasar `--exigir-reloj`; MUST definir fuertes `TIMER0_IRQHandler`,
  `UART0_IRQHandler` y `DMA_IRQHandler` en los vectores 17, 21 y 42; MUST conservar
  débiles las ISR no usadas; MUST NOT contener símbolos indefinidos, `__aeabi_f*`/
  `__aeabi_d*` ni libc; MUST caber en FLASH 512 kB y SRAM local 32 kB.
