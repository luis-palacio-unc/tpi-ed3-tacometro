# Timer — contrato vigente

Origen R10/R15, aclaraciones y aceptación del usuario el 2026-11-10.
B11 wrapper Timer aceptado/archivado; implementación39ab465, correcciónc543fee.
WT1–WT3 recogen el alcance de B11. Integración de arranque/enlace y medición
física quedan pendientes de bloques posteriores; aceptación no acredita placa.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
aceptación física en placa pendiente.
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

### Requirement: WT1 — Wrappers e interrupción de ventana
Dispositivos/Timer.h/.c MUST exponer void Timer_Init(void), void Timer_Start(void),
uint32_t Timer_Read(void) y bool Timer_ConsumirVentana(uint32_t *pulsos). MUST usar
TIM_InitTimer, TIM_ConfigMatch, TIM_Enable, TIM_ReadTimer, TIM_GetIntStatus,
TIM_ClearIntPending y TIM_UpdateMatchValue del driver original, sin acceso directo
a registros. TIMER0 y CMSIS MUST quedar encapsulados en .c; header MUST usar sólo
tipos estándar. Drivers MUST permanecer byte a byte intactos. Timer_Init MUST
configurar TIM_US con TIMER_BASE_MS×1000, MR0 = TPI_CONTROL_PERIODO_MS/TIMER_BASE_MS
con interrupción y sin reset ni stop, y TIMER0_IRQn con prioridad 1 habilitada,
dejando el timer detenido. Timer_Start MUST tomar la referencia de
SensorOptico_Read y luego habilitar la cuenta. (Antes: tres wrappers, sin ISR ni
estado propio.)

#### Scenario: S1 — Inicialización y arranque
- Given timer exclusivo desde reset y SystemCoreClock = CCLK real.
- When se llama Init y luego Start.
- Then MUST pedir TIM_US 1000, match 0 con valor 500, interrupción ENABLE y reset/stop
  DISABLE, prioridad 1, y arrancar después de leer el conteo del sensor.

### Requirement: WT2 — Base y lectura
Sin cambios de contrato: TIMER_BASE_MS configurable, Read = TIM_ReadTimer(TIMER0)×
TIMER_BASE_MS en uint32 modular. TC MUST seguir corriendo libre: ningún match MUST
reiniciarlo ni detenerlo. TPI_CONTROL_PERIODO_MS MUST ser múltiplo de TIMER_BASE_MS,
rechazado al compilar en caso contrario.

#### Scenario: S2 — Unidades
- Given timer inicializado/en marcha y ventanas ya cerradas.
- When se consulta.
- Then MUST conservar salida ms igual a TC y no alterar la cuenta ni MR0.

### Requirement: WT3 — Integración acotada
MUST eliminar Configuracion/Temporizacion.h, documentar USW con Doxygen y
mantener build/controles vigentes. MUST NOT modificar aplicación/main/servicios
aceptados ni especificaciones vigentes; resultado requiere aceptación posterior.

#### Scenario: S3 — Evidencia
- Given driver original y módulos previos preservados.
- When se compilan wrappers/firmware y ejecutan regresiones/documentación.
- Then MUST distinguir compilación de funcionamiento físico y documentar resultados reales.

### Requirement: WT4 — Ventana periódica de medición
TIMER0_IRQHandler MUST, sólo si el flag MR0 está activo: leer SensorOptico_Read,
limpiar el flag, reprogramar MR0 sumando TPI_CONTROL_PERIODO_MS/TIMER_BASE_MS módulo
2^32 y publicar pulsos = conteo − conteo anterior módulo 2^32. MUST NOT esperar ni
iterar. Timer_ConsumirVentana MUST devolver true y escribir los pulsos una sola vez
por ventana cerrada; false sin tocar el destino si no hay ventana nueva o si el
destino es NULL (sin consumir). Si se cierran dos o más ventanas sin consultar,
MUST entregar sólo la más reciente y descartar las anteriores, sin acumular. La
lectura MUST ser coherente con el ISR sin deshabilitar interrupciones.

#### Scenario: S4 — Consumo único
- Given referencia 1000 al arrancar y conteo 1060 al primer match.
- When corre el ISR y se consulta dos veces.
- Then la primera MUST dar true/60 con MR0 = 1000; la segunda false sin escribir.

#### Scenario: S5 — Vueltas de contador
- Given conteo anterior UINT32_MAX−29 y actual 30; MR0 = UINT32_MAX−299.
- When corre el ISR.
- Then MUST publicar 60 pulsos y dejar MR0 = 200.

#### Scenario: S6 — Ventana perdida
- Given dos ventanas cerradas de 60 y 25 pulsos sin consulta intermedia.
- When se consulta dos veces.
- Then MUST dar true/25 y luego false.
