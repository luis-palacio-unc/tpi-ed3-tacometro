# B18 — Integración: arranque, periféricos, composición y proyecto

Origen: plan de integración I1–I4 aprobado por el usuario el 2026-11-11 (decisiones
U1–U6, O1–O6 y cambio de alcance de I2). Vía rápida aprobada: un único cambio, sin
propuesta previa ni ronda de revisión aparte; las delta specs se revisan con el diff.

## Alcance

- **I1 (este bloque):** `Reloj_Init` (FLASHCFG, `SystemInit`), inicialización de los periféricos
  de cada dispositivo con la biblioteca original, ISR de TIMER0/UART0/DMA, UART con
  buffers circulares, ventana de medición de 500 ms, adquisición ADC/DMA y enlace
  ARM con la biblioteca.
- **I2:** servicios simples que llaman a sus dispositivos, Aplicación (modos de 3
  estados, alarma con histéresis 45/43 °C, PI de B17 sin cambios) y `main` real.
  Reemplaza los contratos de mandos, medicion-velocidad, temperatura (servicio),
  mando-motor, salidas-alarma, comunicacion, modos y alarma-temperatura, y agrega
  `sistema`. Retira la excepción O5: Aplicación no incluye Dispositivos.
- **I3:** proyecto MCUXpresso (se agrega a este cambio).

## Exclusiones de I1

`Aplicacion/`, `Servicios/` y `main.c` no cambian. `Drivers/` queda byte a byte
intacto. Sin ejecución en placa: la puesta en marcha es I4. No se sincronizan specs
vigentes ni se archiva hasta cerrar la integración.

## Decisiones aplicadas en I1

CCLK 100 MHz (cristal 12 MHz, PCLK = CCLK/4); PWM1.1/P2.0 a 1 kHz con MR0 = 1000;
TIMER1/CAP1.0/P1.18 como contador de flancos ascendentes; TIMER0 libre a 1 ms con
MR0 cada 500 ms sin reset; UART0 P0.2/P0.3 a 9600 8N1 por interrupción (RX 64 B,
TX 128 B); AD0.0/P0.23 y AD0.1/P0.24 en ráfaga con GPDMA canal 0 y lista circular de
dos descriptores; selector P2.6 con pull-up, buzzer P2.7 y LED P0.22; prioridades
NVIC TIMER0 1, UART0 2, DMA 3; `ADC_VREF_MV` = 3300.

## Aceptación verificable de I1

Host con sanitizadores, build ARM Debug/Release con fixture de enlace I1, Doxygen
sin avisos y `Drivers/` sin cambios; ver `verify-report.md`. Nada físico.
