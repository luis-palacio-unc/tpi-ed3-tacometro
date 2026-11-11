# Delta B18/I1 — motor-dispositivo

Candidato de la vía rápida B18. MD1, MD2 y MD4 sin cambios (Motor_Write intacto).

## MODIFIED Requirements

### Requirement: MD3 — Operación mínima de dispositivo
MUST conservar PWM1.1/P2.0 función 01, un despacho por orden válida aun repetida,
sin estado/contexto/caché/ISR/heap/esperas ni acceso propio a registros.
Motor_Write MUST exigir Motor_Init previo; acceso serializado y latch al próximo
ciclo. (Antes: sin init; PWM preconfigurado como precondición externa.)

#### Scenario: S4 — Repeticiones y cero
- Given Motor_Init ejecutado y órdenes repetidas, cero, máximo y descenso.
- When se aplica la secuencia.
- Then MUST despachar cada valor una vez sin repetir la inicialización.

## ADDED Requirements

### Requirement: MD5 — Inicialización de PWM1.1
Motor MUST exponer void Motor_Init(void). MUST usar, en orden, PWM_InitTimer con
PWM_TICK y divisor 25 (PR = 24: contador a 1 MHz con PCLK 25 MHz), PWM_ConfigMatch
de MR0 = 1000 con reset y sin interrupción/stop, PWM_ConfigMatch de MR1 = 0 sin
acciones, PWM_ChannelConfig canal 1 flanco simple, PWM_PinConfig P2.0,
PWM_ChannelEnable canal 1, PWM_CounterEnable y PWM_Enable. Resultado: 1 kHz, salida
baja hasta la primera orden. MR0 MUST coincidir con TPI_MANDO_MANUAL_MAX, verificado
al compilar.

#### Scenario: S6 — Parámetros acordados
- Given dobles de la biblioteca.
- When se llama Motor_Init.
- Then MUST observarse esa secuencia y esos valores, sin PWM_MatchUpdateSingle.
