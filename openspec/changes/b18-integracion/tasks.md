# Tareas B18 — integración

## I1 — Arranque y periféricos

- [x] 1.1 Leer ESTADO/CONTEXTO, plan de integración, specs vigentes, biblioteca
  original y UM10360 rev. 4.1 (cap. 4, 5, 6, 8, 9, 14, 21, 24, 29, 31).
- [x] 1.2 `Reloj_Init` (soporte): FLASHCFG a 5 ciclos, `SystemInit`,
  `SystemCoreClockUpdate`; startup sin cambios; `verificar_elf.py --exigir-reloj`.
- [x] 1.3 `Motor_Init`: PWM1.1/P2.0, PR = 24, MR0 = 1000 con reset, MR1 = 0.
- [x] 1.4 `SensorOptico_Init`: TIMER1 contador de flancos ascendentes en CAP1.0/P1.18 con pull-up.
- [x] 1.5 `Timer_Init` ampliado, `TIMER0_IRQHandler` y `Timer_ConsumirVentana`.
- [x] 1.6 UART0: `UART0_Init`, buffers circulares RX 64/TX 128, `UART0_IRQHandler`,
  cebado de transmisión; `UART_BAUDRATE` por defecto 9600.
- [x] 1.7 `Adquisicion_ADC_DMA_Init`, `DMA_IRQHandler`, `HayDatos`/`ContarPares`;
  `ADC_VREF_MV` por defecto 3300.
- [x] 1.8 `Selector_Init`, `LED_Init`, `Buzzer_Init`.
- [x] 1.9 Build ARM con las fuentes usadas de la biblioteca (flags propios) y puente
  `plataforma/compat/lpc17xx.h`; fixture `cmake/verificar_enlace_i1.cmake` Debug/Release.
- [x] 1.10 Pruebas host: inicializaciones, buffers UART, ventana de Timer, ADC/DMA;
  actualización de `test_uart.c`, `test_motor.c`, `test_sensor_optico.c` y
  `test_dispositivos_gpio.c`.
- [x] 1.11 Doxygen de cada función/ISR con capítulo/tabla del manual; Doxyfile ampliado.
- [x] 1.12 Delta specs de I1, verify-report y ESTADO.
- [ ] 1.13 Revisión de Claude del diff contra UM10360 y commit local.

## I2 — Servicios simples, Aplicación y main

- [ ] 2.1 Pendiente; se detalla al iniciar I2.
- [ ] 2.2 `main` llama `Reloj_Init` como primera acción; activar `--exigir-reloj`
  en el firmware (post-build y `b00.imagen` de `CMakeLists.txt`).

## I3 — Proyecto MCUXpresso

- [ ] 3.1 Pendiente; se detalla al iniciar I3. Agregar el puente
  `plataforma/compat/lpc17xx.h` a las rutas de inclusión y no definir `__USE_CMSIS`
  (el reloj lo configura `Reloj_Init`).
