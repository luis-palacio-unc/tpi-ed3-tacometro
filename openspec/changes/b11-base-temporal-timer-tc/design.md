> RETIRADO por el usuario el 2026-11-10. Este documento conserva el trabajo
> descartado como antecedente; no describe implementación ni aceptación actuales.
> Timer/Tiempo eliminados. Definir juntos la responsabilidad del timer antes de continuar.

# Diseño B11 — corrección de drivers y nombres

Origen: aclaración del usuario 2026-11-10 sobre reutilización y nombres cortos.
Sustituye el diseño de 86107b9, cuyo informe/evidencia se conserva en Git.
Contratos TC1–TC5/S1–S8 conservan unidades, límites, fallos y aislamiento.

## Módulos y APIs

Un único dispositivo `Dispositivos/soporte/Timer.h/.c`, ESW.2.1.17:
`Timer_Init(pclk_hz,periodo_ms)`, `Timer_Read(lectura*)`, `Timer_Stop()` y
`Timer_Source(void*,uint32_t*)`. Tipos Timer_Estado/Timer_Lectura, constantes TIMER_.
USW .17.1/.2/.3/.9 conservan roles; retiradas .4–.8 SysTick y .10–.13 puerto.
Se eliminan Base_Tiempos_Puerto y Base_Tiempos_LPC1769: no hay doble capa interna.

Servicios/Tiempo permanece independiente, sin modificaciones: Tiempo_Leer y
Tiempo_Transcurrido usan proveedor, marcas ms y contextos opacos. Aplicación futura
consultará Tiempo; dispositivo sólo conoce driver TIM/CMSIS, nunca servicio.
Main y composición física se implementarán en un bloque posterior.

## Reutilización segura del driver

TIM_InitTimer existente fuerza PCLK=CCLK/4 y TIM_TICK no representa N=2^32.
Agregar TIM_InitFreeRunning(TIMx,PR) al mismo driver, conservando APIs anteriores:
alimenta con CLKPWR_ConfigPPWR; deshabilita; mantiene reset mientras limpia TC/PC,
CTCR/MCR/CCR/EMR e IR=0x3f; configura PR crudo 0..UINT32_MAX; libera reset y deja
detenido. No modifica PCLKSEL/PLL/flash/pines/MRx/otros timers. El llamador controla
NVIC y exclusividad. Reusar TIM_Enable, TIM_Disable y TIM_ReadTimer para operar.
Licencias intactas. Corregir includes `lpc17xx.h` a `LPC17xx.h` en encabezados
usados para Linux. Mapeo de punteros privado con comparaciones portables equivalentes.
Mantener CHECK_PARAM y enlazar check_failed existente; no desactivar aserciones.

Timer_Init valida primero, con prioridad reloj/período/prescaler y producto uint64
acotado; calcula PR=N-1 exacto, PCLK1..120MHz, período1..INT32_MAX, N1..2^32.
Siempre deshabilita IRQ TIMER0, inicializa hardware detenido, limpia pendiente y
sincroniza, incluso si inválido. Sólo configuración válida habilita timer; error
conserva período cero y marca inválida. No hay ISR/contador software.
Timer_Read hace una lectura TIM_ReadTimer y TC*período uint32 modular; no escrituras.
Timer_Stop sólo actúa sobre ACTIVA; proveedor true sólo ACTIVA, incluido cero.
Metadatos/PCONP/configuración serializados desde lazo privilegiado, timer exclusivo.
Default1ms configurable; épocas y semirrango <2^31ms como candidato anterior.

## Verificación

Host compila Timer y fuente real lpc17xx_timer.c contra registros modelados.
Sólo CLKPWR/PINSEL/check_failed son dobles: fallos abortan; la prueba no acredita
CLKPWR físico. Agregar regresión de API anterior tick/us y nuevo modo en cuatro
timers, no cambio de reloj, extremos PR, desactivación de match/captura y lectura.
Mantener 16 casos TC, 9 wrapper independiente, 5 integración B10 y oráculo absoluto.
Host de objetos incluye Timer y encabezados modelados; ARM usa CMSIS/CLKPWR reales.
Verificar sanitizadores, ARM Debug/Release/vectores/indefinidos/recursos, Doxygen
estricto, diff y preservación de contratos aceptados. Logs fuera del repositorio.

Referencias: UM10360 §21; errata NXP ES_LPC176x 10.6 §3.11
https://www.nxp.com/docs/en/errata/ES_LPC176X.pdf (consultada 2026-11-10).
No modificar relojes después de PLL0 conectado. PCLK real/precisión/revisión física
siguen pendientes; 25MHz del modelo es dato de prueba. Sin flasheo/push/archivo.
