# Delta B18/I1 — build

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
`SystemCoreClockUpdate`, o si `Reloj_Init` no es la primera llamada de main. La
opción MUST aplicarse al fixture I1 y, desde I2, al firmware.

#### Scenario S10: Orden de FLASHCFG
- GIVEN LPC_SC redirigido a RAM con FLASHCFG 0x303A, 0x003A, 0xF03A, 0x403A,
  0xFFFFFFFF y 0, y dobles de `SystemInit`/`SystemCoreClockUpdate`
- WHEN se llama `Reloj_Init`
- THEN ambos dobles MUST observar FLASHTIM = 4 con el resto de bits intacto, y
  ejecutarse una vez cada uno en ese orden.

#### Scenario S11: Imagen sin arranque de reloj
- GIVEN el firmware con main vacío o un fixture lógico sin biblioteca
- WHEN se verifica con `--exigir-reloj`
- THEN MUST rechazarse por falta de `Reloj_Init`; sin la opción MUST seguir aprobando.
