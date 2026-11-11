# Delta B18/I1 — sensor-optico

Candidato de la vía rápida B18. SO1 y SO4 sin cambios (SensorOptico_Read intacto).
I2 modifica SO3: la conversión a RPM dejó de ser un servicio con factor explícito.

## MODIFIED Requirements

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

## ADDED Requirements

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
