# Sensor óptico — contrato vigente

Origen R02/R04/R09/R10/R11/R15. Resultado B14 aceptado por el usuario el 2026-11-10.
Sensor físico confirmado: XD-51; características eléctricas no verificadas en este bloque.

B18 (integración I1–I5) aceptado por el usuario el 2026-11-11 sobre e3d3bce; contrato
sincronizado desde el delta del cambio. Verificado en host y por compilación/enlace ARM;
aceptación física en placa pendiente.
[Evidencia](../../changes/archive/2026-11-11-b18-integracion/verify-report.md).

## Requirements

### Requirement: SO1 — Conteo acumulado

Sensor_Optico MUST exponer uint32_t SensorOptico_Read(void), conteo de eventos
acumulados de TIMER1 preconfigurado como contador externo ascendente CAP1.0/P1.18,
PINSEL3 función11. MUST delegar la lectura en TIM_ReadTimer original. MUST devolver
conteo íntegro, incluido0/UINT32_MAX; MUST NOT convertir unidades, calcular RPM,
aplicar factor60 ni recortar por límite185 de consignas. TIMER0 de B11 MUST preservarse.

#### Scenario: S1 — Lecturas exactas e independientes
- Given contador de eventos con valores0/1/60/185/200/MAX y TIMER0 con otro valor.
- When se consulta el sensor repetidamente.
- Then MUST devolver exactamente el conteo de eventos, sin modificar ambos contadores.

### Requirement: SO2 — Lectura sin perturbación
Consultar MUST NOT reiniciar/detener/configurar/arrancar el contador ni implementar
estado mutable, contexto, caché, heap, ventanas o diagnóstico de parada. MUST exigir
SensorOptico_Init previo. La consulta MAY hacerse desde el lazo o desde el ISR de
TIMER0, que cierra las ventanas (contrato timer WT4). Tras reinicializar, el
consumidor MUST adquirir nueva referencia. (Antes: configuración externa y uso sólo
serializado.)

#### Scenario: S2 — Repetición y avance
- Given un conteo y luego más eventos entre consultas.
- When se lee varias veces.
- Then MUST conservar la referencia acumulada y reflejar el avance sin reset propio.

### Requirement: SO3 — Compatibilidad con la medición (I2)
Conteo MUST permitir obtener los pulsos de una ventana como diferencia uint32_t
modular de dos muestras de la misma época, con menos de 2^32 eventos entre ellas.
La diferencia la calcula el ISR de Timer (WT4) y la conversión a RPM el servicio
Medicion_Velocidad (MV2). (Antes: prueba de composición con B03 y factor explícito.)

#### Scenario: S3 — Ventana de referencia
- Given dos muestras separadas por 0, 60, 185 o 200 eventos.
- When se resta.
- Then MUST obtenerse esa cantidad de pulsos, sin leer la base de tiempos.

#### Scenario: S4 — Rollover de conteo
- Given anterior = MAX−29 y actual = 30, misma época.
- When se calcula la diferencia modular.
- Then MUST obtener 60 pulsos, sin overflow indefinido ni reset.

### Requirement: SO4 — Capas, documentación y evidencia

MUST usar biblioteca original intacta y documentar ESW10/USW10.1 con Doxygen.
MUST NOT modificar Timer, Medicion_Velocidad, aplicación/main u otros dispositivos
ni agregar inicialización/clock/pinmux reales. Pinmux, Counter Init/Enable, nivel
eléctrico/cableado y cadencia MUST quedar para integración final. Host con doble
de API y build ARM MUST distinguirse de adquisición/temporización física.

#### Scenario: S5 — Verificación y alcance
- Given wrapper y fixture de medición con fuente de conteo emulada.
- When se ejecutan host/sanitizadores/build/Doxygen y revisión del diff/hashes.
- Then MUST registrar resultados y preservar biblioteca/módulos aceptados; hardware pendiente.

### Requirement: SO5 — Inicialización del contador
Sensor_Optico MUST exponer void SensorOptico_Init(void). MUST usar, en orden,
TIM_InitCounter(TIMER1, CAP1.0, flanco ascendente) — CTCR = 0x01 —, PINSEL_ConfigPin
de P1.18 como CAP1.0 (función 11) con pull-up interno y sin drenador abierto, y
TIM_Enable(TIMER1). El pull-up da reposo definido ante una salida de comparador a
colector abierto o un sensor desconectado. MUST NOT habilitar captura,
match ni interrupciones de TIMER1.

#### Scenario: S6 — Parámetros acordados
- Given dobles de la biblioteca.
- When se llama SensorOptico_Init.
- Then MUST observarse esa secuencia y esos parámetros, y el conteo parte de cero.
