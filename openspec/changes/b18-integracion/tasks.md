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
- [x] 1.13 Revisión de Claude del diff contra UM10360 y commit local (ff07e3e).

## I2 — Servicios simples, Aplicación y main

- [x] 2.1 Leer el diseño detallado de I2, el análisis de simplificación, I1 y el
  código/specs vigentes.
- [x] 2.2 `main` llega a `Reloj_Init` como primera acción (main →
  `Aplicacion_Inicializar` → `Sistema_Init` → `Reloj_Init`); `--exigir-reloj` activo
  en el post-build del firmware y en `b00.imagen`; `verificar_elf.py` sigue la
  cadena de primeras llamadas.
- [x] 2.3 Servicios simples que llaman a sus dispositivos: `Sistema` (nuevo),
  `Mandos`, `Medicion_Velocidad`, `Temperatura`, `Driver_Motor`, `Driver_Alarmas`
  y `Comunicacion`, con estado `static` y sin contextos, callbacks ni enums de error.
- [x] 2.4 `Procesamiento`: modos de 3 estados y alarma con histéresis 45/43 °C
  (`Configuracion/Alarma.h`); regulador PI de B17 sin cambios.
- [x] 2.5 `main.c`/`main.h`: `Aplicacion_Inicializar` y `Aplicacion_Iterar`;
  `main()` sólo las encadena. Aplicación sin encabezados de Dispositivos.
- [x] 2.6 Pruebas host con dispositivos simulados (`fakes_dispositivos`): una suite
  por servicio, `i2.procesamiento` y la prueba de sistema `i2.aplicacion`.
- [x] 2.7 Control automático de capas `i2.capas` (`scripts/verificar_capas.py`).
- [x] 2.8 Build: `Sistema.c` en el firmware; fixture `b17` reducido al regulador,
  `b05.lm35` adaptado, `i2.firmware` nuevo; eliminación de suites, oráculos y
  fixtures obsoletos de B01–B10; `pruebas_b00.py` con `Sistema.c.o` y S11 automático.
- [x] 2.9 Doxygen con USW nuevos (sin reutilizar IDs retirados) y Doxyfile ampliado.
- [x] 2.10 Delta specs de I2 (9 dominios nuevos o reemplazados, más build,
  temperatura, sensor-optico y regulacion-velocidad), verify-report, ESTADO y MAPA.
- [x] 2.11 Mutación manual fuera del árbol: 48 mutantes.
- [ ] 2.12 Revisión de Claude del diff y commit local.

## I3 — Proyecto MCUXpresso

- [ ] 3.1 Pendiente; se detalla al iniciar I3. Agregar el puente
  `plataforma/compat/lpc17xx.h` a las rutas de inclusión y no definir `__USE_CMSIS`
  (el reloj lo configura `Reloj_Init`).
