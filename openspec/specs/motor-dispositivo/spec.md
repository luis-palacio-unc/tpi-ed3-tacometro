# Dispositivo Motor — contrato vigente

Origen R03/R06/R10/R11/R15. El usuario fija MR0=1000 y exige envío directo del
mando2026-11-10. Resultado b473d0f aceptado por el usuario el2026-11-10.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
aceptación física en placa pendiente.
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

### Requirement: MD1 — Período fijo y mando directo

Motor MUST exponer bool Motor_Write(uint16_t mando_por_mil). Mando válido MUST
ser0..1000 y enviarse íntegro a PWM_MATCH_1 con PWM_MatchUpdateSingle original.
MUST NOT calcular match, escalar, redondear, leer MR0 o transformar1000 en1001.
La configuración de uso MUST fijar MR0=1000 ticks en la inicialización final;
Motor_Write MUST NOT reconfigurar el período por llamada. No interpretar RPM.

#### Scenario: S1 — Valores de referencia
- Given PWM preconfigurado con MR0=1000 y mandos0/250/500/1000.
- When se solicita cada mando.
- Then MUST despachar valores0/250/500/1000 exactos y devolver true.

#### Scenario: S2 — Dominio completo
- Given cada mando representable entre0 y1000.
- When se llama Motor_Write.
- Then MUST enviar el mismo valor una vez, sin recorte185 ni caso especial del máximo.

### Requirement: MD2 — Rechazo y recuperación

MUST devolver false si mando>1000, sin llamar al driver ni emitir cero.
Una llamada válida posterior MUST recuperar sin retener errores. true MUST
significar despacho lógico, no confirmar estado eléctrico o movimiento.
La configuración MR0=1000 MUST ser precondición externa, sin diagnóstico nuevo.

#### Scenario: S3 — Fuera de rango y recuperación
- Given orden válida y mandos1001..UINT16_MAX posteriores.
- When se rechazan y luego llega otra orden válida.
- Then MUST evitar toda escritura por rechazo y recuperar con un único despacho.

### Requirement: MD3 — Operación mínima de dispositivo
MUST conservar PWM1.1/P2.0 función 01, un despacho por orden válida aun repetida,
sin estado/contexto/caché/ISR/heap/esperas ni acceso propio a registros.
Motor_Write MUST exigir Motor_Init previo; acceso serializado y latch al próximo
ciclo. (Antes: sin init; PWM preconfigurado como precondición externa.)

#### Scenario: S4 — Repeticiones y cero
- Given Motor_Init ejecutado y órdenes repetidas, cero, máximo y descenso.
- When se aplica la secuencia.
- Then MUST despachar cada valor una vez sin repetir la inicialización.

### Requirement: MD4 — Documentación y evidencia

MUST actualizar Doxygen ESW.2.1.9/USW.2.1.9.1 y verificar host/san/ARM/regresiones.
MUST retirar el modelo RAM de MR0 y preinclude al dejar de necesitar lectura.
MUST NOT modificar Drivers, servicios/otros dispositivos/contratos aceptados ni
inicializar/enlazar globalmente periféricos. Configuración MR0=1000, frecuencia,
potencia/adaptación/main y validación física de errata PWM.1 quedan al final.

#### Scenario: S5 — Verificación por modalidad
- Given wrapper directo, doble de escritura y módulos aceptados preservados.
- When se ejecutan controles/diff/hashes.
- Then MUST registrar evidencia actual, separada de contratos descartados y del hardware pendiente.

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
