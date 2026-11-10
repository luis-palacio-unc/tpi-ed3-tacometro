# Timer — candidato B11

Origen: alcance aprobado por usuario 2026-11-10; R10/R15, consumidor B10.

## Requirements

### Requirement: WT1 — Tres wrappers
Dispositivos/Timer.h/.c MUST exponer void Timer_Init(void), void Timer_Start(void)
y uint32_t Timer_Read(void). MUST delegar en TIM_InitTimer, TIM_Enable y
TIM_ReadTimer del driver original. Timer elegido (TIMER0) y CMSIS MUST quedar
encapsulados en .c; header MUST usar sólo tipos estándar, sin hardware específico.
Drivers MUST permanecer byte a byte intactos. MUST NOT recrear servicio Tiempo,
proveedores, puerto, estado de diagnóstico ni acumulador software/ISR.

#### Scenario: S1 — Inicialización y arranque
- Given timer exclusivo desde reset, sin acciones match/captura, y reloj informado correcto.
- When se llama Init y luego Start.
- Then MUST preparar TIM_US con el período configurado y habilitar el timer mediante el driver.

### Requirement: WT2 — Base y lectura
TIMER_BASE_MS MUST ser configurable al compilar, default1, entero positivo y
convertible a microsegundos uint32. Período/PCLK MUST ser exactos/representables
según el driver, precondición del integrador; no validar ni reconfigurar relojes propios.
Read MUST retornar TIM_ReadTimer(TIMER0)*TIMER_BASE_MS en uint32 modular, sin
reset/parada/escritura de TC. Consumidores MUST guardar marcas de misma época;
duraciones reales<2^31ms. Init MUST preceder Start/Read; configurar desde lazo
serializado, sin otro propietario. No promete validez antes de Init ni precisión física.

#### Scenario: S2 — Unidades
- Given bases1/2/5ms y timer inicializado/en marcha.
- When se consulta.
- Then MUST conservar salida ms y no alterar la cuenta.

### Requirement: WT3 — Integración acotada
MUST eliminar Configuracion/Temporizacion.h, documentar USW con Doxygen y
mantener build/controles vigentes. MUST NOT modificar aplicación/main/servicios
aceptados ni especificaciones vigentes; resultado requiere aceptación posterior.

#### Scenario: S3 — Evidencia
- Given driver original y módulos previos preservados.
- When se compilan wrappers/firmware y ejecutan regresiones/documentación.
- Then MUST distinguir compilación de funcionamiento físico y documentar resultados reales.
