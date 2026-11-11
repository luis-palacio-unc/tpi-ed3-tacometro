# Sensor óptico — candidato B14

Origen R02/R04/R09/R10/R11/R15. Alcance/recursos acordados2026-11-10; resultado sin aceptar.
Sensor físico confirmado: XD-51; características eléctricas no verificadas en este bloque.

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
ISR, estado mutable, contexto, caché, heap, ventanas o diagnóstico de parada.
TIMER1 exclusivo/estable/arrancado, un flanco ascendente por ranura y señal válida
MUST ser precondiciones externas. Captura/IRQ del canal seleccionado MUST estar
deshabilitada y ningún match MUST reiniciar/detener el conteo. Uso serializado.
Tras reinicio/reconfiguración, consumidor MUST adquirir nueva referencia.

#### Scenario: S2 — Repetición y avance
- Given un conteo y luego más eventos entre consultas.
- When se lee varias veces.
- Then MUST conservar la referencia acumulada y reflejar el avance sin reset propio.

### Requirement: SO3 — Compatibilidad con medición genérica

Conteo MUST permitir al consumidor obtener pulsos de ventana como diferencia
uint32_t modular de dos muestras de la misma época, con menos de2^32 eventos
entre ellas. Muestras/duración/factor coherentes MUST ser externos; la diferencia
MUST NOT añadirse al dispositivo ni al servicio aceptado en este bloque.
Prueba de composición MUST conservar factor explícito de B03 y cero válido.

#### Scenario: S3 — Ventana de referencia
- Given dos muestras separadas por60 eventos, duración1s y factor60.
- When un fixture calcula diferencia y llama B03, también con0/185/200 eventos.
- Then MUST obtener0/60000/185000/200000 milésimasRPM según el caso, sin recorte.

#### Scenario: S4 — Rollover de conteo
- Given anterior=MAX-29 y actual=30, misma época y60 eventos/1s/factor60.
- When el fixture calcula la diferencia modular y llama B03.
- Then MUST obtener60 pulsos y60000 milésimasRPM, sin overflow indefinido o reset.

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
